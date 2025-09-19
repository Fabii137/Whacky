#pragma once

#include "Parser.hpp"
#include "TypeChecker.hpp"

class OperationGenerator {
public:
    OperationGenerator(std::stringstream& output) : m_Output(output) {}
    
    void generateArithmetic(BinOp op, VarType leftType, VarType rightType);
    void generateComparison(BinOp op, VarType leftType, VarType rightType);
    void generateLogical(BinOp op, VarType leftType, VarType rightType);
    void generateBitwise(BinOp op, VarType leftType, VarType rightType);
    
private:
    std::stringstream& m_Output;
};
