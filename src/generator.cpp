#include "Generator.hpp"

#include <iostream>
#include <format>
#include <assert.h>

Generator::Generator(NodeProg root): m_Prog(std::move(root)) {

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
            switch (var->type)
            {
                case VarType::Int:
                case VarType::Bool:
                    generator.push(std::format("qword [rsp + {}]", generator.m_StackSize - var->stackLoc));
                    break;
                case VarType::String: {
                    size_t ptrOffset = generator.m_StackSize - (var->stackLoc + 8);
                    generator.push(std::format("qword [rsp + {}]", ptrOffset));

                    size_t lenOffset = generator.m_StackSize - (var->stackLoc + 16);
                    generator.push(std::format("qword [rsp + {}]", lenOffset));
                    break;
                }
                default:
                    assert(false && "Invalid Variable Type");
                    break;
            }
        }

        void operator()(const NodeTermString* string) const {
            std::string label = generator.findStringLiteral(string->string.value.value());

            generator.m_Output << "\tlea rax, [rel " << label << "]\n";
            generator.push("rax");

            generator.m_Output << "\tmov rax, qword [rel " << label << "_len]\n";
            generator.push("rax");
        }

        void operator()(const NodeTermParen* paren) const {
            generator.generateExpr(paren->expr);
        }
    };

    TermVisitor visitor({ .generator = *this });
    std::visit(visitor, term->var);
}

VarType Generator::findType(const NodeExpr* expr) {
    struct ExprTypeVisitor {
        Generator& generator;
        VarType operator()(const NodeTerm* term) const {
            struct TermTypeVisitor {
                Generator& generator;
                VarType operator()(const NodeTermIntLit* intLit) const {
                    return VarType::Int;
                }

                VarType operator()(const NodeTermBool* _bool) const {
                    return VarType::Bool;
                }

                VarType operator()(const NodeTermIdent* ident) const {
                    Var* var = generator.lookupVar(ident->ident.value.value());
                    return var->type;
                }

                VarType operator()(const NodeTermString* string) const {
                    return VarType::String;
                }

                VarType operator()(const NodeTermParen* paren) const {
                    return generator.findType(paren->expr);
                }
            };

            TermTypeVisitor termVisitor({ .generator = generator });
            return std::visit(termVisitor, term->var);
        }

        VarType operator()(const NodeBinExpr* binExpr) const {
            switch (binExpr->op) {
                case BinOp::Add:
                case BinOp::Sub:
                case BinOp::Mul:
                case BinOp::Div:
                    return VarType::Int;
                case BinOp::Eq:
                case BinOp::Neq:
                case BinOp::Lt:
                case BinOp::Le:
                case BinOp::Gt:
                case BinOp::Ge:
                case BinOp::And:
                case BinOp::Or:
                case BinOp::Xor:
                    return VarType::Bool;
                default:
                    assert(false && "Unknown binary operator");
                    return VarType::Int;
            }
        }
    };

    ExprTypeVisitor visitor({ .generator = *this });
    return std::visit(visitor, expr->var);
}

