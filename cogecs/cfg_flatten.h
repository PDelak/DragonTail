#pragma once

// Control Flow Flattening is first transformation phase 
// where compound statements are flattening to be used
// for low level or machine code generation

#include <stack>
#include <vector>
#include "astvisitor.h"
#include "ast.h"
#include "tools.h"

struct CFGFlattener : public AstVisitor
{	
	CFGFlattener() {}
	
	void visitPre(const BasicStatement*) {}
	void visitPre(const VarDecl* stmt)
	{
		auto node = makeNode<VarDecl>(scope, stmt->var_name);
		nodesStack.push_back(node);
	}
	void visitPre(const Expression* stmt)
	{
		auto node = makeNode<Expression>(Expression(scope));		
		if (is<Expression>(node)) {
			cast<Expression>(node)->elements = stmt->elements;
			cast<Expression>(node)->isPartOfCompoundStmt = stmt->isPartOfCompoundStmt;
		}
		nodesStack.push_back(node);

	}
	void visitPre(const IfStatement* stmt)
	{
		auto node = makeNode<IfStatement>(IfStatement(scope));
		nodesStack.push_back(node);
	}

	void visitPre(const WhileLoop*)
	{
		auto node = makeNode<WhileLoop>(WhileLoop(scope));
		nodesStack.push_back(node);
	}

	void visitPre(const BlockStatement* stmt)
	{
		auto node = makeNode<BlockStatement>(BlockStatement(scope));
		nodesStack.push_back(node);
		++scope;
	}

	void visitPre(const LabelStatement* stmt)
	{
		auto node = makeNode<LabelStatement>(LabelStatement(scope));
		cast<LabelStatement>(node)->label = stmt->label;
		nodesStack.push_back(node);
	}

	void visitPre(const GotoStatement* stmt) 
	{
		auto node = makeNode<GotoStatement>(GotoStatement(scope));
		cast<GotoStatement>(node)->label = stmt->label;
		nodesStack.push_back(node);
	}

	void visitPost(const BasicStatement* stmt)
	{
	}
	void visitPost(const VarDecl* stmt)
	{
		auto begin = nodesStack.rbegin();
		auto node = *begin;
		statements.push_back(node);
		nodesStack.erase(std::next(begin).base());
	}
	void visitPost(const Expression* stmt)
	{
		auto begin = nodesStack.rbegin();
		auto node = *begin;
		statements.push_back(node);
		nodesStack.erase(std::next(begin).base());
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
		auto begin = nodesStack.rbegin();
		auto ifstmt = *begin;
		nodesStack.erase(std::next(begin).base());

		auto it = statements.rbegin();
		// get statement that is connected to if
		auto statement = *it;
		std::vector<StatementPtr> if_children;
		if (is<BlockStatement>(statement)) {
			auto block = cast<BlockStatement>(statement);
			std::for_each(block->statements.begin(), block->statements.end(), [](const StatementPtr& stmt) {stmt->scope = stmt->scope - 1; });
			std::copy(block->statements.begin(), block->statements.end(), std::back_inserter(if_children));
		}
		else {
			if_children.push_back(statement);
		}

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

		std::copy(if_children.begin(), if_children.end(), std::back_inserter(statements));
		
		statements.push_back(makeNode<LabelStatement>(LabelStatement(scope, label)));
	}

