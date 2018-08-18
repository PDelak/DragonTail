#include "compiler.h"
#include "ast.h"
#include "gtest/gtest.h"

void testProgram(std::string text, StatementList result)
{
	auto parser = initialize_parser();
	auto statements = parse(parser.get(), &text[0], &text[0] + text.size());
	EXPECT_EQ(statements.size(), result.size());

	std::ostringstream expected;
	dumpAST(result, expected);
	std::ostringstream parsed;
	dumpAST(statements, parsed);

	EXPECT_EQ(parsed.str(), expected.str());
}

TEST(CoGeCs, test1)
{
	testProgram("var a;",
	{
		makeNode<VarDecl>(0, "a")
	});
}

TEST(CoGeCs, test2)
{
	testProgram("a = 1;",
	{
		makeNode<Expression>(Expression(0,{ "a", "=", "1" }))
	});
}

TEST(CoGeCs, test3)
{
	testProgram("a = 1; a = a - 1;", 
	{
		makeNode<Expression>(Expression(0,{ "a", "=", "1" })),
		makeNode<Expression>(Expression(0,{ "a", "=", "a", "-", "1" })),
	});
}


int main(int argc, char* argv[]) 
{    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    
}
