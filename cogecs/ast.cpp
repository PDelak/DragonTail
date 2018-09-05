#include "ast.h"

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

void dumpCode(const StatementList& statementList, std::ostream& out)
{	
	auto begin = statementList.begin();
	for (; begin != statementList.end(); ++begin) {
		(*begin)->text(out);
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

std::ostream& operator << (std::ostream& stream, const VarDecl& varDecl)
{
	stream << "var ";
	stream << varDecl.var_name;
	stream << ";\n";
	return stream;
}

std::ostream& operator << (std::ostream& stream, const Expression& expression)
{	
	std::for_each(expression.elements.begin(), expression.elements.end(), [&](const std::string& e) {		
		stream << e;
	});
	if (!expression.isPartOfCompoundStmt) stream << ";\n";
	return stream;
}

std::ostream& operator << (std::ostream& stream, const IfStatement& ifStatement)
{
	stream << "if (";
	stream << ifStatement.condition;
	stream << ") ";
	std::for_each(ifStatement.statements.begin(), ifStatement.statements.end(), [&](const StatementPtr& stmt) {
		stmt->text(stream);
	});
	return stream;
}

std::ostream& operator << (std::ostream& stream, const WhileLoop& loop)
{
	stream << "while (";
	stream << loop.condition;
	stream << ") ";
	std::for_each(loop.statements.begin(), loop.statements.end(), [&](const StatementPtr& stmt) {
		stmt->text(stream);
	});
	return stream;
}

std::ostream& operator << (std::ostream& stream, const LabelStatement& label)
{
	stream << label.label;
	stream << ":\n";
	return stream;
}

std::ostream& operator << (std::ostream& stream, const GotoStatement& gotoStatement)
{
	stream << "goto ";
	stream << gotoStatement.label;
	stream << ";\n";
	return stream;
}

std::ostream& operator << (std::ostream& stream, const BlockStatement& block)
{
	stream << "{\n";
	std::for_each(block.statements.begin(), block.statements.end(), [&](const StatementPtr& stmt) {
		stmt->text(stream);
	});
	stream << "}\n";
	return stream;
}
