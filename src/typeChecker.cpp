#include "TypeChecker.hpp"
#include <format>

TypeChecker::TypeChecker(const std::vector<Scope>& scopes) : m_Scopes(scopes) {

}

TypeInfo TypeChecker::checkExpr(const NodeExpr* expr) {
    struct ExprTypeVisitor {
        TypeChecker& checker;
        TypeInfo operator()(const NodeTerm* term) const {
            return checker.checkTerm(term);
        }
        TypeInfo operator()(const NodeBinExpr* binExpr) const {
            return checker.checkBinExpr(binExpr);
        }
    };
    
    ExprTypeVisitor visitor{ *this };
    return std::visit(visitor, expr->var);
}

TypeInfo TypeChecker::checkTerm(const NodeTerm* term) {
    struct TermTypeVisitor {
        TypeChecker& checker;
        TypeInfo operator()(const NodeTermIntLit*) const {
            return TypeInfo::valid(VarType::Number);
        }
        TypeInfo operator()(const NodeTermBool*) const {
            return TypeInfo::valid(VarType::Bool);
        }
        TypeInfo operator()(const NodeTermString*) const {
            return TypeInfo::valid(VarType::String);
        }
        TypeInfo operator()(const NodeTermIdent* ident) const {
            const Var* var = checker.lookupVar(ident->ident.value.value());
            if (!var) {
                return TypeInfo::error("Undeclared identifier: " + ident->ident.value.value());
            }
            return TypeInfo::valid(var->type);
        }
        TypeInfo operator()(const NodeTermParen* paren) const {
            return checker.checkExpr(paren->expr);
        }
        TypeInfo operator()(const NodeTermCall* call) const {
            const Thingy* thingy = checker.lookupThingy(call->ident.value.value());
            if (!thingy) {
                return TypeInfo::error("Undeclared function: " + call->ident.value.value());
            }

            if (call->args.size() != thingy->paramTypes.size()) {
                return TypeInfo::error(std::format("Argument count mismatch for function: {}. Expected: {}. Count: {}", call->ident.value.value(), thingy->paramTypes.size(), call->args.size()));
            }

            for(size_t i = 0; i < call->args.size(); i++) {
                TypeInfo argType = checker.checkExpr(call->args[i]);
                if(!argType.isValid) {
                    return argType;
                }

                if(argType.type != thingy->paramTypes[i]) {
                    return TypeInfo::error(std::format("Type mismatch in argument {} of function '{}'. Expected {}, got {}", 
                        i, call->ident.value.value(), getTypeName(thingy->paramTypes[i]), getTypeName(argType.type)));
                }
            }

            return TypeInfo::valid(thingy->returnType);
        }
    };
    
    TermTypeVisitor visitor{*this};
    return std::visit(visitor, term->var);
}

TypeInfo TypeChecker::checkBinExpr(const NodeBinExpr* binExpr) {
    TypeInfo leftType = checkExpr(binExpr->left);
    TypeInfo rightType = checkExpr(binExpr->right);
    
    if (!leftType.isValid)  {
        return leftType;
    }
    
    if (!rightType.isValid) {
        return rightType;
    }
    
    
    switch (binExpr->op) {
        case BinOp::Add:
            if (leftType.type == VarType::String || rightType.type == VarType::String) {
                return TypeInfo::valid(VarType::String);
            }
            if (leftType.type == VarType::Number && rightType.type == VarType::Number) {
                return TypeInfo::valid(VarType::Number);
            }
            return TypeInfo::error(std::format("Invalid types for addition: cannot add {} and {}", 
                getTypeName(leftType.type), getTypeName(rightType.type)));
            
        case BinOp::Mul:
            if ((leftType.type == VarType::String && rightType.type == VarType::Number) ||
                (leftType.type == VarType::Number && rightType.type == VarType::String)) {
                return TypeInfo::valid(VarType::String);
            }
            if (leftType.type == VarType::Number && rightType.type == VarType::Number) {
                return TypeInfo::valid(VarType::Number);
            }
            return TypeInfo::error(std::format("Invalid types for multiplication: cannot multiply {} and {}", 
                getTypeName(leftType.type), getTypeName(rightType.type)));
            
        case BinOp::Sub:
        case BinOp::Div:
            if (leftType.type != VarType::Number || rightType.type != VarType::Number) {
                return TypeInfo::error("Arithmetic operations require numbers");
            }
            return TypeInfo::valid(VarType::Number);
            
        case BinOp::Eq:
        case BinOp::Neq:
            return TypeInfo::valid(VarType::Bool);
            
        case BinOp::Lt:
        case BinOp::Le:
        case BinOp::Gt:
        case BinOp::Ge:
            if (leftType.type == VarType::String || rightType.type == VarType::String) {
                return TypeInfo::error("Comparison operations not supported on strings");
            }
            return TypeInfo::valid(VarType::Bool);
            
        case BinOp::And:
        case BinOp::Or:
            return TypeInfo::valid(VarType::Bool);
            
        case BinOp::Band:
        case BinOp::Bor:
        case BinOp::Xor:
            if (leftType.type == VarType::String || rightType.type == VarType::String) {
                return TypeInfo::error("Bitwise operations not supported on strings");
            }
            return TypeInfo::valid(VarType::Number);
            
        default:
            return TypeInfo::error("Unknown binary operator");
    }
}

const Var* TypeChecker::lookupVar(const std::string& name) {
    for (auto it = m_Scopes.rbegin(); it != m_Scopes.rend(); it++) {
        auto found = it->vars.find(name);
        if (found != it->vars.end()) {
            return &found->second;
        }
    }
    return nullptr;
}

const Thingy* TypeChecker::lookupThingy(const std::string& name) {
     for (auto it = m_Scopes.rbegin(); it != m_Scopes.rend(); ++it) {
        auto found = it->functions.find(name);
        if (found != it->functions.end()) {
            return &found->second;
        }
    }
    return nullptr;
}