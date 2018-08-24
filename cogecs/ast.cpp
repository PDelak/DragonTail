#include "ast.h"

void BlockStatement::setStatements(const StatementList& stmts)
{
	statements = stmts;
}

void printAST(const StatementList& statementList)
{
	size_t depth = 1;
	auto begin = statementList.begin();
	for (; begin != statementList.end(); ++begin) {
		(*begin)->dump(depth, std::cout);
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

void traverse(const StatementList& statementList, AstVisitor& visitor)
{
	for (auto stmt : statementList) {
		stmt->traverse(visitor);
	}
}

std::string getTabs(size_t depth)
{
	std::string tab;
	for (size_t i = 0; i < depth; ++i)
		tab.append("\t");
	return tab;
}
