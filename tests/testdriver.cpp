#include <cctype>
#include "compiler.h"
#include "ast.h"
#include "nullvisitor.h"
#include "astcloner.h"
#include "cfg_flatten.h"
#include "prefixtree.h"
#include "gtest/gtest.h"
#include "../tests/compiler.h"
#include "../tests/codegen.h"

int main(int argc, char* argv[]) 
{    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    
}
