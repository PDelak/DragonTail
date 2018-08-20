#ifndef COGECS_DUMP_VISITOR
#define COGECS_DUMP_VISITOR

#include "astvisitor.h"

struct DumpVisitor : public AstVisitor
{	
	DumpVisitor(size_t depth, std::ostream& out) :depth(depth), out(out) {}
	
	void visitPre(const BasicStatement*) {}
	void visitPre(const VarDecl*) {}
	void visitPre(const Expression*) {}
	void visitPre(const IfStatement*) {}
	void visitPre(const WhileLoop*) {}
	void visitPre(const BlockStatement*) {}
	void visitPre(const LabelStatement*) {}

	void visitPost(const BasicStatement* stmt) 
	{
		if (stmt->scope != 0) return;
		stmt->dump(depth, out);
	}
	void visitPost(const VarDecl* stmt) 
	{
		if (stmt->scope != 0) return;
		stmt->dump(depth, out);
	}
	void visitPost(const Expression* stmt) 
	{
		if (stmt->scope != 0) return;
		stmt->dump(depth, out);
	}
	void visitPost(const IfStatement* stmt) 
	{
		if (stmt->scope != 0) return;
		stmt->dump(depth, out);
	}
	void visitPost(const WhileLoop* stmt) 
	{
		if (stmt->scope != 0) return;
		stmt->dump(depth, out);
	}
	void visitPost(const BlockStatement* stmt) 
	{
		// only block statements at global scope
		// needs to be visited
		// all others are connected to compound statements
		// like if or while loop
		if (stmt->scope != 0) return;
		stmt->dump(depth, out);
	}
	void visitPost(const LabelStatement* stmt)
	{
		if (stmt->scope != 0) return;
		stmt->dump(depth, out);
	}
private:
	size_t depth;
	std::ostream& out;
};

#endif
