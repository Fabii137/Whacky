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
    struct BinExprVisitor {
        Generator& generator;

        void operator()(const NodeBinExprBor* bor) const {
            generator.generateExpr(bor->right);
            generator.generateExpr(bor->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tor rax, rbx\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprBand* band) const {
            generator.generateExpr(band->right);
            generator.generateExpr(band->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tand rax, rbx\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprOr* _or) const {
            generator.generateExpr(_or->right);
            generator.generateExpr(_or->left);

            generator.pop("rax");
            generator.pop("rbx");

            generator.m_Output << "\tcmp rax, 0\n";
            generator.m_Output << "\tsetne al\n";
            generator.m_Output << "\tmovzx rax, al\n";

            generator.m_Output << "\tcmp rbx, 0\n";
            generator.m_Output << "\tsetne bl\n";
            generator.m_Output << "\tmovzx rbx, bl\n";
            
            generator.m_Output << "\tor rax, rbx\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprAnd* _and) const {
            generator.generateExpr(_and->right);
            generator.generateExpr(_and->left);

            generator.pop("rax");
            generator.pop("rbx");

            generator.m_Output << "\tcmp rax, 0\n";
            generator.m_Output << "\tsetne al\n";
            generator.m_Output << "\tmovzx rax, al\n";

            generator.m_Output << "\tcmp rbx, 0\n";
            generator.m_Output << "\tsetne bl\n";
            generator.m_Output << "\tmovzx rbx, bl\n";
            
            generator.m_Output << "\tand rax, rbx\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprXor* _xor) const {
            generator.generateExpr(_xor->right);
            generator.generateExpr(_xor->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\txor rax, rbx\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprNeq* neq) const {
            generator.generateExpr(neq->right);
            generator.generateExpr(neq->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tcmp rax, rbx\n";
            generator.m_Output << "\tsetne al\n";
            generator.m_Output << "\tmovzx rax, al\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprEq* eq) const {
            generator.generateExpr(eq->right);
            generator.generateExpr(eq->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tcmp rax, rbx\n";
            generator.m_Output << "\tsete al\n";
            generator.m_Output << "\tmovzx rax, al\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprGe* ge) const {
            generator.generateExpr(ge->right);
            generator.generateExpr(ge->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tcmp rax, rbx\n";
            generator.m_Output << "\tsetge al\n";
            generator.m_Output << "\tmovzx rax, al\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprGt* gt) const {
            generator.generateExpr(gt->right);
            generator.generateExpr(gt->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tcmp rax, rbx\n";
            generator.m_Output << "\tsetg al\n";
            generator.m_Output << "\tmovzx rax, al\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprLe* le) const {
            generator.generateExpr(le->right);
            generator.generateExpr(le->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tcmp rax, rbx\n";
            generator.m_Output << "\tsetle al\n";
            generator.m_Output << "\tmovzx rax, al\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprLt* lt) const {
            generator.generateExpr(lt->right);
            generator.generateExpr(lt->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tcmp rax, rbx\n";
            generator.m_Output << "\tsetl al\n";
            generator.m_Output << "\tmovzx rax, al\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprAdd* add) const {
            generator.generateExpr(add->right);
            generator.generateExpr(add->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tadd rax, rbx\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprSub* sub) const {
            generator.generateExpr(sub->right);
            generator.generateExpr(sub->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tsub rax, rbx\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprMul* mul) const {
            generator.generateExpr(mul->right);
            generator.generateExpr(mul->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tmul rbx\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprDiv* div) const {
            generator.generateExpr(div->right);
            generator.generateExpr(div->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tdiv rbx\n";
            generator.push("rax");
        }
    };

    BinExprVisitor visitor({ .generator = *this });
    std::visit(visitor, binExpr->var);
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
            generator.declareVar(gimme->ident.value.value(),  Var { .stackLoc = generator.m_StackSize });
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

            generator.m_Output << "\ttest rax, rax\n";
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

void Generator::declareVar(const std::string& name, Var var) {
    auto& currentScope = m_Scopes.back().vars;
    if (currentScope.contains(name)) {
        std::cerr << "Identifier already declared in this scope: " << name << std::endl;
        exit(EXIT_FAILURE);
    }
    currentScope.insert({name, var});
}