#ifndef LANGUAGE_AST_H
#define LANGUAGE_AST_H

#include <iostream>
#include <vector>

struct Statement
{
	size_t scope = 0;
	virtual ~Statement() {}
	virtual void dump() const = 0;
};


struct BasicStatement : public Statement
{
	void dump() const {}
};

struct VarDecl : public Statement
{
	std::string var_name;
	void dump() const {
		std::cout << "Variable declaration" << std::endl;
		std::cout << "\tname:" << var_name << std::endl;
	}
};

struct Expression : public Statement
{
	std::vector<std::string> elements;
	void dump() const {
		std::cout << "Expression" << std::endl;
		std::cout << "\telements size:" << elements.size() << std::endl;
		if (elements.size() == 1) {
			std::cout << "\tvar : " << elements[0] << std::endl;
		}
		int index = 0;
		for (const auto e: elements) {
				if (!(index % 2))
				std::cout << "\tvar : " << e << std::endl;
				else 
				std::cout << "\top  : " << e << std::endl;
				
				++index;
		}
	}
	
};

typedef std::vector<std::shared_ptr<Statement>> StatementList;

struct IfStatement : public Statement
{
	StatementList statements;
	void dump() const {
		std::cout << "IfStatement" << std::endl;
		for (auto const e : statements) {
			e->dump();
		}
	}

};

typedef std::vector<std::shared_ptr<Statement>> StatementList;
typedef std::vector<std::string> StatementStack;

void printAST(const StatementList& statementList)
{
	auto begin = statementList.begin();
	for (; begin != statementList.end(); ++begin) {
		(*begin)->dump();
	}
}


#endif
