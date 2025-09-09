#include "Generator.hpp"

#include <iostream>
#include <assert.h>

Generator::Generator(NodeProg root): m_Prog(std::move(root)) {

}

void Generator::generateTerm(const NodeTerm* term) {
    struct TermVisitor {
        Generator* generator;
        void operator()(const NodeTermIntLit* termIntList) const {
            generator->m_Output << "\tmov rax, " << termIntList->int_lit.value.value() << "\n";
            generator->push("rax");
        }

        void operator()(const NodeTermIdent* termIdent) const {
            if(const auto& varEntry = generator->m_Vars.find(termIdent->ident.value.value()); varEntry != generator->m_Vars.end()) {
                std::stringstream offset;
                offset << "QWORD [rsp + " << (generator->m_StackSize - varEntry->second.stackLoc-1) * 8 << "]";
                generator->push(offset.str());
            } else {
                std::cerr << "Undeclared identifier: " << termIdent->ident.value.value() << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    };

    TermVisitor visitor({ .generator = this });
    std::visit(visitor, term->var);
}

void Generator::generateExpr(const NodeExpr* expr) {
    struct ExprVisitor {
        Generator* generator;
        void operator()(const NodeTerm* term) const {
            generator->generateTerm(term);
        }

        void operator()(const NodeBinExpr* binExpr) const {
            generator->generateExpr(binExpr->add->left);
            generator->generateExpr(binExpr->add->right);

            generator->pop("rax");
            generator->pop("rbx");
            generator->m_Output << "\tadd rax, rbx\n";
            generator->push("rax");
        }
    };

    ExprVisitor visitor({ .generator = this });
    std::visit(visitor, expr->var);
}

void Generator::generateStmt(const NodeStmt* stmt) {
    struct StmtVisitor {
        Generator* generator;
        void operator()(const NodeStmtBye* stmtBye) const {
            generator->generateExpr(stmtBye->expr);
            generator->m_Output << "\tmov rax, 60\n";
            generator->pop("rdi");
            generator->m_Output << "\tsyscall\n";
        }

        void operator()(const NodeStmtLet* stmtLet) {
            if(generator->m_Vars.contains(stmtLet->ident.value.value())) {
                std::cerr << "Identifier already used: " << stmtLet->ident.value.value() << std::endl;
                exit(EXIT_FAILURE);
            }

            generator->m_Vars.insert({ stmtLet->ident.value.value(), Var { .stackLoc = generator->m_StackSize } });
            generator->generateExpr(stmtLet->expr);

        }
    };

    StmtVisitor visitor({ .generator = this });
    std::visit(visitor, stmt->var);
}

std::string Generator::generateProg() {
    m_Output << "global _start\n_start:\n";

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