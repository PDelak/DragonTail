#pragma once

#include <ostream>
#include <vector>
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
			if (allocs.size() < index + 1)
			{
				allocs.push_back(0);
			}
			++index;
		}
		if (expr->value == "__dealloc__")
		{
			--index;
		}
	}
	void visitPre(const VarDecl*) { allocs[index - 1]++; }

	void dump()
	{
		for (const auto& alloc : allocs)
		{
			std::cout << "allocation:" << alloc << std::endl;
		}
	}
	std::vector<size_t> getAllocationVector() const { return allocs; }
private:
	size_t index = 0;
	std::vector<size_t> allocs;
};

struct Basicx86Emitter : public AstVisitor
{
	Basicx86Emitter(X86InstrVector& v, std::vector<size_t> allocsVector):i_vector(v), allocationVector(allocsVector)
	{
		symbolTable.insertSymbol("print", "function");
		currentAllocation = allocationVector.begin();
	}
	void visitPre(const BasicStatement*) {}
	void visitPre(const VarDecl*) {}
	void visitPre(const BasicExpression* expr) 
	{
		if (expr->value == "__alloc__")
		{
			std::cout << "allocate:" << *currentAllocation << std::endl;
			size_t numOfVariables = *currentAllocation;
			// TODO: do that at once
			for (size_t i = 0; i < numOfVariables; ++i) {
				i_vector.push_back({ std::byte(0x83), std::byte(0xEC), std::byte(0x04) }); // sub esp, 4 (alloc)
			}
			++currentAllocation;
		}
		if (expr->value == "__dealloc__")
		{
			std::cout << "deallocate:" << *--currentAllocation << std::endl;
			size_t numOfVariables = *--currentAllocation;
			// TODO: do that at once
			for (size_t i = 0; i < numOfVariables; ++i) {
				i_vector.push_back({ std::byte(0x83), std::byte(0xC4), std::byte(0x04) }); // add esp, 4 (dealloc)
			}
			allocationVector.erase(currentAllocation);
			currentAllocation = allocationVector.rbegin().base();
		}
	}
	void visitPre(const Expression*) {}
	void visitPre(const IfStatement*) {}
	void visitPre(const WhileLoop*) {}
	void visitPre(const BlockStatement*) { symbolTable.enterScope();}
	void visitPre(const LabelStatement*) {}
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
	void visitPost(const Expression*) {}
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
	std::vector<size_t> allocationVector;
	std::vector<size_t>::iterator currentAllocation;

};

auto emitMachineCode(const StatementList& statements)
{
	X86InstrVector i_vector;
	i_vector.push_function_prolog();
	
	AllocationPass allocPass;
	traverse(statements, allocPass);

	allocPass.dump();

	Basicx86Emitter visitor(i_vector, allocPass.getAllocationVector());

	traverse(statements, visitor);
	
	i_vector.push_function_epilog();

	JitCompiler jit(i_vector);
	return jit.compile();
}

