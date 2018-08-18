#include "compiler.h"
#include "ast.h"
#include "gtest/gtest.h"


TEST(CoGeCs, test1)
{
	std::string VarText = "var a;";
	
	StatementList result = {
		makeNode<VarDecl>(0, "a")
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

TEST(CoGeCs, test2)
{
	std::string text = "a = 1;";
		
	StatementList result = {
		makeNode<Expression>(Expression(0, {"a", "=", "1"}))
	};

	auto parser = initialize_parser();
	auto statements = parse(parser.get(), &text[0], &text[0] + text.size());
	EXPECT_EQ(statements.size(), 1);

	std::ostringstream expected;
	dumpAST(result, expected);
	std::ostringstream parsed;
	dumpAST(statements, parsed);

	EXPECT_EQ(parsed.str(), expected.str());
}

TEST(CoGeCs, test3)
{
	std::string text = "a = 1; a = a - 1;";

	StatementList result = {
		makeNode<Expression>(Expression(0,{ "a", "=", "1" })),
		makeNode<Expression>(Expression(0,{ "a", "=", "a", "-", "1" })),
	};

	auto parser = initialize_parser();
	auto statements = parse(parser.get(), &text[0], &text[0] + text.size());
	EXPECT_EQ(statements.size(), 2);

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
