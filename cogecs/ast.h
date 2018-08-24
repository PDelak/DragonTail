#ifndef LANGUAGE_AST_H
#define LANGUAGE_AST_H

#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <initializer_list>
#include "astvisitor.h"

struct Statement
{
	size_t scope = 0;
	Statement():scope(0) {}
	explicit Statement(size_t scope):scope(scope) {}
	virtual ~Statement() {}
	virtual void dump(size_t& depth, std::ostream& out) const = 0;
	// traverses whole ast tree 
	// accept visitor as parameter
	virtual void traverse(AstVisitor& visitor) = 0;
};


struct BasicStatement : public Statement
{
	explicit BasicStatement(size_t scope):Statement(scope) {}
	void dump(size_t& depth, std::ostream& out) const {}
	void traverse(AstVisitor& visitor) {}
};

std::string getTabs(size_t depth);

struct VarDecl : public Statement
{
	std::string var_name;
	VarDecl(size_t scope):Statement(scope) {}
	VarDecl(size_t scope, std::string var) :Statement(scope), var_name(var) {}
	void dump(size_t& depth, std::ostream& out) const {
		out << getTabs(depth);
		out << "Variable declaration" << std::endl;
		out << getTabs(depth + 1);
		out << "name:" << var_name << std::endl;
	}
	void traverse(AstVisitor& visitor) 
	{
		visitor.visitPre(this);
		visitor.visitPost(this);
	}
};

struct Expression : public Statement
{
	std::vector<std::string> elements;
	Expression() {}
	Expression(size_t scope):Statement(scope) {}
	Expression(size_t scope, const std::initializer_list<std::string>& elems) :Statement(scope), elements(elems.begin(), elems.end()) {}
	
	void dump(size_t& depth, std::ostream& out) const {
		out << getTabs(depth);
		out << "Expression" << std::endl;
		++depth;
		out << getTabs(depth);
		out << "elements size:" << elements.size() << std::endl;
		int index = 0;
		for (const auto e : elements) {
			out << getTabs(depth);
			if (!(index % 2))
				out << "var : " << e << std::endl;
			else
				out << "op  : " << e << std::endl;

			++index;
		}
		--depth;
	}
	void traverse(AstVisitor& visitor) 
	{
		visitor.visitPre(this);
		visitor.visitPost(this);
	}
};

using StatementPtr = std::shared_ptr<Statement>;
using StatementList = std::vector<StatementPtr>;

struct IfStatement : public Statement
{
	Expression condition;
	StatementList statements;
	IfStatement() {}
	IfStatement(size_t scope):Statement(scope) {}
	IfStatement(size_t scope, Expression expr, StatementList stmt) 
		:Statement(scope)
		,condition(expr)
		,statements(stmt)
	{}
	void dump(size_t& depth, std::ostream& out) const {
		out << getTabs(depth);
		out << "IfStatement" << std::endl;
		++depth;
		condition.dump(depth, out);
		for (auto const e : statements) {
			e->dump(depth, out);
		}
		--depth;
	}
	void traverse(AstVisitor& visitor) 
	{
		visitor.visitPre(this);
		condition.traverse(visitor);
		for (auto stmt : statements) {
			stmt->traverse(visitor);
		}
		visitor.visitPost(this);
	}
};

struct WhileLoop : public Statement
{
	Expression condition;
	StatementList statements;
	WhileLoop() {}
	WhileLoop(size_t scope) :Statement(scope) {}
	void dump(size_t& depth, std::ostream& out) const {
		std::cout << getTabs(depth);
		std::cout << "WhileLoop" << std::endl;
		++depth;
		condition.dump(depth, out);
		for (auto const e : statements) {
			e->dump(depth, out);
		}
		--depth;
	}
	void traverse(AstVisitor& visitor) 
	{
		visitor.visitPre(this);
		condition.traverse(visitor);
		for (auto stmt : statements) {
			stmt->traverse(visitor);
		}
		visitor.visitPost(this);
	}
};

struct BlockStatement : public Statement
{
	StatementList statements;
	BlockStatement() {}
	BlockStatement(size_t scope) :Statement(scope) {}
	void setStatements(const StatementList& stmts);
	
	void dump(size_t& depth, std::ostream& out) const {
		out << getTabs(depth);
		out << "BlockStatement" << std::endl;
		++depth;
		for (auto const e : statements) {
			e->dump(depth, out);
		}
		--depth;
	}
	void traverse(AstVisitor& visitor) 
	{
		visitor.visitPre(this);		
		for (auto stmt : statements) {
			stmt->traverse(visitor);
		}
		visitor.visitPost(this);
	}
};

struct LabelStatement : public Statement
{
	explicit LabelStatement(size_t scope) :Statement(scope) {}
	void dump(size_t& depth, std::ostream& out) const 
	{
		out << getTabs(depth);
		out << "Label" << std::endl;
		out << getTabs(depth + 1);
		out << "name:" << label << std::endl;

	}
	void traverse(AstVisitor& visitor) 
	{
		visitor.visitPre(this);
		visitor.visitPost(this);
	}
	std::string label;
};

struct GotoStatement : public Statement
{
	explicit GotoStatement(size_t scope) :Statement(scope) {}
	void dump(size_t& depth, std::ostream& out) const
	{
		out << getTabs(depth);
		out << "Goto" << std::endl;
		out << getTabs(depth + 1);
		out << "name:" << label << std::endl;

	}
	void traverse(AstVisitor& visitor)
	{
		visitor.visitPre(this);
		visitor.visitPost(this);
	}
	std::string label;
};

typedef std::vector<std::string> StatementStack;

void printAST(const StatementList& statementList);

void dumpAST(const StatementList& statementList, std::ostream& out);

void traverse(const StatementList& statementList, AstVisitor& visitor);

template<typename Node, typename... Params>
StatementPtr makeNode(Params&&... params) { return std::make_shared<Node>(Node(params...));}

#endif
