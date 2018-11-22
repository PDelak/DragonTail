#pragma once

#include "gtest/gtest.h"
#include "nullvisitor.h"
#include "../cogecs/compiler.h"
#include "tools.h"

TEST(compiler, test1)
{
	StatementStack stmtStack;
	StatementList statementList;
	NullVisitor nvisitor;
	size_t scope = 0;
	stmtStack.push_back("var_statement");
	pre_visit_node("id", "x", stmtStack, statementList, scope, nvisitor);
	post_visit_node("var_statement", "", stmtStack, statementList, scope, nvisitor);
	EXPECT_EQ(statementList.size(), 1);
	checkASTs(statementList,
	{
		makeNode(VarDecl(0, "x"))
	});
}
