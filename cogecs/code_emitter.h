#pragma once

#include <ostream>
#include <map>
#include <string>
#include "ast.h"
#include "astvisitor.h"
#include "jitcompiler.h"
#include "builtin.h"
#include "symbol_table.h"
#include "nullvisitor.h"

struct AllocationPass : public NullVisitor
{
	void visitPre(const BasicExpression* expr) 
	{
		if (expr->value == "__alloc__") 
		{
			std::cout << "allocation" << std::endl;
		}

		if (expr->value == "__dealloc__")
		{
			std::cout << "deallocation" << std::endl;
		}
	}
	void visitPre(const VarDecl*) { allocsPerLabel[currentLabel]++; }

	void dump()
	{
		for (const auto& alloc : allocsPerLabel)
		{
			std::cout << "label:" << alloc.first << " allocs:" << alloc.second << std::endl;
		}
	}
	std::string currentLabel;
	std::map<std::string, size_t> allocsPerLabel;
};

struct Basicx86Emitter : public AstVisitor
{
	Basicx86Emitter(X86InstrVector& v):i_vector(v) 
	{
		symbolTable.insertSymbol("print", "function");
	}
	void visitPre(const BasicStatement*) {}
	void visitPre(const VarDecl*) {}
	void visitPre(const BasicExpression*) {}
	void visitPre(const Expression*) {}
	void visitPre(const IfStatement*) {}
	void visitPre(const WhileLoop*) {}
	void visitPre(const BlockStatement*) { symbolTable.enterScope();}
	void visitPre(const LabelStatement* label) 
	{
		std::cout << "label:" << label->label << std::endl;
	}
	void visitPre(const GotoStatement*) {}
	void visitPre(const FunctionCall*) {}
	void visitPre(const FunctionDecl*) {}
	void visitPre(const ReturnStatement*) {}
	void visitPost(const BasicStatement*) {}
	void visitPost(const VarDecl* varDecl) 
	{
		symbolTable.insertSymbol(varDecl->var_name, "number");
		std::cout << "var:" << varDecl->var_name << std::endl;
	}
	void visitPost(const BasicExpression*) {}
	void visitPost(const Expression* expr) 
	{
	}
	void visitPost(const IfStatement*) {}
	void visitPost(const WhileLoop*) {}
	void visitPost(const BlockStatement*) { symbolTable.exitScope(); }
	void visitPost(const LabelStatement*) {}
	void visitPost(const GotoStatement*) {}
	void visitPost(const FunctionCall* fcall) 
	{	
		symbolTable.dump();
		// push params
		for (const auto& param : fcall->parameters) 
		{
			i_vector.push_back({ std::byte(0x68) }); // push
			i_vector.push_back(i_vector.int_to_bytes(std::stoi(param)));
		}

		i_vector.push_back({ std::byte(0xB8) });  // \  mov eax, address of function
		
		symbolTable.findSymbol("print", 0);
				
		if (fcall->name == "print") 
		{
			i_vector.push_back(i_vector.get_address(reinterpret_cast<void*>(&builtin_print)));
		}
		i_vector.push_back({ std::byte(0xFF), std::byte(0xD0) }); // call eax
		i_vector.push_back({ std::byte(0x83), std::byte(0xC4), std::byte(0x04) }); // add esp, 4 (clean stack)

	}
	void visitPost(const ReturnStatement*) {}
	void visitPost(const FunctionDecl*) {}

	StatementList getStatements() const { return statements; }

private:
	StatementList statements;
	BasicSymbolTable symbolTable;
	X86InstrVector& i_vector;

};

auto emitMachineCode(const StatementList& statements)
{
	X86InstrVector i_vector;
	i_vector.push_function_prolog();
	
	AllocationPass allocPass;
	traverse(statements, allocPass);

	allocPass.dump();

	Basicx86Emitter visitor(i_vector);

	traverse(statements, visitor);
	
	i_vector.push_function_epilog();

	JitCompiler jit(i_vector);
	return jit.compile();
}

