#ifndef LANGUAGE_AST_H
#define LANGUAGE_AST_H

#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <initializer_list>

struct Statement
{
	size_t scope = 0;
	Statement():scope(0) {}
	explicit Statement(size_t scope):scope(scope) {}
	virtual ~Statement() {}
	virtual void dump(size_t& depth) const = 0;
	virtual void dump(size_t& depth, std::ostream& out) const = 0;
};


struct BasicStatement : public Statement
{
	explicit BasicStatement(size_t scope):Statement(scope) {}
	void dump(size_t& depth) const {}
	void dump(size_t& depth, std::ostream& out) const {}
};

std::string getTabs(size_t depth)
{
	std::string tab;
	for (size_t i = 0; i < depth; ++i)
		tab.append("\t");
	return tab;
}

struct VarDecl : public Statement
{
	std::string var_name;
	VarDecl(size_t scope):Statement(scope) {}
	VarDecl(size_t scope, std::string var) :Statement(scope), var_name(var) {}
	void dump(size_t& depth) const {
		std::cout << getTabs(depth);
		std::cout << "Variable declaration" << std::endl;
		std::cout << getTabs(depth+1);
		std::cout << "name:" << var_name << std::endl;
	}
	void dump(size_t& depth, std::ostream& out) const {
		out << getTabs(depth);
		out << "Variable declaration" << std::endl;
		out << getTabs(depth + 1);
		out << "name:" << var_name << std::endl;
	}
};

struct Expression : public Statement
{
	std::vector<std::string> elements;
	Expression() {}
	Expression(size_t scope):Statement(scope) {}
	void dump(size_t& depth) const {
		std::cout << getTabs(depth);
		std::cout << "Expression" << std::endl;
		++depth;
		std::cout << getTabs(depth);
		std::cout << "elements size:" << elements.size() << std::endl;
		int index = 0;
		for (const auto e: elements) {
				std::cout << getTabs(depth);
				if (!(index % 2))
				std::cout << "var : " << e << std::endl;
				else 
				std::cout << "op  : " << e << std::endl;
				
				++index;
		}
		--depth;
	}
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
};

using StatementPtr = std::shared_ptr<Statement>;
using StatementList = std::vector<StatementPtr>;

struct IfStatement : public Statement
{
	Expression condition;
	StatementList statements;
	IfStatement() {}
	IfStatement(size_t scope):Statement(scope) {}
	void dump(size_t& depth) const {
		std::cout << getTabs(depth);
		std::cout << "IfStatement" << std::endl;
		++depth;
		condition.dump(depth);
		for (auto const e : statements) {
			e->dump(depth);
		}
		--depth;
	}
	void dump(size_t& depth, std::ostream& out) const {
		out << getTabs(depth);
		out << "IfStatement" << std::endl;
		++depth;
		condition.dump(depth);
		for (auto const e : statements) {
			e->dump(depth, out);
		}
		--depth;
	}

};

struct WhileLoop : public Statement
{
	Expression condition;
	StatementList statements;
	WhileLoop() {}
	WhileLoop(size_t scope) :Statement(scope) {}
	void dump(size_t& depth) const {
		std::cout << getTabs(depth);
		std::cout << "WhileLoop" << std::endl;
		++depth;
		condition.dump(depth);
		for (auto const e : statements) {
			e->dump(depth);
		}
		--depth;
	}
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
};

struct BlockStatement : public Statement
{
	StatementList statements;
	BlockStatement() {}
	BlockStatement(size_t scope) :Statement(scope) {}

	void dump(size_t& depth) const {
		std::cout << getTabs(depth);
		std::cout << "BlockStatement" << std::endl;
		++depth;
		for (auto const e : statements) {
			e->dump(depth);
		}
		--depth;
	}
	void dump(size_t& depth, std::ostream& out) const {
		out << getTabs(depth);
		out << "BlockStatement" << std::endl;
		++depth;
		for (auto const e : statements) {
			e->dump(depth, out);
		}
		--depth;
	}

};


typedef std::vector<std::string> StatementStack;

void printAST(const StatementList& statementList)
{
	size_t depth = 1;
	auto begin = statementList.begin();
	for (; begin != statementList.end(); ++begin) {
		(*begin)->dump(depth);
	}
}

void dumpAST(const StatementList& statementList, std::ostream& out)
{
	size_t depth = 1;
	auto begin = statementList.begin();
	for (; begin != statementList.end(); ++begin) {
		(*begin)->dump(depth, out);
	}
}


template<typename Node, typename... Params>
StatementPtr makeNode(Params&&... params) { return std::make_shared<Node>(params...);}

#endif