void Generator::generateBinExpr(const NodeBinExpr* binExpr) {
    generateExpr(binExpr->right);
    generateExpr(binExpr->left);
    pop("rax");
    pop("rbx");
    switch (binExpr->op)
    {
        case BinOp::Bor:
            m_Output << "\tor rax, rbx\n";
            break;

        case BinOp::Band:
            m_Output << "\tand rax, rbx\n";
            break;

        case BinOp::Or:
            m_Output << "\tcmp rax, 0\n";
            m_Output << "\tsetne al\n";
            m_Output << "\tmovzx rax, al\n";

            m_Output << "\tcmp rbx, 0\n";
            m_Output << "\tsetne bl\n";
            m_Output << "\tmovzx rbx, bl\n";
            
            m_Output << "\tor rax, rbx\n";
            break;

        case BinOp::And:
            m_Output << "\tcmp rax, 0\n";
            m_Output << "\tsetne al\n";
            m_Output << "\tmovzx rax, al\n";

            m_Output << "\tcmp rbx, 0\n";
            m_Output << "\tsetne bl\n";
            m_Output << "\tmovzx rbx, bl\n";
            
            m_Output << "\tand rax, rbx\n";
            break;

        case BinOp::Xor:
            m_Output << "\txor rax, rbx\n";
            break;

        case BinOp::Neq:
            m_Output << "\tcmp rax, rbx\n";
            m_Output << "\tsetne al\n";
            m_Output << "\tmovzx rax, al\n";
            break;

        case BinOp::Eq:
            m_Output << "\tcmp rax, rbx\n";
            m_Output << "\tsete al\n";
            m_Output << "\tmovzx rax, al\n";
            break;

        case BinOp::Ge:
            m_Output << "\tcmp rax, rbx\n";
            m_Output << "\tsetge al\n";
            m_Output << "\tmovzx rax, al\n";
            break;

        case BinOp::Gt:
            m_Output << "\tcmp rax, rbx\n";
            m_Output << "\tsetg al\n";
            m_Output << "\tmovzx rax, al\n";
            break;

        case BinOp::Le:
            m_Output << "\tcmp rax, rbx\n";
            m_Output << "\tsetle al\n";
            m_Output << "\tmovzx rax, al\n";
            break;

        case BinOp::Lt:
            m_Output << "\tcmp rax, rbx\n";
            m_Output << "\tsetl al\n";
            m_Output << "\tmovzx rax, al\n";
            break;

        case BinOp::Add:
            m_Output << "\tadd rax, rbx\n";
            break;

        case BinOp::Sub:
            m_Output << "\tsub rax, rbx\n";
            break;

        case BinOp::Mul:
            m_Output << "\tmul rbx\n";
            break;

        case BinOp::Div:
            m_Output << "\tdiv rbx\n";
            break; 

        default:
            assert(false && "Invalid Binary Operator");
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

            const std::string label = generator.createLabel();

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
            VarType type = generator.findType(gimme->expr);

            generator.declareVar(gimme->ident.value.value(), type);
            generator.generateExpr(gimme->expr);
        }

        void operator()(const NodeStmtAssignment* assignment) const {
            Var* var = generator.lookupVar(assignment->ident.value.value());

            generator.generateExpr(assignment->expr);

            if(var->type == VarType::String) {
                generator.pop("rax"); // len
                generator.pop("rbx"); // ptr

                size_t ptrOffset = generator.m_StackSize - (var->stackLoc + 8);
                generator.m_Output << std::format("\tmov [rsp + {}], rbx\n", ptrOffset);

                size_t lenOffset = generator.m_StackSize - (var->stackLoc + 16);
                generator.m_Output << std::format("\tmov [rsp + {}], rax\n", lenOffset);
            } else {
                generator.pop("rax");
                generator.m_Output << std::format("\tmov [rsp + {}], rax\n", generator.m_StackSize - var->stackLoc);
            } 
        }

        void operator()(const NodeScope* scope) const {
            generator.generateScope(scope);
        }

        void operator()(const NodeStmtMaybe* maybe) const {
            generator.generateExpr(maybe->expr);
            generator.pop("rax");

            const std::string label = generator.createLabel();

            generator.m_Output << "\tcmp rax, 0\n";
            generator.m_Output << "\tjz " << label << "\n";
            generator.generateScope(maybe->scope);
                
            
            if(maybe->pred.has_value()) {
                const std::string& endLabel = generator.createLabel();
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

            auto& currentScope = generator.m_Scopes.back();
            for (auto varIt = currentScope.vars.begin(); varIt != currentScope.vars.end(); varIt++) {
                std::cout << varIt->first << " at " << varIt->second.stackLoc << " size " << varIt->second.size << std::endl;
            }

            generator.m_Output << "\tmov rax, 1\n";
            generator.m_Output << "\tmov rdi, 1\n";
            generator.pop("rdx"); // len
            generator.pop("rsi"); // ptr
            generator.m_Output << "\tsyscall\n";
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
    m_Scopes.emplace_back(Scope{{}, m_StackSize});
}

void Generator::leaveScope() {
    Scope scope = m_Scopes.back();
    m_Scopes.pop_back();
    
    const size_t popCount = m_StackSize - scope.stackStart;
    m_Output << "\tadd rsp, " << popCount << std::endl;
    m_StackSize = scope.stackStart;
}

std::string Generator::createLabel() {
    return "label" + std::to_string(m_LabelCount++);
}

std::string Generator::findStringLiteral(const std::string& value) {
    if(m_StringLiterals.contains(value)) {
        return m_StringLiterals.at(value);
    }

    std::string label = "str" + std::to_string(m_StringLiterals.size());
    m_StringLiterals.insert({ value, label });

    m_Data << std::format("\t{} db \"{}\", 0\n", label, value);
    m_Data << std::format("\t{}_len: dq {}\n", label, value.size());

    return label;
}

Var* Generator::lookupVar(const std::string& name) {    
    for (auto it = m_Scopes.rbegin(); it != m_Scopes.rend(); it++) {
        auto found = it->vars.find(name);
        if(found != it->vars.end()) {
            return &found->second;
        }
    }

    std::cerr << "Undeclared identifier: " << name << std::endl;
    exit(EXIT_FAILURE);
}

void Generator::declareVar(const std::string& name, VarType type) {
    auto& currentScope = m_Scopes.back().vars;
    if (currentScope.contains(name)) {
        std::cerr << "Identifier already declared in this scope: " << name << std::endl;
        exit(EXIT_FAILURE);
    }
    size_t size = 8;
    if (type == VarType::String) {
        size = 16;
    }

    currentScope.insert({ name, Var{ .size = size, .type = type, .stackLoc = m_StackSize } });
}