#pragma once

// Control Flow Flattening is first transformation phase 
// where compound statements are flattening to be used
// for low level or machine code generation

#include <stack>
#include "astvisitor.h"
#include "ast.h"

template<typename T>
bool is(const StatementPtr& p)
{
	return dynamic_cast<T*>(p.get());
}

template<typename T>
T* cast(const StatementPtr& p)
{
	return static_cast<T*>(p.get());
}


struct CFGFlattener : public AstVisitor
{	
	CFGFlattener() {}
	
	void visitPre(const BasicStatement*) {}
	void visitPre(const VarDecl* stmt)
	{
		auto node = makeNode<VarDecl>(scope, stmt->var_name);
		nodesStack.push(node);
	}
	void visitPre(const Expression* stmt)
	{
		auto node = makeNode<Expression>(Expression(scope));		
		if (is<Expression>(node)) {
			cast<Expression>(node)->elements = stmt->elements;
			cast<Expression>(node)->isPartOfCompoundStmt = stmt->isPartOfCompoundStmt;
		}
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
		cast<LabelStatement>(node)->label = stmt->label;
		nodesStack.push(node);
	}

	void visitPre(const GotoStatement* stmt) 
	{
		auto node = makeNode<GotoStatement>(GotoStatement(scope));
		cast<GotoStatement>(node)->label = stmt->label;
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

	std::string getNextLabel() 
	{
		std::string label = "__label__";
		label.append(std::to_string(id));
		id++;
		return label;
	}

	std::string getNextTempVariable()
	{
		std::string label = "__temp__";
		label.append(std::to_string(id));
		id++;
		return label;
	}


	void visitPost(const IfStatement* stmt)
	{
		auto ifstmt = nodesStack.top();
		nodesStack.pop();

		auto it = statements.rbegin();
		// get statement that is connected to if
		auto statement = *it;
		++it;
		Expression condition;
		auto ifStatementPtr = cast<IfStatement>(ifstmt);

		if (!is<Expression>(*it)) {
			// handle label statement
			auto labelPtr = ifStatementPtr->statements.begin();
			ifStatementPtr->statements.insert(labelPtr, *it);
			++it;
		}
		condition.elements = cast<Expression>(*it)->elements;
		ifStatementPtr->condition.isPartOfCompoundStmt = cast<Expression>(*it)->isPartOfCompoundStmt;
		++it;
		statements.erase(it.base(), statements.end());

		std::string temp = getNextTempVariable();

		statements.push_back(makeNode<VarDecl>(VarDecl(scope, temp)));

		// create reverse condition expression
		auto reverseCondition = makeNode<Expression>(Expression(scope, {temp, "="}));
		std::copy(condition.elements.begin(), condition.elements.end(), std::back_inserter(static_cast<Expression*>(reverseCondition.get())->elements));
		statements.push_back(reverseCondition);

		ifStatementPtr->condition.elements.insert(ifStatementPtr->condition.elements.end(), { "!", temp });

		std::string label = getNextLabel();

		// create goto statement
		ifStatementPtr->statements.push_back(makeNode<GotoStatement>(GotoStatement(scope, label)));
		statements.push_back(ifstmt);

		statements.push_back(statement);
		statements.push_back(makeNode<LabelStatement>(LabelStatement(scope, label)));
	}

	void visitPost(const WhileLoop* stmt)
	{
		auto loop = nodesStack.top();
		nodesStack.pop();
		auto it = statements.rbegin();
		cast<WhileLoop>(loop)->statements.push_back(*it);
		++it;
		if (!is<Expression>(*it)) {
			// handle label statement
			auto labelPtr = cast<WhileLoop>(loop)->statements.begin();
			cast<WhileLoop>(loop)->statements.insert(labelPtr, *it);
			++it;
		}
		cast<WhileLoop>(loop)->condition.elements = cast<Expression>(*it)->elements;
		cast<WhileLoop>(loop)->condition.isPartOfCompoundStmt = cast<Expression>(*it)->isPartOfCompoundStmt;
		++it;
		statements.erase(it.base(), statements.end());
		
		statements.push_back(loop);
		auto labelAfterStatement = makeNode<LabelStatement>(LabelStatement(scope));
		std::string label = getNextLabel();
		cast<LabelStatement>(labelAfterStatement)->label = label;
		statements.push_back(labelAfterStatement);
	}
	void visitPost(const BlockStatement* stmt)
	{		
		// TODO go up stack and check if there is if statement
		// or while at the top and if so
		// reduce all blocks except most outer one
		// otherwise reduce all
		auto block = nodesStack.top();
		nodesStack.pop();
	
		StatementList blockStatements;
		auto it = std::find_if(statements.rbegin(), statements.rend(), [&](const StatementPtr& s) {
			return s->scope != scope;
		});
		std::copy(it.base(), statements.end(), std::back_inserter(blockStatements));
		cast<BlockStatement>(block)->statements = blockStatements;
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
	size_t id = 0;
};
