#ifndef LANGUAGE_AST_H
#define LANGUAGE_AST_H

#include <iostream>

struct Statement
{
 
};

typedef std::list<Statement> StatementList;

void printAST(const StatementList& statementList)
{
    for (auto e : statementList) {
    }
}


#endif
