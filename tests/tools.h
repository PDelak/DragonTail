#pragma once
#include "../src/compiler.h"

void checkASTs(const StatementList& ast1, const StatementList& ast2)
{
	std::ostringstream expected;
	dumpAST(ast1, expected);
	std::ostringstream parsed;
	dumpAST(ast2, parsed);
	auto lhs = expected.str();
	auto rhs = parsed.str();
	lhs.erase(std::remove(lhs.begin(), lhs.end(), ' '), lhs.end());
	rhs.erase(std::remove(rhs.begin(), rhs.end(), ' '), rhs.end());
	EXPECT_EQ(lhs, rhs);
	if (lhs == rhs) return;	
	std::cout << "expected:" << '\n' << '\n';
	std::cout << "--------------------------" << std::endl;
	dumpCode(ast1, std::cout);
	std::cout << '\n' << '\n';
	std::cout << "actual:" << std::endl;
	std::cout << "--------------------------" << std::endl;
	dumpCode(ast2, std::cout);	
}

template<typename Visitor>
void testProgram(std::string text, StatementList result)
{
	Visitor visitor;
	auto parser = initialize_parser();
	auto stmts = parse(parser.get(), &text[0], &text[0] + text.size(), visitor);
	traverse(stmts, visitor);
	auto statements = visitor.getStatements();
	EXPECT_EQ(statements.size(), result.size());

	checkASTs(result, statements);
}

