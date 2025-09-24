#include "Generator.hpp"

#include <iostream>
#include <format>
#include <assert.h>

Generator::Generator(NodeProg root): m_Prog(std::move(root)) {
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
            Var* var = generator.lookupVar(ident->ident.value.value());
            generator.generateVariableLoad(var);
        }

        void operator()(const NodeTermString* string) const {
            auto label = generator.findStringLiteral(string->string.value.value());

            generator.m_Output << "\tlea rax, [rel " << label.value() << "]\n";
            generator.push("rax");

            generator.m_Output << "\tmov rax, "<< label.value() << "_len\n";
            generator.push("rax");
        }

        void operator()(const NodeTermParen* paren) const {
            generator.generateExpr(paren->expr);
        }

        void operator()(const NodeTermCall* call) const {
            for(auto it = call->args.rbegin(); it != call->args.rend(); it++) {
                generator.generateExpr(*it);
            }

            const Thingy* thingy = generator.lookupThingy(call->ident.value.value());
            if(!thingy) {
                generator.error("Undeclared function: " + call->ident.value.value());
            }
            generator.m_Output << "\tcall " << thingy->label << "\n";

            generator.push("rax");
        }
    };

    TermVisitor visitor({ .generator = *this });
    std::visit(visitor, term->var);
}

