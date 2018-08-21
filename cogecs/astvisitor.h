#ifndef COGECS_AST_VISITOR
#define COGECS_AST_VISITOR

#include <vector>
#include <memory>

struct Statement;
struct BasicStatement;
struct VarDecl;
struct Expression;
struct IfStatement;
struct WhileLoop;
struct BlockStatement;
struct LabelStatement;
struct GotoStatement;

using StatementPtr = std::shared_ptr<Statement>;
using StatementList = std::vector<StatementPtr>;

struct AstVisitor
{

	virtual ~AstVisitor() {}
	virtual void visitPre(const BasicStatement*) = 0;
	virtual void visitPost(const BasicStatement*) = 0;
	virtual void visitPre(const VarDecl*) = 0;
	virtual void visitPost(const VarDecl*) = 0;
	virtual void visitPre(const Expression*) = 0;
	virtual void visitPost(const Expression*) = 0;
	virtual void visitPre(const IfStatement*) = 0;
	virtual void visitPost(const IfStatement*) = 0;
	virtual void visitPre(const WhileLoop*) = 0;
	virtual void visitPost(const WhileLoop*) = 0;
	virtual void visitPre(const BlockStatement*) = 0;
	virtual void visitPost(const BlockStatement*) = 0;
	virtual void visitPre(const LabelStatement*) = 0;
	virtual void visitPost(const LabelStatement*) = 0;
	virtual void visitPre(const GotoStatement*) = 0;
	virtual void visitPost(const GotoStatement*) = 0;
};

#endif
