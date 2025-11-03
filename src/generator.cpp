#include "Generator.hpp"

#include <iostream>
#include <format>
#include <cassert>

Generator::Generator(NodeProg prog): m_Prog(std::move(prog)) {
    m_TypeChecker = std::make_unique<TypeChecker>(m_Scopes);
    m_OpGenerator = std::make_unique<OperationGenerator>(m_Output);
}

void Generator::generateTerm(const NodeTerm* term) {
    struct TermVisitor {
        Generator& generator;
        void operator()(const NodeTermIntLit* intLit) const {
            generator.m_Output << "\tmov rax, " << intLit->int_lit.value.value() << "\n";
            generator.push("rax");
        }

        void operator()(const NodeTermBool* _bool) const {
            generator.m_Output << "\tmov rax, " << _bool->_bool.value.value() << "\n";
            generator.push("rax");
        }

        void operator()(const NodeTermIdent* ident) const {
            const Var* var = generator.lookupVar(ident->ident.value.value());
            generator.generateVariableLoad(var);
        }

        void operator()(const NodeTermString* string) const {
            const auto label = generator.findStringLiteral(string->string.value.value());

            generator.m_Output << "\tlea rax, [rel " << label << "]\n";
            generator.push("rax");

            generator.m_Output << "\tmov rax, "<< label << "_len\n";
            generator.push("rax");
        }

        void operator()(const NodeTermParen* paren) const {
            generator.generateExpr(paren->expr);
        }

        void operator()(const NodeTermCall* call) const {
            for(auto it = call->args.rbegin(); it != call->args.rend(); ++it) {
                generator.generateExpr(*it);
            }

            const Thingy* thingy = generator.lookupThingy(call->ident.value.value());
            generator.m_Output << "\tcall " << thingy->label << "\n";

            size_t totalParamSize = 0;
            for(VarType paramType : thingy->paramTypes) {
                totalParamSize += (paramType == VarType::String) ? 16 : 8;
            }
            if (totalParamSize > 0) {
                generator.m_Output << "\tadd rsp, " << totalParamSize << "\n";
                generator.m_StackSize -= totalParamSize;
            }

            generator.push("rax");
        }
    };

    TermVisitor visitor({ .generator = *this });
    std::visit(visitor, term->var);
}

void Generator::generateBinExpr(const NodeBinExpr* binExpr) {
    const TypeInfo leftType = m_TypeChecker->checkExpr(binExpr->left);
    const TypeInfo rightType = m_TypeChecker->checkExpr(binExpr->right);
    const TypeInfo resultType = m_TypeChecker->checkBinExpr(binExpr);

    generateExpr(binExpr->right);
    generateExpr(binExpr->left);

    if (leftType.type == VarType::String) {
        pop("rax"); // len
        pop("rdx"); // ptr
    } else {
        pop("rax");
    }

    if (rightType.type == VarType::String) {
        pop("rbx"); // len
        pop("rcx"); // ptr
    } else {
        pop("rbx");
    }

    switch (binExpr->op)
    {
        case BinOp::Add:
        case BinOp::Sub:
        case BinOp::Mul:
        case BinOp::Div:
            m_OpGenerator->generateArithmetic(binExpr->op, leftType.type, rightType.type);
            break;
        case BinOp::Eq:
        case BinOp::Neq:
        case BinOp::Lt:
        case BinOp::Le:
        case BinOp::Gt:
        case BinOp::Ge:
            m_OpGenerator->generateComparison(binExpr->op, leftType.type, rightType.type);
            break;
        case BinOp::And:
        case BinOp::Or:
            m_OpGenerator->generateLogical(binExpr->op, leftType.type, rightType.type);
            break;

        case BinOp::Band:
        case BinOp::Bor:
        case BinOp::Xor:
            m_OpGenerator->generateBitwise(binExpr->op, leftType.type, rightType.type);
            break;
        
        default:
            error("Unknown Binary Operator");
    }
    
    // for string results, push both pointer and length
    if (resultType.type == VarType::String) {
        push("rdx"); // ptr
        push("rax"); // len
    } else {
        push("rax");
    }
}

