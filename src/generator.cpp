#include "Generator.hpp"
#include <sstream>

Generator::Generator(NodeBye root): m_Root(std::move(root)) {

}

std::string Generator::generate() {
    std::stringstream output;
    output << "global _start\n_start:\n";
    output << "\tmov rax, 60\n";
    output << "\tmov rdi, " << m_Root.expr.int_lit.value.value() << "\n";
    output << "\tsyscall";
    return output.str();
}