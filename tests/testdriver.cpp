#include <cctype>
#include "compiler.h"
#include "ast.h"
#include "nullvisitor.h"
#include "astcloner.h"
#include "cfg_flatten.h"
#include "gtest/gtest.h"
#include "compilertests.h"
#include "codegen.h"

int main(int argc, char* argv[]) 
{    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    
}