void Generator::generateExpr(const NodeExpr* expr) {
    const TypeInfo typeInfo = m_TypeChecker->checkExpr(expr);
    if (!typeInfo.isValid) {
        error(typeInfo.errorMsg);
    }

    struct ExprVisitor {
        Generator& generator;
        void operator()(const NodeTerm* term) const {
            generator.generateTerm(term);
        }

        void operator()(const NodeBinExpr* binExpr) const {
            generator.generateBinExpr(binExpr);
        }
    };

    ExprVisitor visitor({ .generator = *this });
    std::visit(visitor, expr->var);
}

void Generator::generateScope(const NodeScope* scope) {
    enterScope();
            
    for(const NodeStmt* stmt : scope->stmts) {
        generateStmt(stmt);
    }

    leaveScope();
}

void Generator::generateMaybePred(const NodeMaybePred* pred, const std::string& endLabel) {
    struct PredVisitor {
        Generator& generator;
        const std::string& endLabel;
        void operator()(const NodeMaybePredBut* but) const {
            generator.generateExpr(but->expr);
            generator.pop("rax");

            const std::string label = generator.createLabel("maybe_pred");

            generator.m_Output << "\tcmp rax, 0\n";
            generator.m_Output << "\tjz " << label << "\n";
            generator.generateScope(but->scope);
            generator.m_Output << "\tjmp " << endLabel << "\n";
            if (but->pred.has_value()) {
                generator.m_Output << label << ":\n";
                generator.generateMaybePred(but->pred.value(), endLabel);
            }
        }

        void operator()(const NodeMaybePredNah* nah) const {
            generator.generateScope(nah->scope);
        }
    };
    PredVisitor visitor({ .generator = *this, .endLabel = endLabel });
    std::visit(visitor, pred->var);
}

void Generator::generateThingy(const NodeStmtThingy* stmtThingy) {
    std::vector<VarType> params;
    for (const NodeParam* param : stmtThingy->params) {
        params.push_back(tokenTypeToVarType(param->type->type));
    }

    VarType returnType = tokenTypeToVarType(stmtThingy->returnType->type);
    const Thingy thingy {.paramTypes = params, .returnType = returnType, .label = createLabel(stmtThingy->name.value.value()) };

    declareThingy(stmtThingy->name.value.value(), thingy);

    m_Output << thingy.label << ":\n";

    // function prologue
    m_Output << "\tpush rbp\n";
    m_Output << "\tmov rbp, rsp\n";

    enterScope();

    size_t currentParamOffset = 16;
    for(size_t i = 0; i < stmtThingy->params.size(); i++) {
        const NodeParam* param = stmtThingy->params[i];
        VarType paramType = tokenTypeToVarType(param->type->type);

        declareParam(param->name.value.value(), paramType, currentParamOffset);

        size_t paramSize = (paramType == VarType::String) ? 16 : 8;
        currentParamOffset += paramSize;
    }

    generateScope(stmtThingy->scope);

    leaveScope();
    m_Output << "\tpop rbp\n";
    m_Output << "\tret\n";
}

