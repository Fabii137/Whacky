#include "OperationGenerator.hpp"
#include <iostream>

void OperationGenerator::generateArithmetic(BinOp op, VarType leftType, VarType rightType) {
    switch (op) {
        case BinOp::Add:
            if (leftType == VarType::String && rightType == VarType::String) {
                // String concatenation - call runtime function               
                m_Output << "\t; String concatenation\n";
                m_Output << "\tmov rdi, rdx\n";         // left pointer (arg1)
                m_Output << "\tmov rsi, rax\n";         // left length (arg2)
                m_Output << "\tmov rdx, rcx\n";         // right pointer (arg3)
                m_Output << "\tmov rcx, rbx\n";         // right length (arg4)
                m_Output << "\tsub rsp, 8\n";           // allocate space for out_len
                m_Output << "\tmov r8, rsp\n";          // pointer to out_len (arg5)

                m_Output << "\tcall __whacky_strcat\n";
                
                // rax = result pointer
                m_Output << "\tmov rdx, rax\n";         // result pointer
                m_Output << "\tpop rax\n";              // result length
            } else {
                m_Output << "\tadd rax, rbx\n";
            }
            break;
            
        case BinOp::Sub:
            m_Output << "\tsub rax, rbx\n";
            break;
            
        case BinOp::Mul:
              if ((leftType == VarType::String && rightType == VarType::Number) ||
                (leftType == VarType::Number && rightType == VarType::String)) {
                // string multiplication
                m_Output << "\t; String multiplication\n";

                // format: string in rdi/rsi, number in rdx
                if (leftType == VarType::String) {
                    m_Output << "\tmov rdi, rdx\n";         // string pointer (arg1)
                    m_Output << "\tmov rsi, rax\n";         // string length (arg2)
                    m_Output << "\tmov rdx, rbx\n";         // n (arg3)
                } else {
                    m_Output << "\tmov rdi, rcx\n";         // string pointer (arg1)
                    m_Output << "\tmov rsi, rbx\n";         // string length (arg2)
                    m_Output << "\tmov rdx, rax\n";         // n (arg3)
                }
                
                m_Output << "\tsub rsp, 8\n";               // allocate space for out_len
                m_Output << "\tmov rcx, rsp\n";             // pointer to out_len (arg4)
                m_Output << "\tcall __whacky_strmul\n";
                
                // rax = result pointer
                m_Output << "\tmov rdx, rax\n";             // result pointer
                m_Output << "\tpop rax\n";                  // result length
            } else {
                m_Output << "\tmul rbx\n";
            }
            break;
            break;
            
        case BinOp::Div:
            m_Output << "\tdiv rbx\n";
            break;
    }
}

void OperationGenerator::generateComparison(BinOp op, VarType leftType, VarType rightType) {
    m_Output << "\tcmp rax, rbx\n";
    
    switch (op) {
        case BinOp::Eq:
            m_Output << "\tsete al\n";
            break;
        case BinOp::Neq:
            m_Output << "\tsetne al\n";
            break;
        case BinOp::Lt:
            m_Output << "\tsetl al\n";
            break;
        case BinOp::Le:
            m_Output << "\tsetle al\n";
            break;
        case BinOp::Gt:
            m_Output << "\tsetg al\n";
            break;
        case BinOp::Ge:
            m_Output << "\tsetge al\n";
            break;
    }
    
    m_Output << "\tmovzx rax, al\n";
}

void OperationGenerator::generateLogical(BinOp op, VarType leftType, VarType rightType) {
    m_Output << "\tcmp rax, 0\n";
    m_Output << "\tsetne al\n";
    m_Output << "\tmovzx rax, al\n";
    
    m_Output << "\tcmp rbx, 0\n";
    m_Output << "\tsetne bl\n";
    m_Output << "\tmovzx rbx, bl\n";
    
    switch (op) {
        case BinOp::And:
            m_Output << "\tand rax, rbx\n";
            break;
        case BinOp::Or:
            m_Output << "\tor rax, rbx\n";
            break;
    }
}

void OperationGenerator::generateBitwise(BinOp op, VarType leftType, VarType rightType) {
    switch (op) {
        case BinOp::Band:
            m_Output << "\tand rax, rbx\n";
            break;
        case BinOp::Bor:
            m_Output << "\tor rax, rbx\n";
            break;
        case BinOp::Xor:
            m_Output << "\txor rax, rbx\n";
            break;
    }
}