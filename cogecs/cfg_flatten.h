#pragma once

// Control Flow Flattening is first transformation phase 
// where compound statements are flattening to be used
// for low level or machine code generation

#include <stack>
#include "astvisitor.h"
#include "ast.h"

struct CFGFlattener : public AstVisitor
{	
	CFGFlattener() {}
	
	void visitPre(const BasicStatement*) {}
	void visitPre(const VarDecl* stmt) 
	{
		if (stmt->scope != 0) return;
		auto node = makeNode<VarDecl>(stmt->scope, stmt->var_name);
		statements.push_back(node);
	}
	void visitPre(const Expression*) {}
	void visitPre(const IfStatement*) 
	{

	}
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
