#include <cctype>
#include "compiler.h"
#include "ast.h"
#include "nullvisitor.h"
#include "astcloner.h"
#include "cfg_flatten.h"
#include "gtest/gtest.h"

template<typename Visitor>
void testProgram(std::string text, StatementList result)
{
	Visitor visitor;
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
	testProgram<AstCloner>("var a;",
	{
		makeNode(VarDecl(0, "a"))
	});
}

TEST(CoGeCs, test2)
{
	testProgram<AstCloner>("a = 1;",
	{
		makeNode(Expression(0,{ "a", "=", "1" }))
	});
}

TEST(CoGeCs, test3)
{
	testProgram<AstCloner>("a = 1; a = a - 1;", 
	{
		makeNode(Expression(0,{ "a", "=", "1" })),
		makeNode(Expression(0,{ "a", "=", "a", "-", "1" })),
	});
}

TEST(CoGeCs, test4)
{
	
	testProgram<AstCloner>("{}",
	{
		makeNode(BlockStatement(0))
	});
}

TEST(CoGeCs, test5)
{
		
	testProgram<AstCloner>("if(1) {}",
	{
		makeNode(IfStatement(0, 
                             Expression(0,{ "1" }),
                             { makeNode(BlockStatement(0)) }))
	});

}

TEST(CoGeCs, test6)
{

	testProgram<AstCloner>("var a; if(1) {} var b;",
	{
		makeNode(VarDecl(0, "a")),
		makeNode(IfStatement(0,
				 Expression(0,{ "1" }),
				 { makeNode(BlockStatement(0)) })),
				 makeNode(VarDecl(0, "b"))
	});

}

TEST(CoGeCs, test7)
{
	testProgram<CFGFlattener>("if(1) {}",
	{
		makeNode(VarDecl(0, "temp__0")),
		makeNode(Expression(0, {"temp__0", "=", "1"})),
		makeNode(IfStatement(0,
				 Expression(0,{ "!","temp__0" }),
				 { makeNode(GotoStatement(0, "label__1")) })),
		makeNode(LabelStatement(0, "label__1"))
	});
}

TEST(CoGeCs, test8)
{
	testProgram<CFGFlattener>("while(1) {}",
	{
		makeNode(VarDecl(0, "temp__0")),
		makeNode(LabelStatement(0, "label__1")),
		makeNode(Expression(0,{ "temp__0", "=", "1" })),
		makeNode(IfStatement(0,
		Expression(0,{ "!","temp__0" }),
		{ makeNode(GotoStatement(0, "label__2")) })),
		makeNode(GotoStatement(0, "label__1")),
		makeNode(LabelStatement(0, "label__2"))
	});
}

TEST(CoGeCs, test9)
{
	testProgram<CFGFlattener>("{1;}",
	{
		makeNode(Expression(0,{ "1" }))
	});
}

TEST(CoGeCs, test10)
{
	testProgram<CFGFlattener>("{{1;}}",
	{
		makeNode(Expression(0,{ "1" }))
	});
}

TEST(CoGeCs, test11)
{
	testProgram<CFGFlattener>("{1;{2;}}",
	{
		makeNode(Expression(0,{ "1" })),
		makeNode(Expression(0,{ "2" }))
	});
}

TEST(CoGeCs, test12)
{
	testProgram<CFGFlattener>("{label_0:{2;}}",
	{
		makeNode(LabelStatement(0,{ "label_0" })),
		makeNode(Expression(0,{ "2" }))
	});
}

TEST(CoGeCs, test13)
{	
	testProgram<CFGFlattener>("if (1) {2; if(3) {4;} 5; } 6;",
	{
		makeNode(VarDecl(0, "temp__2")),
		makeNode(Expression(0,{ "temp__2", "=", "1" })),
		makeNode(IfStatement(0,
				Expression(0,{ "!","temp__2" }),
				{ makeNode(GotoStatement(0, "label__3")) })),
		makeNode(Expression(0,{ "2" })),
		makeNode(VarDecl(0, "temp__0")),
		makeNode(Expression(0,{ "temp__0", "=", "3" })),
		makeNode(IfStatement(0,
				Expression(0,{ "!","temp__0" }),
				{ makeNode(GotoStatement(0, "label__1")) })),
		makeNode(Expression(0,{ "4" })),
		makeNode(LabelStatement(0, "label__1")),
		makeNode(Expression(0,{ "5" })),
		makeNode(LabelStatement(0, "label__3")),
		makeNode(Expression(0,{ "6" })),

	});
}


int main(int argc, char* argv[]) 
{    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    
}
