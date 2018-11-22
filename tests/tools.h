#pragma once

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

