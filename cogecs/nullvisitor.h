#ifndef COGECS_NULL_VISITOR
#define COGECS_NULL_VISITOR

#include "astvisitor.h"

struct NullVisitor : public AstVisitor
{	
	void visitPre(const BasicStatement*) {}
	void visitPre(const VarDecl*) {}
	void visitPre(const Expression*) {}
	void visitPre(const IfStatement*) {}
	void visitPre(const WhileLoop*) {}
	void visitPre(const BlockStatement*) {}
	void visitPre(const LabelStatement*) {}
	void visitPre(const GotoStatement*) {}
	void visitPost(const BasicStatement*) {}
	void visitPost(const VarDecl*) {}
	void visitPost(const Expression*) {}
	void visitPost(const IfStatement*) {}
	void visitPost(const WhileLoop*) {}
	void visitPost(const BlockStatement*) {}
	void visitPost(const LabelStatement*) {}
	void visitPost(const GotoStatement*) {}
};

#endif