void Generator::generateStmt(const NodeStmt* stmt) {
    struct StmtVisitor {
        Generator& generator;
        void operator()(const NodeStmtBye* bye) const {
            // check that the expression is a number
            const TypeInfo exprType = generator.m_TypeChecker->checkExpr(bye->expr);
            if (!exprType.isValid) {
                error(exprType.errorMsg);
            }
            if (exprType.type != VarType::Number) {
                error(std::format("bye() requires a number argument, got {}", getTypeName(exprType.type)));
            }

            generator.generateExpr(bye->expr);
            generator.m_Output << "\tmov rax, 60\n";
            generator.pop("rdi");
            generator.m_Output << "\tsyscall\n";
        }

        void operator()(const NodeStmtGimme* gimme) const {
            // Get the type from the type annotation
            VarType declaredType = tokenTypeToVarType(gimme->type->type);
            
            // Check expression type matches declared type
            const TypeInfo exprType = generator.m_TypeChecker->checkExpr(gimme->expr);
            if (!exprType.isValid) {
                error(exprType.errorMsg);
            }
            if (exprType.type != declaredType) {
                error(std::format("Type mismatch in variable declaration '{}'. Expected {}, got {}", 
                    gimme->ident.value.value(), getTypeName(declaredType), getTypeName(exprType.type)));
            }

            generator.declareVar(gimme->ident.value.value(), declaredType);
            generator.generateExpr(gimme->expr);

            const Var* var = generator.lookupVar(gimme->ident.value.value());
            generator.generateVariableStore(var);
        }

        void operator()(const NodeStmtAssignment* assignment) const {
            const Var* var = generator.lookupVar(assignment->ident.value.value());
            const TypeInfo exprType = generator.m_TypeChecker->checkExpr(assignment->expr);
            
            if (!exprType.isValid) {
                error(exprType.errorMsg);
            }
            if (exprType.type != var->type) {
                error(std::format("Type mismatch in assignment to '{}'. Expected {}, got {}", 
                    assignment->ident.value.value(), getTypeName(var->type), getTypeName(exprType.type)));
            }

            generator.generateExpr(assignment->expr);
            generator.generateVariableStore(var);
        }

        void operator()(const NodeScope* scope) const {
            generator.generateScope(scope);
        }

        void operator()(const NodeStmtMaybe* maybe) const {
            generator.generateExpr(maybe->expr);
            generator.pop("rax");

            const std::string label = generator.createLabel("maybe");

            generator.m_Output << "\tcmp rax, 0\n";
            generator.m_Output << "\tjz " << label << "\n";
            generator.generateScope(maybe->scope);
                
            
            if(maybe->pred.has_value()) {
                const std::string& endLabel = generator.createLabel("maybe_pred");
                generator.m_Output << "\tjmp " << endLabel << "\n";
                generator.m_Output << label << ":\n";
                generator.generateMaybePred(maybe->pred.value(), endLabel);
                generator.m_Output << endLabel << ":\n";
            } else {
                generator.m_Output << label << ":\n";
            }
        }

        void operator()(const NodeStmtYell* yell) const {
            // check that the expression is a string
            const TypeInfo exprType = generator.m_TypeChecker->checkExpr(yell->expr);
            if (!exprType.isValid) {
                error(exprType.errorMsg);
            }
            if (exprType.type != VarType::String) {
                error(std::format("yell() requires a string argument, got {}", getTypeName(exprType.type)));
            }

            generator.generateExpr(yell->expr);

            generator.m_Output << "\tmov rax, 1\n";
            generator.m_Output << "\tmov rdi, 1\n";
            generator.pop("rdx"); // len
            generator.pop("rsi"); // ptr
            generator.m_Output << "\tsyscall\n";
        }

        void operator()(const NodeStmtThingy* thingy) const {
            generator.generateThingy(thingy);
        }

        void operator()(const NodeStmtGimmeback* gimmeback) const {
            generator.generateExpr(gimmeback->expr);
            generator.pop("rax");

            const Scope& currentScope = generator.m_Scopes.back();
            size_t cleanupSize = generator.m_StackSize - currentScope.stackStart;
            if (cleanupSize > 0) {
                generator.m_Output << "\tadd rsp, " << cleanupSize << "\n";
            }
            generator.m_Output << "\tpop rbp\n";

            generator.m_Output << "\tret\n";
        }

        void operator()(const NodeStmtFour* four) const {
            generator.enterScope();
            
            generator.declareVar(four->ident.value.value(), VarType::Number);
            const Var* var = generator.lookupVar(four->ident.value.value());

            generator.generateExpr(four->start);
            generator.generateVariableStore(var);

            const std::string startLabel = generator.createLabel("loop_start");
            const std::string endLabel = generator.createLabel("loop_end");

            generator.m_Output << startLabel << ":\n";

            generator.generateExpr(four->end);
            generator.pop("rax");
            generator.m_Output << "\tcmp rax, [rbp - " << var->stackLoc << "]\n";
            generator.m_Output << "\tjle " << endLabel << "\n";

            generator.generateScope(four->scope);

            generator.m_Output << "\tadd qword [rbp - " << var->stackLoc << "], 1\n";
            generator.m_Output << "\tjmp " << startLabel << "\n";
            generator.m_Output << endLabel << ":\n";

            generator.leaveScope();
        }

        void operator()(const NodeStmtWhy* why) const {
            const std::string startLabel = generator.createLabel("why_start");
            const std::string endLabel = generator.createLabel("why_end");

            generator.m_Output << startLabel << ":\n";

            generator.generateExpr(why->expr);
            generator.pop("rax");
            
            generator.m_Output << "\tcmp rax, 0\n";
            generator.m_Output << "\tjz " << endLabel << "\n";

            generator.generateScope(why->scope);

            generator.m_Output << "\tjmp " << startLabel << "\n";
            generator.m_Output << endLabel << ":\n";
        }
    };
    StmtVisitor visitor({ .generator = *this });
    std::visit(visitor, stmt->var);
}

