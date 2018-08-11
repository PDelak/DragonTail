#ifndef LANGUAGE_AST_H
#define LANGUAGE_AST_H

#include <iostream>

struct Statement
{
	virtual ~Statement() {}
	virtual void dump() const = 0;
};

struct BasicStatement : public Statement
{
	void dump() const {}
};

struct VarDecl : public Statement
{
	std::string vardecl;
	void dump() const {
		std::cout << "Variable declaration" << std::endl;
		std::cout << "\tname:" << vardecl << std::endl;
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

typedef std::list<std::shared_ptr<Statement>> StatementList;

void printAST(const StatementList& statementList)
{
	auto begin = statementList.begin();
	for (; begin != statementList.end(); ++begin) {
		(*begin)->dump();
	}
}


#endif
