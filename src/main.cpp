#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include "Generator.hpp"
#include "Parser.hpp"
#include "Tokenizer.hpp"

int main(int argc, char* argv[]) {
    if(argc != 2) {
        std::cerr << "Incorrect usage. Correct usage is ..." << std::endl;
        std::cerr << "whacky <input.wy>" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string contents;
    {
        std::stringstream contentsStream;
        std::fstream input(argv[1], std::ios::in);
        contentsStream << input.rdbuf();
        contents = contentsStream.str();
    }

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeProg> prog = parser.parseProg();

    if(!prog.has_value()) {
        std::cerr << "Invalid program" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    {
        Generator generator(std::move(prog.value()));
        std::fstream out("out.asm", std::ios::out);
        out << generator.generateProg();
    }

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

}