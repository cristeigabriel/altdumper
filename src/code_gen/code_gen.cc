/**
 * @file code_gen.cc
 * @author Cristei Gabriel-Marian (cristei.g772@gmail.com)
 * @brief Arbitrarily generate code for C++
 * @version 0.1
 * @date 2021-09-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

// ===========================================
#include "code_gen.hh"
// ===========================================

// ===========================================
using namespace code_gen;
context::context(const std::string& path)
    : _path(path)
    , _file(_path) {
    // Beginning of header
    _file << "#pragma once\n";
}

context::~context() {
    _file.close();
}
// ===========================================