	void visitPost(const WhileLoop* stmt)
	{
		auto begin = nodesStack.rbegin();
		//auto loop = *begin;
		nodesStack.erase(std::next(begin).base());
		auto if_statement = makeNode<IfStatement>(IfStatement(scope));

		auto it = statements.rbegin();
		auto statement = *it;
		std::vector<StatementPtr> while_children;
		if (is<BlockStatement>(statement)) {
			auto block = cast<BlockStatement>(statement);
			std::for_each(block->statements.begin(), block->statements.end(), [](const StatementPtr& stmt) {stmt->scope = stmt->scope - 1; });
			std::copy(block->statements.begin(), block->statements.end(), std::back_inserter(while_children));
		}
		else {
			while_children.push_back(statement);
		}

		cast<IfStatement>(if_statement)->statements.push_back(*it);
		++it;
		if (!is<Expression>(*it)) {
			// handle label statement
			auto labelPtr = cast<IfStatement>(if_statement)->statements.begin();
			cast<IfStatement>(if_statement)->statements.insert(labelPtr, *it);
			++it;
		}
		Expression condition;

		cast<IfStatement>(if_statement)->condition.elements = cast<Expression>(*it)->elements;
		condition.elements = cast<IfStatement>(if_statement)->condition.elements;

		cast<IfStatement>(if_statement)->condition.isPartOfCompoundStmt = cast<Expression>(*it)->isPartOfCompoundStmt;
		condition.isPartOfCompoundStmt = cast<Expression>(*it)->isPartOfCompoundStmt;
		++it;
		statements.erase(it.base(), statements.end());
		
		std::string temp = getNextTempVariable();
		statements.push_back(makeNode<VarDecl>(VarDecl(scope, temp)));
		// create reverse condition expression
		
		auto labelBeforeIfNode = makeNode<LabelStatement>(LabelStatement(scope));
		cast<LabelStatement>(labelBeforeIfNode)->label = getNextLabel();
		statements.push_back(labelBeforeIfNode);

		auto reverseCondition = makeNode<Expression>(Expression(scope, { temp, "=" }));
		std::copy(condition.elements.begin(), condition.elements.end(), std::back_inserter(static_cast<Expression*>(reverseCondition.get())->elements));
		statements.push_back(reverseCondition);
		cast<IfStatement>(if_statement)->condition.elements.clear();
		cast<IfStatement>(if_statement)->condition.elements.insert(cast<IfStatement>(if_statement)->condition.elements.end(), { "!", temp });
		cast<IfStatement>(if_statement)->statements.clear();
		std::string label = getNextLabel();
		cast<IfStatement>(if_statement)->statements.push_back(makeNode<GotoStatement>(GotoStatement(scope, label)));
		
		statements.push_back(if_statement);
		std::copy(while_children.begin(), while_children.end(), std::back_inserter(statements));
		statements.push_back(makeNode<GotoStatement>(GotoStatement(scope, cast<LabelStatement>(labelBeforeIfNode)->label)));
		auto labelAfterStatement = makeNode<LabelStatement>(LabelStatement(scope));
		cast<LabelStatement>(labelAfterStatement)->label = label;
		statements.push_back(labelAfterStatement);
	}
	void visitPost(const BlockStatement* stmt)
	{		
		// reduce all blocks except most outer one
		// otherwise reduce all		
		auto begin = nodesStack.rbegin();
		auto block = *begin;
		auto previous = nodesStack.erase(std::next(begin).base());
		if (previous != nodesStack.begin()) --previous;
		if (!nodesStack.empty() && (is<IfStatement>(*previous) || is<WhileLoop>(*previous))) {
			StatementList blockStatements;
			auto it = std::find_if(statements.rbegin(), statements.rend(), [&](const StatementPtr& s) {
				return s->scope != scope;
			});
			std::copy(it.base(), statements.end(), std::back_inserter(blockStatements));
			cast<BlockStatement>(block)->statements = blockStatements;

			statements.erase(it.base(), statements.end());
			statements.push_back(block);
		}
		else {
			// if previous is block statement as well
			// update all statements that are part of current block 			
			auto it = std::find_if(statements.rbegin(), statements.rend(), [&](const StatementPtr& s) {
				return s->scope != scope;
			});
			std::for_each(it.base(), statements.end(), [](const StatementPtr& stmt) {
				stmt->scope = stmt->scope - 1;
			});
		}
		--scope;
									
	}
	void visitPost(const LabelStatement* stmt)
	{
		auto begin = nodesStack.rbegin();
		auto node = *begin;
		statements.push_back(node);
		nodesStack.erase(std::next(begin).base());

	}
	void visitPost(const GotoStatement* stmt)
	{
		auto begin = nodesStack.rbegin();
		auto node = *begin;
		statements.push_back(node);
		nodesStack.erase(std::next(begin).base());
	}
	StatementList getStatements() const { return statements; }
private:
	size_t scope = 0;
	StatementList statements;
	std::vector<StatementPtr> nodesStack;
	size_t id = 0;
};
