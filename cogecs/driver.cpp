#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <list>
#include <stack>
#include <chrono>
#include <memory>
#include <functional>
#include <windows.h>
#include "dparse.h"
#include "ast.h"
#include "jitcompiler.h"
#include "expressions.h"
#include "interpreter.h"
#include "code_emitter.h"
#include "compiler.h"

void print(const std::vector<int>& v)
{
    for (const auto e : v) std::cout << e << std::endl;    
}

bool expect_eq(const std::vector<int>& v1, const std::vector<int>& v2)
{
    if (v1.size() != v2.size()) return false;
    size_t index = 0;
    for (const auto& e : v1) {
        if (e != v2[index++]) return false;        
    }
    return true;
}

int main(int argc, char* argv[])
{    
    if (argc < 2) {
        std::cerr << "syntax: language filename" << std::endl;
        return -1;
    }
    
    std::vector<int> v = { 4,2,6 };

    auto p = initialize_parser(argv[1]);

    auto statements = compile(argv[1], p.get());

    auto fun = emit_machine_code(&v, statements);
    
    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        for (size_t i = 0; i < 1000; ++i) { fun(); }
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Machine code execution = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << std::endl;
    }
    
    std::vector<int> v2 = { 4,2,6 };

    {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        for (size_t i = 0; i < 1000; ++i) { eval(&v2, statements);}
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Interpretation = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << std::endl;
    }

    if(!expect_eq(v, v2)) throw 1;
 
    return 0;
}