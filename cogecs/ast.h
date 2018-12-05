#ifndef LANGUAGE_AST_H
#define LANGUAGE_AST_H

#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <initializer_list>
#include <algorithm>
#include "astvisitor.h"

std::ostream& operator << (std::ostream& stream, const VarDecl& varDecl);
std::ostream& operator << (std::ostream& stream, const Expression& expression);
std::ostream& operator << (std::ostream& stream, const IfStatement& ifStatement);
std::ostream& operator << (std::ostream& stream, const WhileLoop& loop);
std::ostream& operator << (std::ostream& stream, const BlockStatement& block);
std::ostream& operator << (std::ostream& stream, const LabelStatement& label);
std::ostream& operator << (std::ostream& stream, const GotoStatement& gotoStatement);
std::ostream& operator << (std::ostream& stream, const FunctionCall& functionCall);
std::ostream& operator << (std::ostream& stream, const BasicExpression& basicExpression);
std::ostream& operator << (std::ostream& stream, const ReturnStatement& returnStatement);
std::ostream& operator << (std::ostream& stream, const FunctionDecl& functionDecl);

struct Statement
{
	size_t scope = 0;
	Statement():scope(0) {}
	explicit Statement(size_t scope):scope(scope) {}
	virtual ~Statement() {}
	virtual void dump(size_t& depth, std::ostream& out) const = 0;
	virtual void text(std::ostream& out) const = 0;
	// traverses whole ast tree 
	// accept visitor as parameter
	virtual void traverse(AstVisitor& visitor) = 0;
};

using StatementPtr = std::shared_ptr<Statement>;
using StatementList = std::vector<StatementPtr>;

std::string getTabs(size_t depth);

struct BasicStatement : public Statement
{
	explicit BasicStatement(size_t scope):Statement(scope) {}
	void dump(size_t&, std::ostream&) const {}
	virtual void text(std::ostream&) const {}
	void traverse(AstVisitor&) {}
};

struct BasicExpression : public BasicStatement
{
	BasicExpression(size_t scope, const std::string& v) :BasicStatement(scope), value(v) {}
	void dump(size_t&,  std::ostream& out) const 
	{
		out << value;
	}
	virtual void text(std::ostream& out) const 
	{
		out << *this;
	}
	void traverse(AstVisitor& visitor) 
	{
		visitor.visitPre(this);
		visitor.visitPost(this);
	}
	std::string value;
};


struct VarDecl : public Statement
{
	std::string var_name;
	VarDecl(size_t scope):Statement(scope) {}
	VarDecl(size_t scope, std::string var) :Statement(scope), var_name(var) {}
	void dump(size_t& depth, std::ostream& out) const {
		out << getTabs(depth);
		out << "Variable declaration(" << "scope:" << scope << ")" << std::endl;
		out << getTabs(depth + 1);
		out << "name:" << var_name << std::endl;
	}
	virtual void text(std::ostream& out) const {
		out << *this;
	}
	void traverse(AstVisitor& visitor) 
	{
		visitor.visitPre(this);
		visitor.visitPost(this);
	}
};

struct Expression : public Statement
{
	using ElementType = StatementPtr;
	using ElementsType = std::vector<ElementType>;
	using ElementIterator = std::vector<ElementType>::iterator;
	using ElementConstIterator = std::vector<ElementType>::const_iterator;
private:
	ElementsType elements;
public:
	const ElementsType& getChilds() const { return elements; }
	ElementsType& getChilds() { return elements; }
	void setElements(const ElementsType& elems) { elements = elems; }
	ElementConstIterator child_begin() const { return elements.begin(); }
	ElementConstIterator child_end()   const { return elements.end(); }
	ElementIterator child_begin()  { return elements.begin(); }
	ElementIterator child_end()    { return elements.end(); }

	void insertChild(ElementIterator iterator, const std::initializer_list<ElementType>& childs) { elements.insert(iterator, childs); }

	Expression() {}
	Expression(size_t scope):Statement(scope) {}
	Expression(size_t scope, const std::initializer_list<ElementType>& elems) :Statement(scope), elements(elems.begin(), elems.end()) {}
	