void Generator::generateBinExpr(const NodeBinExpr* binExpr) {
    TypeInfo typeInfo = m_TypeChecker->checkBinExpr(binExpr);
    if (!typeInfo.isValid) {
        error(typeInfo.errorMsg);
    }

    TypeInfo leftType = m_TypeChecker->checkExpr(binExpr->left);
    TypeInfo rightType = m_TypeChecker->checkExpr(binExpr->right);

    generateExpr(binExpr->right);
    generateExpr(binExpr->left);

    if (leftType.type == VarType::String || rightType.type == VarType::String) {
        pop("rax"); // left len
        pop("rdx"); // left ptr
        pop("rbx"); // right len
        pop("rcx"); // right ptr
    } else {
        pop("rax"); // left num
        pop("rbx"); // right num
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
    push("rax");
}

void Generator::generateExpr(const NodeExpr* expr) {
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

void Generator::generateThingy(const NodeStmtThingy* thingy) {
    std::vector<VarType> params(thingy->params.size(), VarType::Int);

    Thingy th {.paramTypes = params, .returnType = VarType::Int, .label = createLabel(thingy->name.value.value()) }; // temp

    declareThingy(thingy->name.value.value(), th);

    m_Output << th.label << ":\n";
    enterScope();
    for(const Token& param : thingy->params) {
        declareVar(param.value.value(), VarType::Int); //temp
    }

    generateScope(thingy->scope);
    leaveScope();
    m_Output << "\tret\n";
}

void Generator::generateStmt(const NodeStmt* stmt) {
    struct StmtVisitor {
        Generator& generator;
        void operator()(const NodeStmtBye* bye) const {
            generator.generateExpr(bye->expr);
            generator.m_Output << "\tmov rax, 60\n";
            generator.pop("rdi");
            generator.m_Output << "\tsyscall\n";
        }

        void operator()(const NodeStmtGimme* gimme) const {
            TypeInfo typeInfo = generator.m_TypeChecker->checkExpr(gimme->expr);
            if (!typeInfo.isValid) {
                generator.error(typeInfo.errorMsg);
            }

            generator.declareVar(gimme->ident.value.value(), typeInfo.type);
            generator.generateExpr(gimme->expr);
        }

        void operator()(const NodeStmtAssignment* assignment) const {
            Var* var = generator.lookupVar(assignment->ident.value.value());

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
            generator.m_Output << "\tret\n";
        }

        void operator()(const NodeStmtLoop* loop) const {
            generator.enterScope();
            
            generator.declareVar(loop->ident.value.value(), VarType::Int);
            Var* var = generator.lookupVar(loop->ident.value.value());

            generator.generateExpr(loop->start);
            generator.generateVariableStore(var);

            const std::string startLabel = generator.createLabel("loop_start");
            const std::string endLabel = generator.createLabel("loop_end");

            generator.m_Output << startLabel << ":\n";

            generator.generateExpr(loop->end);
            generator.pop("rax");
            generator.m_Output << "\tcmp rax, [rsp + " << (generator.m_StackSize - var->stackLoc) << "]\n";
            generator.m_Output << "\tjle " << endLabel << "\n";

            generator.generateScope(loop->scope);

            generator.m_Output << "\tadd qword [rsp + " << (generator.m_StackSize - var->stackLoc) << "], 1\n";
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
    m_Output << "section .text\n\tglobal _start\n_start:\n";
    m_Data << "section .data\n";
    enterScope();

    for(const NodeStmt* stmt : m_Prog.stmts) {
        generateStmt(stmt);
    }

    m_Output << "\tmov rax, 60\n";
    m_Output << "\tmov rdi, 0\n";
    m_Output << "\tsyscall";
    return m_Data.str() +  m_Output.str();
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
    Scope scope = m_Scopes.back();
    m_Scopes.pop_back();
    
    const size_t popCount = m_StackSize - scope.stackStart;
    m_Output << "\tadd rsp, " << popCount << std::endl;
    m_StackSize = scope.stackStart;
}

Var* Generator::lookupVar(const std::string& name) {    
    for (auto it = m_Scopes.rbegin(); it != m_Scopes.rend(); it++) {
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

    currentScope.insert({ name, Var{ .size = size, .type = type, .stackLoc = m_StackSize } });
}

void Generator::declareThingy(const std::string& name, const Thingy& thingy) {
    auto& currentScope = m_Scopes.back().functions;
    if(currentScope.contains(name)) {
        error("Function already declared in this scope: " + name);
    }
    currentScope.insert({ name, thingy });
}
const Thingy* Generator::lookupThingy(const std::string& name) {
    for(auto it = m_Scopes.rbegin(); it != m_Scopes.rend(); it++) {
        auto found = it->functions.find(name);
        if(found != it->functions.end()) {
            return &found->second;
        }
    }
    return nullptr;
}

std::string Generator::createLabel(const std::string& name /*="label"*/) {
    return name + std::to_string(m_LabelCount++);
}

std::optional<std::string> Generator::findStringLiteral(const std::string& value, const bool& create /*=true*/) {
    if(m_StringLiterals.contains(value)) {
        return m_StringLiterals.at(value);
    }

    if(!create) {
        return {};
    }

    std::string label = "str" + std::to_string(m_StringLiterals.size());
    m_StringLiterals.insert({ value, label });

    m_Data << std::format("\t{} db \"{}\", 0\n", label, escapeString(value));
    m_Data << std::format("\t{}_len: equ $- {}\n", label, label);

    return label;
}

const std::string Generator::escapeString(const std::string& input) {
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
    switch (var->type) {
        case VarType::Int:
        case VarType::Bool:
            push(std::format("qword [rsp + {}]", m_StackSize - var->stackLoc));
            break;
            
        case VarType::String: {
            size_t ptrOffset = m_StackSize - (var->stackLoc + 8);
            push(std::format("qword [rsp + {}]", ptrOffset));
            
            size_t lenOffset = m_StackSize - (var->stackLoc + 16);
            push(std::format("qword [rsp + {}]", lenOffset));
            break;
        }
    }
}

void Generator::generateVariableStore(const Var* var) {
    if (var->type == VarType::String) {
        pop("rax"); // len
        pop("rbx"); // ptr
        
        size_t ptrOffset = m_StackSize - (var->stackLoc + 8);
        m_Output << std::format("\tmov [rsp + {}], rbx\n", ptrOffset);
        
        size_t lenOffset = m_StackSize - (var->stackLoc + 16);
        m_Output << std::format("\tmov [rsp + {}], rax\n", lenOffset);
    } else {
        pop("rax");
        m_Output << std::format("\tmov [rsp + {}], rax\n", m_StackSize - var->stackLoc);
    }
}

void Generator::error(const std::string& msg) {
    std::cerr << "[Generator Error] " << msg << std::endl;
    exit(EXIT_FAILURE);
}