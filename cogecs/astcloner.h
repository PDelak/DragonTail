#pragma once

#include <stack>
#include "astvisitor.h"
#include "ast.h"

struct AstCloner : public AstVisitor
{	
	AstCloner() {}
	
	void visitPre(const BasicStatement*) {}
	void visitPre(const VarDecl* stmt) 
	{
		auto node = makeNode<VarDecl>(stmt->scope, stmt->var_name);
		if (stmt->scope == 0) statements.push_back(node);
		else nodesStack.push(node);
	}
	void visitPre(const Expression*) {}
	void visitPre(const IfStatement*) {}
	void visitPre(const WhileLoop*) {}
	void visitPre(const BlockStatement*) {}
	void visitPre(const LabelStatement*) {}
	void visitPre(const GotoStatement*) {}

	void visitPost(const BasicStatement* stmt) 
	{
	}
	void visitPost(const VarDecl* stmt) 
	{
	}
	void visitPost(const Expression* stmt) 
	{
	}
	void visitPost(const IfStatement* stmt) 
	{
	}
	void visitPost(const WhileLoop* stmt) 
	{
	}
	void visitPost(const BlockStatement* stmt) 
	{
	}
	void visitPost(const LabelStatement* stmt)
	{
	}
	void visitPost(const GotoStatement* stmt)
	{
	}
	StatementList getStatements() const { return statements; }
private:
	StatementList statements;
	std::stack<StatementPtr> nodesStack;
};
