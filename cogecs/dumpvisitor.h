#ifndef COGECS_DUMP_VISITOR
#define COGECS_DUMP_VISITOR

#include "astvisitor.h"

struct DumpVisitor : public AstVisitor
{	
	DumpVisitor(size_t depth, std::ostream& out) :depth(depth), out(out) {}
	
	void visit(const BasicStatement* stmt) 
	{
		stmt->dump(depth, out);
	}
	void visit(const VarDecl* stmt) 
	{
		stmt->dump(depth, out);
	}
	void visit(const Expression* stmt) 
	{
		stmt->dump(depth, out);
	}
	void visit(const IfStatement* stmt) 
	{
		stmt->dump(depth, out);
	}
	void visit(const WhileLoop* stmt) 
	{
		stmt->dump(depth, out);
	}
	void visit(const BlockStatement* stmt) 
	{
		// only block statements at global scope
		// needs to be visited
		// all others are connected to compound statements
		// like if or while loop
		if (stmt->scope != 0) return;
		stmt->dump(depth, out);
	}
private:
	size_t depth;
	std::ostream& out;
};

#endif
