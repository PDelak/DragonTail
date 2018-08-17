#include "compiler.h"
#include "ast.h"
#include "gtest/gtest.h"


TEST(CoGeCs, test1)
{
	std::string VarText = "var a;";
	
	StatementList result = {
		std::make_shared<VarDecl>(0, "a")
	};

	auto parser = initialize_parser();
	auto statements = parse(parser.get(), &VarText[0], &VarText[0] + VarText.size());
	EXPECT_EQ(statements.size(), 1);
	
	std::ostringstream expected;	
	dumpAST(result, expected);
	std::ostringstream parsed;
	dumpAST(statements, parsed);

	EXPECT_EQ(parsed.str(), expected.str());
}

int main(int argc, char* argv[]) 
{    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    
}
