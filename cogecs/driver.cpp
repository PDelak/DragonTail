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

    if (argc < 2) {
        std::cerr << "syntax: language filename" << std::endl;
        return -1;
    }

    std::vector<int> v = { 4,2,6 };

    auto p = initialize_parser(argv[1]);
	
    NullVisitor nvisitor;

    auto statements = compile(argv[1], p.get(), nvisitor);

    CFGFlattener visitor;

    traverse(statements, visitor);

    dumpAST(visitor.getStatements(), std::cout);

    dumpCode(visitor.getStatements(), std::cout);

    auto x86_text = emitMachineCode(visitor.getStatements());
	
    x86_text();

    return 0;
}
