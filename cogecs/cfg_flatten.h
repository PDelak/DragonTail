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
		auto node = makeNode(VarDecl(scope, stmt->var_name));
		nodesStack.push_back(node);
	}
	void visitPre(const BasicExpression*)
	{

	}
	void visitPre(const Expression* stmt)
	{
		auto node = makeNode(Expression(scope));		
		if (is<Expression>(node)) {
			cast<Expression>(node)->setElements(stmt->getChilds());
			cast<Expression>(node)->isPartOfCompoundStmt = stmt->isPartOfCompoundStmt;
		}
		nodesStack.push_back(node);

	}
	void visitPre(const IfStatement*)
	{
		auto node = makeNode(IfStatement(scope));
		nodesStack.push_back(node);
	}

	void visitPre(const WhileLoop*)
	{
		auto node = makeNode(WhileLoop(scope));
		nodesStack.push_back(node);
	}

	void visitPre(const BlockStatement*)
	{
		auto node = makeNode(BlockStatement(scope));
		nodesStack.push_back(node);
		++scope;
	}

	void visitPre(const LabelStatement* stmt)
	{
		auto node = makeNode(LabelStatement(scope, stmt->label));
		nodesStack.push_back(node);
	}

	void visitPre(const GotoStatement* stmt) 
	{
		auto node = makeNode(GotoStatement(scope, stmt->label));
		nodesStack.push_back(node);
	}

	void visitPre(const FunctionCall*)
	{

	}

	void visitPost(const BasicStatement*)
	{
	}

	void visitPost(const BasicExpression*)
	{
	}
	void visitPost(const VarDecl*)
	{
		if (nodesStack.empty()) return;
		auto begin = nodesStack.rbegin();
		auto node = *begin;
		statements.push_back(node);
		nodesStack.erase(std::next(begin).base());
	}
	void visitPost(const Expression*)
	{
		if (nodesStack.empty()) return;
		auto begin = nodesStack.rbegin();
		auto node = *begin;
		statements.push_back(node);
		nodesStack.erase(std::next(begin).base());
	}

	std::string getNextLabel() 
	{
		std::string label = "label__";
		label.append(std::to_string(id));
		id++;
		return label;
	}

	std::string getNextTempVariable()
	{
		std::string label = "temp__";
		label.append(std::to_string(id));
		id++;
		return label;
	}


	void visitPost(const IfStatement*)
	{
		if (nodesStack.empty()) return;
		auto begin = nodesStack.rbegin();
		auto if_statement = *begin;
		nodesStack.erase(std::next(begin).base());

		auto currentStatementIterator = statements.rbegin();
		// get statement that is connected to if
		
		std::vector<StatementPtr> if_children;
		if (is<BlockStatement>(*currentStatementIterator)) {
			auto block = cast<BlockStatement>(*currentStatementIterator);
			std::for_each(block->statements.begin(), block->statements.end(), [](const StatementPtr& stmt) {stmt->scope = stmt->scope - 1; });
			std::copy(block->statements.begin(), block->statements.end(), std::back_inserter(if_children));
		}
		else {
			if_children.push_back(*currentStatementIterator);
		}

		++currentStatementIterator;
		
		if (!is<Expression>(*currentStatementIterator)) {
			// handle label statement, that's follow case
			// if (a) label_0: {}
			if_children.insert(if_children.begin(), *currentStatementIterator);
			++currentStatementIterator;
		}

		Expression condition;
		condition.setElements(cast<Expression>(*currentStatementIterator)->getChilds());
		cast<IfStatement>(if_statement)->condition.isPartOfCompoundStmt = cast<Expression>(*currentStatementIterator)->isPartOfCompoundStmt;
		++currentStatementIterator;
		statements.erase(currentStatementIterator.base(), statements.end());

		std::string temp = getNextTempVariable();

		statements.push_back(makeNode(VarDecl(scope, temp)));

		// create reverse condition expression
		auto reverseCondition = makeNode(Expression(scope, {makeNode(BasicExpression(scope, temp)), makeNode(BasicExpression(scope, "="))}));
		
		std::copy(condition.child_begin(), condition.child_end(), std::back_inserter(static_cast<Expression*>(reverseCondition.get())->getChilds()));
		statements.push_back(reverseCondition);

		cast<IfStatement>(if_statement)->condition.insertChild(cast<IfStatement>(if_statement)->condition.child_end(), { makeNode(BasicExpression(scope, "!")), makeNode(BasicExpression(scope, temp)) });

		std::string label = getNextLabel();

		// create goto statement
		// goto statements scope needs to be 0 as everything is flatten
		constexpr size_t gotoStatementScope = 0;
		cast<IfStatement>(if_statement)->statements.push_back(makeNode(GotoStatement(gotoStatementScope, label)));
		statements.push_back(if_statement);

		std::copy(if_children.begin(), if_children.end(), std::back_inserter(statements));

		statements.push_back(makeNode(LabelStatement(scope, label)));
	}

	void visitPost(const WhileLoop*)
	{
		if (nodesStack.empty()) return;
		auto begin = nodesStack.rbegin();
		nodesStack.erase(std::next(begin).base());
		auto if_statement = makeNode(IfStatement(scope));

		auto currentStatementIterator = statements.rbegin();
		
		std::vector<StatementPtr> while_children;
		if (is<BlockStatement>(*currentStatementIterator)) {
			auto block = cast<BlockStatement>(*currentStatementIterator);
			std::for_each(block->statements.begin(), block->statements.end(), [](const StatementPtr& stmt) {stmt->scope = stmt->scope - 1; });
			std::copy(block->statements.begin(), block->statements.end(), std::back_inserter(while_children));
		}
		else { while_children.push_back(*currentStatementIterator); }

		++currentStatementIterator;
		
		if (!is<Expression>(*currentStatementIterator)) {
			// handle label statement, that's follow case
			// while (a) label_0: {}
			while_children.insert(while_children.begin(), *currentStatementIterator);
			++currentStatementIterator;			
		}
		
		Expression condition = *cast<Expression>(*currentStatementIterator);
			
		cast<IfStatement>(if_statement)->scope = cast<Expression>(*currentStatementIterator)->scope;
		cast<IfStatement>(if_statement)->condition.isPartOfCompoundStmt = cast<Expression>(*currentStatementIterator)->isPartOfCompoundStmt;

		++currentStatementIterator;
		statements.erase(currentStatementIterator.base(), statements.end());
		
		std::string temp = getNextTempVariable();
		statements.push_back(makeNode(VarDecl(scope, temp)));

		// create reverse condition expression		
		auto labelBeforeIfNode = makeNode(LabelStatement(scope));
		cast<LabelStatement>(labelBeforeIfNode)->label = getNextLabel();
		statements.push_back(labelBeforeIfNode);

		auto reverseCondition = makeNode(Expression(scope, { makeNode(BasicExpression(scope, temp)), makeNode(BasicExpression(scope, "=")) }));
		std::copy(condition.child_begin(), condition.child_end(), std::back_inserter(static_cast<Expression*>(reverseCondition.get())->getChilds()));
		
		statements.push_back(reverseCondition);
		
		cast<IfStatement>(if_statement)->condition.insertChild(cast<IfStatement>(if_statement)->condition.child_end(), { makeNode(BasicExpression(scope, "!")), makeNode(BasicExpression(scope, temp)) });

		std::string label = getNextLabel();
		// goto statements scope needs to be 0 as everything is flatten
		constexpr size_t gotoStatementScope = 0;
		cast<IfStatement>(if_statement)->statements.push_back(makeNode(GotoStatement(gotoStatementScope, label)));
		
		statements.push_back(if_statement);
		std::copy(while_children.begin(), while_children.end(), std::back_inserter(statements));
		statements.push_back(makeNode(GotoStatement(scope, cast<LabelStatement>(labelBeforeIfNode)->label)));
		statements.push_back(makeNode(LabelStatement(scope, label)));
	}
	void visitPost(const BlockStatement*)
	{		
		// reduce all blocks except most outer one
		// otherwise reduce all		
		if (nodesStack.empty()) return;
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
	void visitPost(const LabelStatement*)
	{
		if (nodesStack.empty()) return;
		auto begin = nodesStack.rbegin();
		auto node = *begin;
		statements.push_back(node);
		nodesStack.erase(std::next(begin).base());

	}
	void visitPost(const GotoStatement*)
	{
		if (nodesStack.empty()) return;
		auto begin = nodesStack.rbegin();
		auto node = *begin;
		statements.push_back(node);
		nodesStack.erase(std::next(begin).base());
	}
	void visitPost(const FunctionCall*)
	{

	}

	StatementList getStatements() const { return statements; }
private:
	size_t scope = 0;
	StatementList statements;
	std::vector<StatementPtr> nodesStack;
	size_t id = 0;
};
