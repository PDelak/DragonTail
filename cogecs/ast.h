#ifndef LANGUAGE_AST_H
#define LANGUAGE_AST_H

#include <iostream>

struct Statement
{
	std::string vardecl;
};

typedef std::list<Statement> StatementList;

void printAST(const StatementList& statementList)
{
    for (auto e : statementList) {
		std::cout << e.vardecl << std::endl;
    }
}


#endif
