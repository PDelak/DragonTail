#ifndef LANGUAGE_AST_H
#define LANGUAGE_AST_H

#include <iostream>
#include <vector>

struct Statement
{
	size_t scope = 0;
	virtual ~Statement() {}
	virtual void dump(size_t& depth) const = 0;
};


struct BasicStatement : public Statement
{
	void dump(size_t& depth) const {}
};

std::string getTabs(size_t depth)
{
	std::string tab;
	for (int i = 0; i < depth; ++i)
		tab.append("\t");
	return tab;
}

struct VarDecl : public Statement
{
	std::string var_name;
	void dump(size_t& depth) const {
		std::cout << getTabs(depth);
		std::cout << "Variable declaration" << std::endl;
		std::cout << getTabs(depth+1);
		std::cout << "name:" << var_name << std::endl;
	}
};

struct Expression : public Statement
{
	std::vector<std::string> elements;
	void dump(size_t& depth) const {
		std::cout << getTabs(depth);
		std::cout << "Expression" << std::endl;
		++depth;
		std::cout << getTabs(depth);
		std::cout << "elements size:" << elements.size() << std::endl;
		if (elements.size() == 1) {
			std::cout << getTabs(depth);
			std::cout << "var : " << elements[0] << std::endl;
		}
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
	
};

typedef std::vector<std::shared_ptr<Statement>> StatementList;

struct IfStatement : public Statement
{
	Expression condition;
	StatementList statements;
	void dump(size_t& depth) const {
		std::cout << getTabs(depth);
		std::cout << "IfStatement" << std::endl;
		++depth;
		for (auto const e : statements) {
			e->dump(depth);
		}
		--depth;
	}

};

struct WhileLoop : public Statement
{
	Expression condition;
	StatementList statements;
	void dump(size_t& depth) const {
		std::cout << getTabs(depth);
		std::cout << "WhileLoop" << std::endl;
		++depth;
		for (auto const e : statements) {
			e->dump(depth);
		}
		--depth;
	}

};

struct BlockStatement : public Statement
{
	StatementList statements;
	void dump(size_t& depth) const {
		std::cout << getTabs(depth);
		std::cout << "BlockStatement" << std::endl;
		++depth;
		for (auto const e : statements) {
			e->dump(depth);
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


#endif
