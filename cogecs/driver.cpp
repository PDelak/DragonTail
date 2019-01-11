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
#include "compiler.h"
#include "nullvisitor.h"
#include "dumpvisitor.h"
#include "astcloner.h"
#include "cfg_flatten.h"
#include "code_emitter.h"

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

    if (argc < 3) {
        std::cerr << "syntax: compiler.exe filename [ast|run|transform|emitx86]" << std::endl;        
        return -1;
    }

    try {
        std::string command;

        if (argc == 3) command = argv[2];

        std::vector<int> v = { 4,2,6 };

        auto inputFile = argv[1];

        auto p = initialize_parser(inputFile);

        NullVisitor nvisitor;

        auto statements = compile(argv[1], p.get(), nvisitor);

        CFGFlattener visitor;

        traverse(statements, visitor);

        if (command == "ast") {
            dumpAST(visitor.getStatements(), std::cout);
        }
        else if (command == "transform") {
            dumpCode(visitor.getStatements(), std::cout);
        }
        else if (command == "emitx86") {
            auto x86_text = emitMachineCode(visitor.getStatements());
            x86_text.dumpExt();
        }
        else if (command == "run") {
            auto x86_text = emitMachineCode(visitor.getStatements());
            JitCompiler jit(x86_text);
            auto x86function = jit.compile();
            x86function();
        }
    }
    catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
    }
    return 0;
}
