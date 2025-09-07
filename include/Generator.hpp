#pragma once

#include "Parser.hpp"

class Generator {
public:
    Generator(NodeBye root);
    std::string generate();
private:
    const NodeBye m_Root;
};