std::string Generator::generateProg() {
    m_Output << "section .text\n";
    m_Output << "\tglobal _start\n";
    m_Output << "\textern __whacky_strcat\n";
    m_Output << "\textern __whacky_strmul\n\n";

    m_Data << "section .data\n";
    
    enterScope();
    // generate all thingy definitions
    for(const NodeStmt* stmt : m_Prog.stmts) {
        if (auto* stmtVar = std::get_if<NodeStmtThingy*>(&stmt->var)) {
            generateThingy(*stmtVar);
        }
    }
    
    m_Output << "_start:\n";
    m_Output << "\tpush rbp\n";
    m_Output << "\tmov rbp, rsp\n";
    
    for(const NodeStmt* stmt : m_Prog.stmts) {
        // skip thingies
        if (std::holds_alternative<NodeStmtThingy*>(stmt->var)) {
            continue;
        }
        generateStmt(stmt);
    }

    leaveScope();

    m_Output << "\tpop rbp\n";
    m_Output << "\tmov rax, 60\n";
    m_Output << "\tmov rdi, 0\n";
    m_Output << "\tsyscall";
    
    return m_Data.str() + m_Output.str();
}

void Generator::push(const std::string& reg, size_t size /*=8*/) {
    m_Output << "\tpush " << reg << "\n";
    m_StackSize += size;
}

void Generator::pop(const std::string& reg, size_t size /*=8*/) {
    m_Output << "\tpop " << reg << "\n";
    m_StackSize -= size;
}


void Generator::enterScope() {
    m_Scopes.emplace_back(Scope{{}, {}, m_StackSize});
}

void Generator::leaveScope() {
    const Scope scope = m_Scopes.back();
    m_Scopes.pop_back();
    
    const size_t popCount = m_StackSize - scope.stackStart;
    if (popCount > 0) {
        m_Output << "\tadd rsp, " << popCount << "\n";
        m_StackSize = scope.stackStart;
    }
}

Var* Generator::lookupVar(const std::string& name) {    
    for (auto it = m_Scopes.rbegin(); it != m_Scopes.rend(); ++it) {
        auto found = it->vars.find(name);
        if(found != it->vars.end()) {
            return &found->second;
        }
    }

    error("Undeclared identifier: " + name);
    return nullptr;
}

void Generator::declareVar(const std::string& name, VarType type) {
    auto& currentScope = m_Scopes.back().vars;
    if (currentScope.contains(name)) {
        error("Identifier already declared in this scope: " + name);
    }
    size_t size = 8;
    if (type == VarType::String) {
        size = 16;
    }

    m_Output << "\tsub rsp, " << size << "\n";
    m_StackSize += size;

    currentScope.insert({ name, Var{ .size = size, .type = type, .stackLoc = m_StackSize, .isParam = false } });
}

