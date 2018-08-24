#pragma once

#include <stack>
#include "astvisitor.h"
#include "ast.h"

// AstCloner clones AST deeply

struct AstCloner : public AstVisitor
{	
	AstCloner() {}
	
	void visitPre(const BasicStatement*) {}
	void visitPre(const VarDecl* stmt) 
	{
		auto node = makeNode<VarDecl>(stmt->scope, stmt->var_name);
		nodesStack.push(node);
	}
	void visitPre(const Expression* stmt) 
	{
		auto node = makeNode<Expression>(Expression(stmt->scope));
		static_cast<Expression*>(node.get())->elements = stmt->elements;
		nodesStack.push(node);

	}
	void visitPre(const IfStatement* stmt) 
	{
	}
	void visitPre(const WhileLoop*) {}
	void visitPre(const BlockStatement* stmt) 
	{
		auto node = makeNode<BlockStatement>(BlockStatement(stmt->scope));
		nodesStack.push(node);
		++scope;
	}
	void visitPre(const LabelStatement*) {}
	void visitPre(const GotoStatement*) {}

	void visitPost(const BasicStatement* stmt) 
	{
	}
	void visitPost(const VarDecl* stmt) 
	{
		auto node = nodesStack.top();
		statements.push_back(node);
		nodesStack.pop();
	}
	void visitPost(const Expression* stmt) 
	{
		auto node = nodesStack.top();
		statements.push_back(node);
		nodesStack.pop();
	}
	void visitPost(const IfStatement* stmt) 
	{
	}
	void visitPost(const WhileLoop* stmt) 
	{
	}
	void visitPost(const BlockStatement* stmt) 
	{
		auto block = nodesStack.top();
		nodesStack.pop();
		StatementList blockStatements;
		auto it = std::find_if(statements.rbegin(), statements.rend(), [&](const StatementPtr& s) {
			return s->scope != scope;
		});
		std::copy(it.base(), statements.end(), std::back_inserter(blockStatements));
		static_cast<BlockStatement*>(block.get())->statements = blockStatements;
		statements.erase(it.base(), statements.end());
		statements.push_back(block);
		--scope;
	}
	void visitPost(const LabelStatement* stmt)
	{
	}
	void visitPost(const GotoStatement* stmt)
	{
	}
	StatementList getStatements() const { return statements; }
private:
	size_t scope = 0;
	StatementList statements;
	std::stack<StatementPtr> nodesStack;
};
