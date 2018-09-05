#include <cctype>
#include "compiler.h"
#include "ast.h"
#include "nullvisitor.h"
#include "astcloner.h"
#include "gtest/gtest.h"

void testProgram(std::string text, StatementList result)
{
	AstCloner visitor;
	auto parser = initialize_parser();
	auto stmts = parse(parser.get(), &text[0], &text[0] + text.size(), visitor);
	traverse(stmts, visitor);
	auto statements = visitor.getStatements();
	EXPECT_EQ(statements.size(), result.size());

	std::ostringstream expected;
	dumpAST(result, expected);
	std::ostringstream parsed;
	dumpAST(statements, parsed);
	auto lhs = expected.str();
	auto rhs = parsed.str();
	lhs.erase(std::remove(lhs.begin(), lhs.end(), ' '), lhs.end());
	rhs.erase(std::remove(rhs.begin(), rhs.end(), ' '), rhs.end());
	EXPECT_EQ(lhs, rhs);
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

TEST(CoGeCs, test4)
{
	
	testProgram("{}",
	{
		makeNode<BlockStatement>(0)
	});
}

TEST(CoGeCs, test5)
{
		
	testProgram("if(1) {}",
	{
		makeNode<IfStatement>(IfStatement(0, 
                                          Expression(0,{ "1" }),
                                          { makeNode<BlockStatement>(BlockStatement(0)) }))
	});

}

TEST(CoGeCs, test6)
{

	testProgram("var a; if(1) {} var b;",
	{
		makeNode<VarDecl>(0, "a"),
		makeNode<IfStatement>(IfStatement(0,
		Expression(0,{ "1" }),
		{ makeNode<BlockStatement>(BlockStatement(0)) })),
		makeNode<VarDecl>(0, "b")
	});

}


int main(int argc, char* argv[]) 
{    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    
}
