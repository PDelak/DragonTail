#ifndef COGECS_AST_VISITOR
#define COGECS_AST_VISITOR

struct BasicStatement;
struct VarDecl;
struct Expression;
struct IfStatement;
struct WhileLoop;
struct BlockStatement;

struct AstVisitor
{
	virtual ~AstVisitor() {}
	virtual void visit(const BasicStatement*) = 0;
	virtual void visit(const VarDecl*) = 0;
	virtual void visit(const Expression*) = 0;
	virtual void visit(const IfStatement*) = 0;
	virtual void visit(const WhileLoop*) = 0;
	virtual void visit(const BlockStatement*) = 0;
};

#endif
