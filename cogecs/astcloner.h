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
		auto node = makeNode<VarDecl>(scope, stmt->var_name);
		nodesStack.push(node);
	}
	void visitPre(const Expression* stmt) 
	{
		auto node = makeNode<Expression>(Expression(scope));
		static_cast<Expression*>(node.get())->elements = stmt->elements;
		static_cast<Expression*>(node.get())->isPartOfCompoundStmt = stmt->isPartOfCompoundStmt;
		nodesStack.push(node);

	}
	void visitPre(const IfStatement* stmt) 
	{
		auto node = makeNode<IfStatement>(IfStatement(scope));
		nodesStack.push(node);
	}

	void visitPre(const WhileLoop*) 
	{
		auto node = makeNode<WhileLoop>(WhileLoop(scope));
		nodesStack.push(node);
	}

	void visitPre(const BlockStatement* stmt) 
	{
		auto node = makeNode<BlockStatement>(BlockStatement(scope));
		nodesStack.push(node);
		++scope;
	}

	void visitPre(const LabelStatement* stmt) 
	{
		auto node = makeNode<LabelStatement>(LabelStatement(scope));		
		static_cast<LabelStatement*>(node.get())->label = stmt->label;
		nodesStack.push(node);
	}

	void visitPre(const GotoStatement* stmt) 
	{
		auto node = makeNode<GotoStatement>(GotoStatement(scope));
		static_cast<GotoStatement*>(node.get())->label = stmt->label;
		nodesStack.push(node);
	}

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
		auto ifstmt = nodesStack.top();
		nodesStack.pop();
		auto it = statements.rbegin();		
		static_cast<IfStatement*>(ifstmt.get())->statements.push_back(*it);
		++it;
		if (dynamic_cast<Expression*>(it->get())) {
			static_cast<IfStatement*>(ifstmt.get())->condition.elements = static_cast<Expression*>(it->get())->elements;
			static_cast<IfStatement*>(ifstmt.get())->condition.isPartOfCompoundStmt = static_cast<Expression*>(it->get())->isPartOfCompoundStmt;
		}
		else {
			// handle label statement
			auto labelPtr = static_cast<IfStatement*>(ifstmt.get())->statements.begin();
			static_cast<IfStatement*>(ifstmt.get())->statements.insert(labelPtr ,*it);
			++it;
			static_cast<IfStatement*>(ifstmt.get())->condition.elements = static_cast<Expression*>(it->get())->elements;
			static_cast<IfStatement*>(ifstmt.get())->condition.isPartOfCompoundStmt = static_cast<Expression*>(it->get())->isPartOfCompoundStmt;
		}
		++it;
		statements.erase(it.base(), statements.end());
		statements.push_back(ifstmt);
	}
	void visitPost(const WhileLoop* stmt) 
	{
		auto loop = nodesStack.top();
		nodesStack.pop();
		auto it = statements.rbegin();
		static_cast<WhileLoop*>(loop.get())->statements.push_back(*it);
		++it;
		if (dynamic_cast<Expression*>(it->get())) {
			static_cast<WhileLoop*>(loop.get())->condition.elements = static_cast<Expression*>(it->get())->elements;
			static_cast<WhileLoop*>(loop.get())->condition.isPartOfCompoundStmt = static_cast<Expression*>(it->get())->isPartOfCompoundStmt;

		}
		else {
			// handle label statement
			auto labelPtr = static_cast<WhileLoop*>(loop.get())->statements.begin();
			static_cast<WhileLoop*>(loop.get())->statements.insert(labelPtr, *it);
			++it;
			static_cast<WhileLoop*>(loop.get())->condition.elements = static_cast<Expression*>(it->get())->elements;
			static_cast<WhileLoop*>(loop.get())->condition.isPartOfCompoundStmt = static_cast<Expression*>(it->get())->isPartOfCompoundStmt;
		}
		++it;
		statements.erase(it.base(), statements.end());
		statements.push_back(loop);

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
		auto node = nodesStack.top();		
		statements.push_back(node);
		nodesStack.pop();
	}
	void visitPost(const GotoStatement* stmt)
	{
		auto node = nodesStack.top();
		statements.push_back(node);
		nodesStack.pop();
	}
	StatementList getStatements() const { return statements; }
private:
	size_t scope = 0;
	StatementList statements;
	std::stack<StatementPtr> nodesStack;
};
