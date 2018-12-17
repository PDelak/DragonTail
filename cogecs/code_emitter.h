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

// AllocationPass counts number of variables per each
// __alloc__ builtin expression
// Value is stored in a vector indexed per allocation 
// It is further used to allocate specific number
// of variables (memory) on the stack
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

struct Basicx86Emitter : public NullVisitor
{
	Basicx86Emitter(X86InstrVector& v, std::vector<size_t> allocsVector)
		:i_vector(v), allocationVector(allocsVector)
	{
		symbolTable.insertSymbol("print", "function");
		currentAllocation = allocationVector.begin();
	}

	void visitPre(const BasicExpression* expr) 
	{
		if (expr->value == "__alloc__")
		{
			variable_position_on_stack = 0;
			size_t numOfVariables = *currentAllocation;
			// TODO: do that at once
			for (size_t i = 0; i < numOfVariables; ++i) {
				i_vector.push_back({ std::byte(0x83), std::byte(0xEC), std::byte(0x04) }); // sub esp, 4 (alloc)
			}
			++currentAllocation;
		}
		if (expr->value == "__dealloc__")
		{
			size_t numOfVariables = *--currentAllocation;
			// TODO: do that at once
			for (size_t i = 0; i < numOfVariables; ++i) {
				i_vector.push_back({ std::byte(0x83), std::byte(0xC4), std::byte(0x04) }); // add esp, 4 (dealloc)
			}
			allocationVector.erase(currentAllocation);
			currentAllocation = allocationVector.rbegin().base();
		}
	}
	void visitPost(const VarDecl* varDecl) 
	{
		symbolTable.insertSymbol(varDecl->var_name, "number", variable_position_on_stack++);			
	}
	void visitPost(const Expression* expr) 
	{
		auto children = expr->getChilds();
		switch(children.size()) 
		{
			case 3: {
				auto op = cast<BasicExpression>(children[1]);
				if (op->value == "=")
				{
					auto lhs = cast<BasicExpression>(children[0]);
					auto lhsSymbol = symbolTable.findSymbol(lhs->value, 0);
					auto rhs = cast<BasicExpression>(children[2]);

					// TODO: only variable = number works for now
					if (std::isalpha(rhs->value[0])) return;

					int rhsValue = std::stoi(rhs->value);

					i_vector.push_back({ std::byte(0xB8) }); // mov eax, rhsValue
					i_vector.push_back(i_vector.int_to_bytes(rhsValue));

					// TODO: this is only true for 32 bit 
					char variableSize = 4;
					char ebpOffset = (lhsSymbol.stack_position + 1) * variableSize;

					// TODO: just for now stack for local variables will be only 256 bytes
					constexpr unsigned int stackSize = 256;
					// mov [ebp - ebpOffset], eax
					i_vector.push_back({ std::byte(0x89), std::byte(0x45), std::byte(stackSize - ebpOffset) });
				}
				break;
			}
			case 4: {
				break;
			}
			case 5: {
				break;
			}
		}		
	}
	void visitPost(const FunctionCall* fcall) 
	{	
		// push params
		for (const auto& param : fcall->parameters) 
		{
			if (std::isdigit(param[0])) {
				i_vector.push_back({ std::byte(0x68) }); // push
				i_vector.push_back(i_vector.int_to_bytes(std::stoi(param)));
			}
			if (std::isalpha(param[0])) {
				// if it is not value check index on the stack
				// and copy value that is indexed by this index
				// FF 75 FC           push        dword ptr [ebp-4]
				auto sym = symbolTable.findSymbol(param, 0);
				char variableSize = 4;
				char ebpOffset = (sym.stack_position + 1) * variableSize;

				// just for now stack for local variables will be only 256 bytes
				constexpr unsigned int stackSize = 256;
				i_vector.push_back({ std::byte(0xFF), std::byte(0x75), std::byte(stackSize - ebpOffset) });
			}
		}

		i_vector.push_back({ std::byte(0xB8) });  // \  mov eax, address of function
		
		symbolTable.findSymbol(fcall->name, 0);
				
		if (fcall->name == "print") 
		{
			i_vector.push_back(i_vector.get_address(reinterpret_cast<void*>(&builtin_print)));
		}
		i_vector.push_back({ std::byte(0xFF), std::byte(0xD0) }); // call eax
		i_vector.push_back({ std::byte(0x83), std::byte(0xC4), std::byte(0x04) }); // add esp, 4 (clean stack)

	}

	StatementList getStatements() const { return statements; }

private:
	StatementList statements;
	BasicSymbolTable symbolTable;
	X86InstrVector& i_vector;
	std::vector<size_t> allocationVector;
	std::vector<size_t>::iterator currentAllocation;
	unsigned char variable_position_on_stack = 0;
};

auto emitMachineCode(const StatementList& statements)
{
	X86InstrVector i_vector;
	i_vector.push_function_prolog();
	
	AllocationPass allocPass;
	traverse(statements, allocPass);

	Basicx86Emitter visitor(i_vector, allocPass.getAllocationVector());

	traverse(statements, visitor);
	
	i_vector.push_function_epilog();

	JitCompiler jit(i_vector);
	return jit.compile();
}

