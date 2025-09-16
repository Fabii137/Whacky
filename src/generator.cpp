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
            std::string offset = std::format("qword [rsp + {}]", (generator.m_StackSize - var->stackLoc-1) * 8);
            generator.push(offset);
            
        }

        void operator()(const NodeTermParen* paren) const {
            generator.generateExpr(paren->expr);
        }

    };

    TermVisitor visitor({ .generator = *this });
    std::visit(visitor, term->var);
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
            generator.declareVar(gimme->ident.value.value());
            generator.generateExpr(gimme->expr);
        }

        void operator()(const NodeStmtAssignment* assignment) const {
            Var* var = generator.lookupVar(assignment->ident.value.value());

            generator.generateExpr(assignment->expr);
            generator.pop("rax");

            generator.m_Output << std::format("\tmov [rsp + {}], rax\n", (generator.m_StackSize - var->stackLoc-1) * 8);
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
    };
    StmtVisitor visitor({ .generator = *this });
    std::visit(visitor, stmt->var);
}

std::string Generator::generateProg() {
    m_Output << "global _start\n_start:\n";
    enterScope();

    for(const NodeStmt* stmt : m_Prog.stmts) {
        generateStmt(stmt);
    }

    m_Output << "\tmov rax, 60\n";
    m_Output << "\tmov rdi, 0\n";
    m_Output << "\tsyscall";
    return m_Output.str();
}

void Generator::push(const std::string& reg) {
    m_Output << "\tpush " << reg << "\n";
    m_StackSize++;
}

void Generator::pop(const std::string& reg) {
    m_Output << "\tpop " << reg << "\n";
    m_StackSize--;
}

void Generator::enterScope() {
    m_Scopes.emplace_back(Scope{{}, m_StackSize});
}

void Generator::leaveScope() {
    Scope scope = m_Scopes.back();
    m_Scopes.pop_back();
    
    const size_t popCount = m_StackSize - scope.stackStart;
    m_Output << "\tadd rsp, " << popCount * 8 << std::endl;
    m_StackSize = scope.stackStart;
}

std::string Generator::createLabel() {
    return "label" + std::to_string(m_LabelCount++);
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

void Generator::declareVar(const std::string& name) {
    auto& currentScope = m_Scopes.back().vars;
    if (currentScope.contains(name)) {
        std::cerr << "Identifier already declared in this scope: " << name << std::endl;
        exit(EXIT_FAILURE);
    }
    currentScope.insert({ name, Var{ .stackLoc = m_StackSize } });
}