	void dump(size_t& depth, std::ostream& out) const {
		out << getTabs(depth);
		out << "Expression(" << "scope:" << scope << ")" << std::endl;
		++depth;
		out << getTabs(depth);
		out << "elements size:" << elements.size() << std::endl;
		int index = 0;
		for (const auto e : elements) {
			out << getTabs(depth);
			if (!(index % 2)) {
				out << "var : ";
				e->dump(depth, out);
				out << std::endl;
			}
			else {
				out << "op  : ";
				e->dump(depth, out);
				out << std::endl;
			}
			++index;
		}
		--depth;
	}
	virtual void text(std::ostream& out) const {
		out << *this;
	}
	void traverse(AstVisitor& visitor)
	{
		visitor.visitPre(this);
		visitor.visitPost(this);
	}
	bool isPartOfCompoundStmt = false;
};


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
	virtual void text(std::ostream& out) const {
		out << *this;
	}
	void dump(size_t& depth, std::ostream& out) const {
		out << getTabs(depth);
		out << "IfStatement" << "(" << "scope:" << scope << ")" << std::endl;
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
	WhileLoop(size_t scope, Expression expr, StatementList stmt)
		:Statement(scope)
		, condition(expr)
		, statements(stmt)
	{}

	void dump(size_t& depth, std::ostream& out) const {
		std::cout << getTabs(depth);
		std::cout << "WhileLoop" << "(" << "scope:" << scope << ")" << std::endl;
		++depth;
		condition.dump(depth, out);
		for (auto const e : statements) {
			e->dump(depth, out);
		}
		--depth;
	}
	virtual void text(std::ostream& out) const {
		out << *this;
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
	BlockStatement(size_t scope, StatementList stmt)
		:Statement(scope)
		,statements(stmt)
	{}

	void dump(size_t& depth, std::ostream& out) const {
		out << getTabs(depth);
		out << "BlockStatement" << "(" << "scope:" << scope << ")" << std::endl;
		++depth;
		for (auto const e : statements) {
			e->dump(depth, out);
		}
		--depth;
	}
	virtual void text(std::ostream& out) const {
		out << *this;
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
	LabelStatement(size_t scope, const std::string& label) :Statement(scope), label(label) {}
	void dump(size_t& depth, std::ostream& out) const 
	{
		out << getTabs(depth);
		out << "Label" << "(" << "scope:" << scope << ")" << std::endl;
		out << getTabs(depth + 1);
		out << "name:" << label << std::endl;

	}
	virtual void text(std::ostream& out) const {
		out << *this;
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
	GotoStatement(size_t scope, const std::string& label) :Statement(scope), label(label) {}
	void dump(size_t& depth, std::ostream& out) const
	{
		out << getTabs(depth);
		out << "Goto" << "(" << "scope:" << scope << ")" << std::endl; 
		out << getTabs(depth + 1);
		out << "name:" << label << std::endl;

	}
	virtual void text(std::ostream& out) const {
		out << *this;
	}
	void traverse(AstVisitor& visitor)
	{
		visitor.visitPre(this);
		visitor.visitPost(this);
	}
	std::string label;
};

struct FunctionCall : public Statement
{
	std::string name;
	std::vector<std::string> parameters;
	FunctionCall() {}
	FunctionCall(size_t scope) :Statement(scope) {}
	virtual void dump(size_t& depth, std::ostream& out) const 
	{
		out << "FunctionCall" << "(" << "scope:" << scope << ")" << std::endl;
		out << getTabs(depth + 1);
		out << "name:" << name.c_str() << std::endl;
		
		for (auto param : parameters) {
			out << getTabs(depth + 1) << "param : " << param << std::endl;
		}
	}
	virtual void text(std::ostream& out) const 
	{
		out << *this;
	}
	virtual void traverse(AstVisitor& visitor) 
	{
		visitor.visitPre(this);
		visitor.visitPost(this);
	}
};

struct ReturnStatement : public Statement
{
	std::string param;
	ReturnStatement() {}
	ReturnStatement(size_t scope) :Statement(scope) {}
	ReturnStatement(size_t scope, std::string p) :Statement(scope), param(p) {}
	virtual void dump(size_t& depth, std::ostream& out) const
	{
		out << getTabs(depth);
		out << "Return Statement" << "(" << "scope:" << scope << ")" << std::endl;
		out << getTabs(depth + 1);
		out << "param:" << param.c_str() << std::endl;
	}
	virtual void text(std::ostream& out) const
	{
		out << *this;
	}
	virtual void traverse(AstVisitor& visitor)
	{
		visitor.visitPre(this);
		visitor.visitPost(this);
	}
};

struct FunctionDecl : public Statement
{
	std::string name;
	std::vector<std::string> parameters;
	StatementList statements;
	FunctionDecl() {}
	FunctionDecl(size_t scope) :Statement(scope) {}
	FunctionDecl(size_t scope, std::string name) :Statement(scope), name(name) {}
	virtual void dump(size_t& depth, std::ostream& out) const
	{
		out << getTabs(depth);
		out << "FunctionDecl" << "(" << "scope:" << scope << ")" << std::endl;
		out << getTabs(depth + 1);
		out << "name:" << name.c_str() << std::endl;

		for (auto param : parameters) {
			out << getTabs(depth + 1) << "param : " << param << std::endl;
		}
	}
	virtual void text(std::ostream& out) const
	{
		out << *this;
	}
	virtual void traverse(AstVisitor& visitor)
	{
		visitor.visitPre(this);
		visitor.visitPost(this);
	}
};

typedef std::vector<std::string> StatementStack;

void printAST(const StatementList& statementList);

void dumpAST(const StatementList& statementList, std::ostream& out);

void dumpCode(const StatementList& statementList, std::ostream& out);

void traverse(const StatementList& statementList, AstVisitor& visitor);

template<typename Node>
StatementPtr makeNode(Node&& node) { return std::make_shared<Node>(Node(node));}


#endif
