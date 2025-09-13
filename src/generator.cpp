#include "Generator.hpp"

#include <iostream>
#include <assert.h>

Generator::Generator(NodeProg root): m_Prog(std::move(root)) {

}

void Generator::generateTerm(const NodeTerm* term) {
    struct TermVisitor {
        Generator& generator;
        void operator()(const NodeTermIntLit* termIntLit) const {
            generator.m_Output << "\tmov rax, " << termIntLit->int_lit.value.value() << "\n";
            generator.push("rax");
        }

        void operator()(const NodeTermIdent* termIdent) const {
            Var* var = generator.lookupVar(termIdent->ident.value.value());
            std::stringstream offset;
            offset << "QWORD [rsp + " << (generator.m_StackSize - var->stackLoc-1) * 8 << "]";
            generator.push(offset.str());
            
        }

        void operator()(const NodeTermParen* termParen) const {
            generator.generateExpr(termParen->expr);
        }

    };

    TermVisitor visitor({ .generator = *this });
    std::visit(visitor, term->var);
}

void Generator::generateBinExpr(const NodeBinExpr* binExpr) {
    struct BinExprVisitor {
        Generator& generator;
        void operator()(const NodeBinExprAdd* binExprAdd) const {
            generator.generateExpr(binExprAdd->right);
            generator.generateExpr(binExprAdd->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tadd rax, rbx\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprSub* binExprSub) const {
            generator.generateExpr(binExprSub->right);
            generator.generateExpr(binExprSub->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tsub rax, rbx\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprMul* binExprMul) const {
            generator.generateExpr(binExprMul->right);
            generator.generateExpr(binExprMul->left);

            generator.pop("rax");
            generator.pop("rbx");
            generator.m_Output << "\tmul rbx\n";
            generator.push("rax");
        }

        void operator()(const NodeBinExprDiv* binExprDiv) const {
            generator.generateExpr(binExprDiv->right);
            generator.generateExpr(binExprDiv->left);

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

void Generator::generateStmt(const NodeStmt* stmt) {
    struct StmtVisitor {
        Generator& generator;
        void operator()(const NodeStmtBye* stmtBye) const {
            generator.generateExpr(stmtBye->expr);
            generator.m_Output << "\tmov rax, 60\n";
            generator.pop("rdi");
            generator.m_Output << "\tsyscall\n";
        }

        void operator()(const NodeStmtLet* stmtLet) {
            generator.declareVar(stmtLet->ident.value.value(),  Var { .stackLoc = generator.m_StackSize });
            generator.generateExpr(stmtLet->expr);
        }

        void operator()(const NodeScope* scope) const {
            generator.generateScope(scope);
        }

        void operator()(const NodeStmtMaybe* stmtMaybe) {
            generator.generateExpr(stmtMaybe->expr);
            generator.pop("rax");

            const std::string label = generator.createLabel();

            generator.m_Output << "\ttest rax, rax\n";
            generator.m_Output << "\tjz " << label << "\n";
            generator.generateScope(stmtMaybe->scope);
            generator.m_Output << label << ":\n";
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
    m_StackSize -= popCount;
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