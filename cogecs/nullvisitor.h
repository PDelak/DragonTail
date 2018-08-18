#ifndef COGECS_NULL_VISITOR
#define COGECS_NULL_VISITOR

#include "astvisitor.h"

struct NullVisitor : public AstVisitor
{	
	void visit(const BasicStatement*) {}
	void visit(const VarDecl*) {}
	void visit(const Expression*) {}
	void visit(const IfStatement*) {}
	void visit(const WhileLoop*) {}
	void visit(const BlockStatement*) {}
};

#endif
