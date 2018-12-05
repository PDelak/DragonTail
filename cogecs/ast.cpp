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

std::ostream& operator << (std::ostream& stream, const BasicExpression& expression)
{
	stream << expression.value;
	return stream;
}

std::ostream& operator << (std::ostream& stream, const Expression& expression)
{	
	std::for_each(expression.child_begin(), expression.child_end(), [&](const StatementPtr& stmt) {		
		static_cast<Statement*>(stmt.get())->text(stream);
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


std::ostream& operator << (std::ostream& stream, const FunctionCall& functionCall)
{
	bool first = false;
	stream << functionCall.name;
	stream << "(";
	std::for_each(functionCall.parameters.begin(), functionCall.parameters.end(), [&](const std::string& param) {
		if (first) {
			stream << " ";
		}
		first = true;
		stream << param;
	});
	stream << ")";
	return stream;
}

std::ostream& operator << (std::ostream& stream, const ReturnStatement& returnStatement)
{
	stream << "return ";
	stream << returnStatement.param;
	stream << ";\n";
	return stream;
}

std::ostream& operator << (std::ostream& stream, const FunctionDecl& functionDecl)
{
	bool first = false;
	stream << "function ";
	stream << functionDecl.name;
	stream << "(";
	std::for_each(functionDecl.parameters.begin(), functionDecl.parameters.end(), [&](const std::string& param) {
		if (first) {
			stream << " ";
		}
		first = true;
		stream << param;
	});
	stream << ")\n";
	for (const auto& stmt : functionDecl.statements) {
		stmt->text(stream);
	}
	return stream;
}
