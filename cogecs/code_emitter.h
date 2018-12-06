#pragma once

#include <ostream>
#include "ast.h"
#include "astvisitor.h"
#include "jitcompiler.h"

struct Basicx86Emitter : public AstVisitor
{
	void visitPre(const BasicStatement*) {}
	void visitPre(const VarDecl*) {}
	void visitPre(const BasicExpression*) {}
	void visitPre(const Expression*) {}
	void visitPre(const IfStatement*) {}
	void visitPre(const WhileLoop*) {}
	void visitPre(const BlockStatement*) {}
	void visitPre(const LabelStatement*) {}
	void visitPre(const GotoStatement*) {}
	void visitPre(const FunctionCall*) {}
	void visitPre(const FunctionDecl*) {}
	void visitPre(const ReturnStatement*) {}
	void visitPost(const BasicStatement*) {}
	void visitPost(const VarDecl*) {}
	void visitPost(const BasicExpression*) {}
	void visitPost(const Expression*) {}
	void visitPost(const IfStatement*) {}
	void visitPost(const WhileLoop*) {}
	void visitPost(const BlockStatement*) {}
	void visitPost(const LabelStatement*) {}
	void visitPost(const GotoStatement*) {}
	void visitPost(const FunctionCall*) {}
	void visitPost(const ReturnStatement*) {}
	void visitPost(const FunctionDecl*) {}

	StatementList getStatements() const { return statements; }
private:
	StatementList statements;
};

auto emitMachineCode(const StatementList& statements)
{
	Basicx86Emitter visitor;

	traverse(statements, visitor);
	dumpCode(visitor.getStatements(), std::cout);
	X86InstrVector i_vector;
	i_vector.push_function_prolog();

	i_vector.push_function_epilog();
	JitCompiler jit(i_vector);
	return jit.compile();
}