void Generator::declareParam(const std::string& name, VarType type, size_t paramOffset) {
    auto& currentScope = m_Scopes.back().vars;
    if (currentScope.contains(name)) {
        error("Parameter already declared: " + name);
    }

    size_t size = 8;
    if (type == VarType::String) {
        size = 16;
    }

    currentScope.insert({ name, Var { .size = size, .type = type, .stackLoc = paramOffset, .isParam = true } });
}

void Generator::declareThingy(const std::string& name, const Thingy& thingy) {
    auto& currentScope = m_Scopes.back().functions;
    if(currentScope.contains(name)) {
        error("Function already declared in this scope: " + name);
    }
    currentScope.insert({ name, thingy });
}
const Thingy* Generator::lookupThingy(const std::string& name) {
    for(auto it = m_Scopes.rbegin(); it != m_Scopes.rend(); ++it) {
        auto found = it->functions.find(name);
        if(found != it->functions.end()) {
            return &found->second;
        }
    }
    error("Undeclared function: " + name);
    return nullptr;
}

std::string Generator::createLabel(const std::string& name /*="label"*/) {
    return name + std::to_string(m_LabelCount++);
}

std::string Generator::findStringLiteral(const std::string& value) {
    if(m_StringLiterals.contains(value)) {
        return m_StringLiterals.at(value);
    }

    std::string label = "str" + std::to_string(m_StringLiterals.size());
    m_StringLiterals.insert({ value, label });

    m_Data << std::format("\t{} db \"{}\", 0\n", label, escapeString(value));
    m_Data << std::format("\t{}_len: equ $- {}\n", label, label);

    return label;
}

std::string Generator::escapeString(const std::string& input) {
    std::string out;
    for (size_t i = 0; i < input.size(); i++) {
        if (input[i] == '\\' && i + 1 < input.size()) {
            switch (input[i+1]) {
                case 'n': out += "\", 10, \""; i++;
                    break;
                case 't': out += "\", 9, \""; i++;
                    break;
                case 'r': out += "\", 13, \""; i++;
                    break;
                case '\\': out += "\", 92, \""; i++;
                    break;
                case '"': out += "\", 34, \""; i++;
                    break;
                default:
                    out.push_back(input[i]);
            }
        } else {
            out.push_back(input[i]);
        }
    }
    return out;
}

void Generator::generateVariableLoad(const Var* var) {
    const char op = (var->isParam) ? '+' : '-';
    switch (var->type) {
        case VarType::Number:
        case VarType::Bool:
            push(std::format("qword [rbp {} {}]", op, var->stackLoc));
            break;
            
        case VarType::String: {
            const size_t lenOffset = var->isParam ? var->stackLoc + 8 : var->stackLoc - 8;
            push(std::format("qword [rbp {} {}]", op, var->stackLoc));
            push(std::format("qword [rbp {} {}]", op, lenOffset));
            break;
        }
    }
}

void Generator::generateVariableStore(const Var* var) {
    const char op = (var->isParam) ? '+' : '-';
    switch(var->type) {
        case VarType::Number:
        case VarType::Bool:
            pop("rax");
            m_Output << std::format("\tmov [rbp {} {}], rax\n", op, var->stackLoc);
            break;
        case VarType::String:
            pop("rax"); // len
            pop("rbx"); // ptr
            const size_t lenOffset = var->isParam ? var->stackLoc + 8 : var->stackLoc - 8;
            m_Output << std::format("\tmov [rbp {} {}], rbx\n", op, var->stackLoc);
            m_Output << std::format("\tmov [rbp {} {}], rax\n", op, lenOffset);
            break;
    }
}

void Generator::error(const std::string& msg) {
    std::cerr << "[Generator Error] " << msg << std::endl;
    exit(EXIT_FAILURE);
}