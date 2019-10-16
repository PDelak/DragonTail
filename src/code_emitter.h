#pragma once

#include <ostream>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <set>
#include <stack>
#include <utility>
#include "ast.h"
#include "astvisitor.h"
#include "jitcompiler.h"
#include "builtin.h"
#include "symbol_table.h"
#include "nullvisitor.h"
#include "sema.h"

/*
// AllocationPass counts number of variables per each block
// those between __alloc__ and __dealloc__ builtin expressions
// Value (which is number of variables) is stored in a map, indexed by pair (level and position on the level)
//               0              - level 0 (root) 
//              / \
//             /   \
//            0     1           - level 1 (indexes in map (1,0) (1,1))
//           / \   / \
//          0   1 0   1         - level 2 (indexes in map (2,0) (2,1) (2,2) (2,3)
// It is further used to allocate specific number
// of variables (memory) on the stack
*/

using AllocationMap = std::map<std::pair<size_t, size_t>, size_t>;

using GotoLabelsFromIf = std::set<std::string>;
using LabelToCodePosition = std::map<std::string, size_t>;

// TODO: just for now stack for local variables will be only 256 bytes
constexpr unsigned int stackSize = 256;
using TypeSizeOfMap = std::map<std::string, int>;

// for 32 bit arch
TypeSizeOfMap typeSizeOfMap = {
        {"i32", 4},
        {"^i32", 4},
};

struct PreAllocationPass : public NullVisitor
{
    void visitPre(const BasicExpression* expr)
    {
        if (expr->value == "__alloc__")
        {
            ++allocationLevel;
        }
        if (expr->value == "__dealloc__")
        {           
            allocationLevelIndex[allocationLevel]++;
            --allocationLevel;
        }
    }
    void visitPre(const VarDecl*) 
    {
        allocs[std::make_pair(allocationLevel, allocationLevelIndex[allocationLevel])]++;
    }

    void dump()
    {
        for (auto elem : allocs)
        {
            //std::cout << "(" << elem.first.first << "," << elem.first.second << ")" << "->" << elem.second << std::endl;
        }
    }
    std::map<std::pair<size_t, size_t>, size_t> getAllocationVector() const { return allocs; }
private:
    size_t allocationLevel = 0;
    std::map<size_t, size_t> allocationLevelIndex;
    AllocationMap allocs;
};

struct CodeEmitterException : public std::runtime_error
{
    CodeEmitterException(const std::string& msg):std::runtime_error(msg) {}
};

unsigned int calculateVariableOffset(const symbol& sym, size_t currentAllocationLevel, const AllocationMap& allocs)
{
  // TODO variable size is hardcoded for now and is always 4 bytes
  // this is only true for 32 bit
  // calculation variable position is frame layout dependent
  // currently each variable is allocated at the beginning of stack frame
  // each layer/scope adds additional 4 bytes due to fact of emitting
  // new function prolog eg push ebp; mov ebp, esp;
  char variableSize = 4;
  // if in the same scope
  if ((currentAllocationLevel - sym.scope) == 0)
  {
    char ebpOffset = (sym.stack_position + 1) * variableSize;
    return ebpOffset;
  }
  // if variable is defined in outer scope
  // it requires to go up the stack
  // so [ebp + value]
  char numberOfLevelsUp = (currentAllocationLevel - sym.scope) * variableSize;
  auto alloc_it = allocs.find(std::make_pair(sym.allocation_level, sym.level_index));
  auto numOfVariablesOnLevel = alloc_it->second;
  char numberOfVariablesInBetween = (numOfVariablesOnLevel - (sym.stack_position + 1)) * variableSize;
  char ebpOffset = numberOfLevelsUp + (numberOfVariablesInBetween);
  // std::cout << "sym:" << sym.id << " level:" << currentAllocationLevel
  // 	      << " scope: " << sym.scope << " stack_position: " << (int)sym.stack_position
  //	      << " between:" << (int)numberOfVariablesInBetween << " epbOffset:" << (int)ebpOffset << std::endl;

  return ebpOffset;
}


unsigned int calculateVariablePositionOnStack(const symbol& sym, size_t currentAllocationLevel, const AllocationMap& allocs)
{
    if ((currentAllocationLevel - sym.scope) == 0)
    {
      return stackSize - calculateVariableOffset(sym, currentAllocationLevel, allocs);
    }
    return calculateVariableOffset(sym, currentAllocationLevel, allocs);
}

struct Basicx86Emitter : public NullVisitor
{
    Basicx86Emitter(X86InstrVector& v, std::map<std::pair<size_t, size_t>, size_t> allocVector, BasicSymbolTable& symTable, const std::map<std::string, void*>& fMap)
        :i_vector(v), allocs(allocVector), symbolTable(symTable), functionMap(fMap)
    {
        allocationLevelIndex[allocationLevel] = 0;
    }
    ~Basicx86Emitter()
    {
    }
    void visitPre(const BasicExpression* expr) 
    {
        if (expr->value == "__alloc__")
        {
            symbolTable.enterScope();
            // emitting function prolog
            // push ebp
            // mov ebp, esp
            // is used to simplify relative access to variables
            // to calculate right offset
            i_vector.push_function_prolog();

            size_t numOfVariables = allocs[std::make_pair(allocationLevel, allocationLevelIndex[allocationLevel])];
            //std::cout << "alloc" << "(" << allocationLevel << "," << allocationLevelIndex[allocationLevel] << ")" << ":" << numOfVariables << std::endl;
            for (size_t i = 0; i < numOfVariables; ++i) {
                i_vector.push_back({ std::byte(0x83), std::byte(0xEC), std::byte(0x04) }); // sub esp, 4 (alloc)
            }
            scopeId.push(std::make_pair(allocationLevel, allocationLevelIndex[allocationLevel]));
            ++allocationLevel;

        }
        if (expr->value == "__dealloc__")
        {
            size_t level = allocationLevel - 1;
            size_t numOfVariables = allocs[std::make_pair(level, allocationLevelIndex[level])];
            //std::cout << "dealloc" << "(" << level << "," << allocationLevelIndex[level] << ")" << ":" << numOfVariables << std::endl;
            for (size_t i = 0; i < numOfVariables; ++i) {
                i_vector.push_back({ std::byte(0x83), std::byte(0xC4), std::byte(0x04) }); // add esp, 4 (dealloc)
            }
            --allocationLevel;
            allocationLevelIndex[allocationLevel]++;

            scopeId.pop();
            // pop ebp
            i_vector.push_back({std::byte(0x5D)});
            symbolTable.exitScope();

        }
    }
    void visitPost(const VarDecl* varDecl) 
    {   
        if(symbolTable.exists(varDecl->var_name)) 
        {
            throw CodeEmitterException("variable already defined : " + varDecl->var_name);
        }

        auto scope = scopeId.top();
        // std::cout << varDecl->var_name << " : " << "(" << scope.first << "," << scope.second << ")" << " stack position:" << (int)variable_position_on_stack_map[scope] << std::endl;

        symbolTable.insertSymbol(varDecl->var_name, varDecl->type, variable_position_on_stack_map[scope], scope.first, scope.second);

        variable_position_on_stack_map[scope]++;
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

                    // variable alias on rhs
                    if (std::isalpha(rhs->value[0])) 
                    {
                        auto sym = symbolTable.findSymbol(rhs->value, 0);

                        auto currentAllocationLevel = scopeId.top().first;
                        unsigned int variablePosition = calculateVariablePositionOnStack(sym, currentAllocationLevel, allocs);
                        // mov eax, [ebp - ebpOffset]
                        i_vector.push_back({ std::byte(0x8B), std::byte(0x45), std::byte(variablePosition)});                       
                    } 
                    // value on rhs
                    else 
                    {
                        int rhsValue = std::stoi(rhs->value);

                        i_vector.push_back({ std::byte(0xB8) }); // mov eax, rhsValue
                        i_vector.push_back(i_vector.int_to_bytes(rhsValue));
                    }

                    auto currentAllocationLevel = scopeId.top().first;

                    //std::cout << "sym : " << lhsSymbol.id << " level:" << currentAllocationLevel << " scope:" << lhsSymbol.scope << std::endl;
                    unsigned int variablePosition = calculateVariablePositionOnStack(lhsSymbol, currentAllocationLevel, allocs);
                    // mov [ebp - ebpOffset], eax
                    i_vector.push_back({ std::byte(0x89), std::byte(0x45), std::byte(variablePosition) });
                }
                else
                {
                    std::string errMessage = "expression is noop operation : ";
                    std::stringstream outStream;
                    for(const auto child : expr->getChilds())
                    {
                        child->text(outStream);
                    }
                    errMessage += outStream.str();
                    throw CodeEmitterException(errMessage);
                }
                break;
            }
            case 4: {
                // just for now
                // support only 3 pointer operations
                // var a;
                // var p;
                // p = &a; (get address of)
                // *p = 1; (assignment to dereferenced pointer)
                // a = *p; (pointer dereference and assignment)
                auto lhs = cast<BasicExpression>(children[0]);
                if (lhs->value == "*")
                {
                  auto lhsVariable =  cast<BasicExpression>(children[1]);
                  auto op =  cast<BasicExpression>(children[2]);
                  if(op->value != "=")
                  {
                    throw CodeEmitterException("only = is supported for pointers");
                  }
                  auto rhsVariable =  cast<BasicExpression>(children[3]);
                  auto sym = symbolTable.findSymbol(lhsVariable->value, 0);
                  if(sym.type[0] != '^')
                  {
                    std::string errMessage = "only pointers can be dereferenced : ";
                    std::stringstream outStream;
                    for(const auto child : expr->getChilds())
                    {
                        child->text(outStream);
                    }
                    errMessage += outStream.str();
                    throw CodeEmitterException(errMessage);
                  }
                  auto currentAllocationLevel = scopeId.top().first;
                  unsigned int variablePosition = calculateVariablePositionOnStack(sym, currentAllocationLevel, allocs);
                  // mov eax, [ebp - ebpOffset]
                  i_vector.push_back({std::byte(0x8B), std::byte(0x45), std::byte(variablePosition)});
                  if (!std::isalpha(rhsVariable->value[0]))
                  {
                    int value = std::stoi(rhsVariable->value);
                    // mov [eax], value
                    i_vector.push_back({std::byte(0xC7), std::byte(0x00)});
                    i_vector.push_back(i_vector.int_to_bytes(value));
                  }
                  else {
                    auto sym = symbolTable.findSymbol(rhsVariable->value, 0);

                    auto currentAllocationLevel = scopeId.top().first;
                    unsigned int variablePosition = calculateVariablePositionOnStack(sym, currentAllocationLevel, allocs);
                    // mov ebx, [ebp - ebpOffset]
                    i_vector.push_back({ std::byte(0x8B), std::byte(0x5D), std::byte(variablePosition)});

                    // mov [eax], ebx
                    i_vector.push_back({std::byte(0x89), std::byte(0x18)});
                  }
                }
                else
                {
                  auto lhsSymbol = symbolTable.findSymbol(lhs->value, 0);
                  auto op = cast<BasicExpression>(children[1]);
                  if (op->value != "=") throw CodeEmitterException("Expression should have form of a = op b");
                  auto unaryOp = cast<BasicExpression>(children[2]);
                  auto rhs = cast<BasicExpression>(children[3]);
                  if (unaryOp->value == "!")
                  {
                    auto sym = symbolTable.findSymbol(rhs->value, 0);
                    auto currentAllocationLevel = scopeId.top().first;
                    unsigned int variablePosition = calculateVariablePositionOnStack(sym, currentAllocationLevel,
                                                                                     allocs);
                    // mov eax, [ebp - ebpOffset]
                    i_vector.push_back({std::byte(0x8B), std::byte(0x45), std::byte(variablePosition)});
                    // compare rhsValue with 0
                    comparisonOperatorValue(0, insertJG);
                    // TODO: this is only true for 32 bit 
                    unsigned int lhsVariablePosition = calculateVariablePositionOnStack(lhsSymbol,
                                                                                        currentAllocationLevel, allocs);

                    // mov [ebp - ebpOffset], eax
                    i_vector.push_back({std::byte(0x89), std::byte(0x45), std::byte(lhsVariablePosition)});
                  }
                  if (unaryOp->value == "&")
                  {
                    i_vector.push_back({std::byte(0x89), std::byte(0xe8)});
                    auto sym = symbolTable.findSymbol(rhs->value, 0);
                    auto currentAllocationLevel = scopeId.top().first;

                    unsigned int offset = calculateVariableOffset(sym, currentAllocationLevel, allocs);
                    // go down the stack, variable in current scope
                    if ((currentAllocationLevel - sym.scope) == 0)
                    {
                      // sub eax, offset
                      i_vector.push_back({ std::byte(0x83), std::byte(0xe8), std::byte(offset) });

                    }
                    // go up the stack, variable in outer scope
                    else
                    {
                        // add eax, offset
                      i_vector.push_back({ std::byte(0x83), std::byte(0xc0), std::byte(offset) });
                    }
                    unsigned int lhsVariablePosition = calculateVariablePositionOnStack(lhsSymbol,
                                                                                        currentAllocationLevel, allocs);
                    // mov [ebp - ebpOffset], eax
                    i_vector.push_back({std::byte(0x89), std::byte(0x45), std::byte(lhsVariablePosition)});
                  }
                  if (unaryOp->value == "*")
                  {
                    auto sym = symbolTable.findSymbol(rhs->value, 0);
                    if(sym.type[0] != '^')
                    {
                        std::string errMessage = "only pointers can be dereferenced : ";
                        std::stringstream outStream;
                        for(const auto child : expr->getChilds())
                        {
                            child->text(outStream);
                        }
                        errMessage += outStream.str();
                        throw CodeEmitterException(errMessage);
                    }
                    auto currentAllocationLevel = scopeId.top().first;
                    unsigned int variablePosition = calculateVariablePositionOnStack(sym, currentAllocationLevel,
                                                                                     allocs);
                    // mov eax, [ebp - ebpOffset]
                    i_vector.push_back({std::byte(0x8B), std::byte(0x45), std::byte(variablePosition)});
                    // mov eax, [eax]
                    i_vector.push_back({std::byte(0x8B), std::byte(0x00)});
                    unsigned int lhsVariablePosition = calculateVariablePositionOnStack(lhsSymbol,
                                                                                        currentAllocationLevel, allocs);
                    // mov [ebp - ebpOffset], eax
                    i_vector.push_back({std::byte(0x89), std::byte(0x45), std::byte(lhsVariablePosition)});
                  }
                }
              break;
            }
            case 5: {
                auto op = cast<BasicExpression>(children[1]);
                if (op->value != "=") throw CodeEmitterException("Expression should have form of a = b op c");
                auto lhs = cast<BasicExpression>(children[0]);
                auto lhsSymbol = symbolTable.findSymbol(lhs->value, 0);
                auto firstParam = cast<BasicExpression>(children[2]);
                auto secondParam = cast<BasicExpression>(children[4]);
                auto binOp = cast<BasicExpression>(children[3]);
                if (binOp->value == "+" || binOp->value == "-" || binOp->value == "*" || binOp->value == "/" || 
                    binOp->value == "==" || binOp->value == "!=" || binOp->value == "<" || binOp->value == ">" || binOp->value == "<=" || binOp->value == ">=") 
                {
                    // variable alias as firstParam
                    if (std::isalpha(firstParam->value[0]))
                    {
                        auto sym = symbolTable.findSymbol(firstParam->value, 0);
                        auto currentAllocationLevel = scopeId.top().first;
                        unsigned int variablePosition = calculateVariablePositionOnStack(sym, currentAllocationLevel, allocs);
                        // mov eax, [ebp - ebpOffset]
                        i_vector.push_back({ std::byte(0x8B), std::byte(0x45), std::byte(variablePosition) });
                    }
                    else
                    {
                        int rhsValue = std::stoi(firstParam->value);

                        i_vector.push_back({ std::byte(0xB8) }); // mov eax, rhsValue
                        i_vector.push_back(i_vector.int_to_bytes(rhsValue));
                    }
                    if (std::isalpha(secondParam->value[0]))
                    {
                        auto sym = symbolTable.findSymbol(secondParam->value, 0);
                        auto currentAllocationLevel = scopeId.top().first;

                        unsigned int variablePosition = calculateVariablePositionOnStack(sym, currentAllocationLevel, allocs);
                        
                        if (binOp->value == "+") 
                        {
                            auto sym = symbolTable.findSymbol(firstParam->value, 0);
                            // stack grows downwards which means
                            // that for pointer subtraction means addition and reverse
                            if(sym.type[0] == '^')
                            {
                                auto sizeOf = typeSizeOfMap[sym.type];
                                for(int i = 0; i < sizeOf; ++i) {
                                    i_vector.push_back({std::byte(0x2B), std::byte(0x45), std::byte(variablePosition)});
                                }
                            }
                            else
                            {
                                // add eax, [ebp - ebpOffset]
                                i_vector.push_back({ std::byte(0x03), std::byte(0x45), std::byte(variablePosition) });
                            }
                        }
                        if (binOp->value == "-")
                        {
                            auto sym = symbolTable.findSymbol(firstParam->value, 0);
                            // stack grows downwards which means
                            // that for pointer subtraction means addition and reverse
                            if(sym.type[0] == '^')
                            {
                                auto sizeOf = typeSizeOfMap[sym.type];
                                for(int i = 0; i < sizeOf; ++i) {
                                    i_vector.push_back({ std::byte(0x03), std::byte(0x45), std::byte(variablePosition) });
                                }
                            }
                            else
                            {
                                // sub eax, [ebp - ebpOffset]
                                i_vector.push_back({ std::byte(0x2B), std::byte(0x45), std::byte(variablePosition) });
                            }
                        }
                        if (binOp->value == "*")
                        {
                            // imul        eax, dword ptr[ebp - ebpOffset]
                            i_vector.push_back({ std::byte(0x0F), std::byte(0xAF), std::byte(0x45), std::byte(variablePosition) });
                        }
                        if (binOp->value == "/")
                        {
                            // push ebx
                            i_vector.push_back({ std::byte(0x53) });
                            // cdq sign-extend EAX into EDX
                            i_vector.push_back({ std::byte(0x99) });
                            // mov ebx, dword ptr[ebp - ebpOffset]
                            i_vector.push_back({ std::byte(0x8B), std::byte(0x5D), std::byte(variablePosition) });
                            // idiv ebx
                            i_vector.push_back({ std::byte(0xF7), std::byte(0xFB) });
                            // pop ebx
                            i_vector.push_back({ std::byte(0x5B) });
                        }
                        if (binOp->value == "==")
                        {
                            comparisonOperatorVariable(variablePosition, insertJNE);
                        }
                        if (binOp->value == "!=")
                        {
                            comparisonOperatorVariable(variablePosition, insertJE);
                        }
                        if (binOp->value == "<")
                        {
                            comparisonOperatorVariable(variablePosition, insertJNL);
                        }
                        if (binOp->value == ">")
                        {
                            comparisonOperatorVariable(variablePosition, insertJNG);
                        }
                        if (binOp->value == ">=")
                        {
                            comparisonOperatorVariable(variablePosition, insertJNGE);
                        }
                        if (binOp->value == "<=")
                        {
                            comparisonOperatorVariable(variablePosition, insertJNLE);
                        }
                    }
                    else
                    {
                        int rhsValue = std::stoi(secondParam->value);
                        if (binOp->value == "+")
                        {
                            auto sym = symbolTable.findSymbol(firstParam->value, 0);
                            // stack grows downwards which means
                            // that for pointer subtraction means addition and reverse
                            if(sym.type[0] == '^')
                            {
                                auto sizeOf = typeSizeOfMap[sym.type];
                                for(int i = 0; i < sizeOf; ++i) {
                                    i_vector.push_back({std::byte(0x2D)}); // sub eax, rhsValue
                                    i_vector.push_back(i_vector.int_to_bytes(rhsValue));
                                }
                            }
                            else
                            {
                                i_vector.push_back({std::byte(0x05)}); // add eax, rhsValue
                                i_vector.push_back(i_vector.int_to_bytes(rhsValue));
                            }
                        }
                        if (binOp->value == "-")
                        {
                            auto sym = symbolTable.findSymbol(firstParam->value, 0);
                            // stack grows downwards which means
                            // that for pointer subtraction means addition and reverse
                            if(sym.type[0] == '^')
                            {
                                auto sizeOf = typeSizeOfMap[sym.type];
                                for(int i = 0; i < sizeOf; ++i) {
                                    i_vector.push_back({std::byte(0x05)}); // add eax, rhsValue
                                    i_vector.push_back(i_vector.int_to_bytes(rhsValue));
                                }
                            }
                            else
                            {
                                i_vector.push_back({std::byte(0x2D)}); // sub eax, rhsValue
                                i_vector.push_back(i_vector.int_to_bytes(rhsValue));
                            }
                        }
                        if (binOp->value == "*")
                        {
                            //69 C0 rhsValue  imul        eax, eax, rhsValue
                            i_vector.push_back({ std::byte(0x69), std::byte(0xC0) }); 
                            i_vector.push_back(i_vector.int_to_bytes(rhsValue));

                        }
                        if (binOp->value == "/")
                        {
                            // push ebx
                            i_vector.push_back({ std::byte(0x53) });
                            // cdq sign-extend EAX into EDX
                            i_vector.push_back({ std::byte(0x99) });
                            // mov ebx, rhsValue
                            i_vector.push_back({ std::byte(0xBB)});
                            i_vector.push_back(i_vector.int_to_bytes(rhsValue));
                            // idiv ebx
                            i_vector.push_back({ std::byte(0xF7), std::byte(0xFB) });
                            // pop ebx
                            i_vector.push_back({ std::byte(0x5B) });
                        }
                        if (binOp->value == "==")
                        {
                            comparisonOperatorValue(rhsValue, insertJNE);
                        }
                        if (binOp->value == "!=")
                        {
                            comparisonOperatorValue(rhsValue, insertJE);
                        }
                        if (binOp->value == "<")
                        {
                            comparisonOperatorValue(rhsValue, insertJNL);
                        }
                        if (binOp->value == ">")
                        {
                            comparisonOperatorValue(rhsValue, insertJNG);
                        }
                        if (binOp->value == ">=")
                        {
                            comparisonOperatorValue(rhsValue, insertJNGE);
                        }
                        if (binOp->value == "<=")
                        {
                            comparisonOperatorValue(rhsValue, insertJNLE);
                        }
                    }
                    auto currentAllocationLevel = scopeId.top().first;
                    unsigned int variablePosition = calculateVariablePositionOnStack(lhsSymbol, currentAllocationLevel, allocs);
                    // mov [ebp - ebpOffset], eax
                    i_vector.push_back({ std::byte(0x89), std::byte(0x45), std::byte(variablePosition) });
                }
                break;
            }
        }   
    }
    void visitPost(const FunctionCall* fcall) 
    {   
        // push params
        // parameters are passed on the stack in reverse order from right to left
        for (auto it = fcall->parameters.rbegin(); it != fcall->parameters.rend(); ++it)
        {
            const auto& param = *it;
            if (std::isdigit(param[0])) {
                i_vector.push_back({ std::byte(0x68) }); // push
                i_vector.push_back(i_vector.int_to_bytes(std::stoi(param)));
            }
            if (std::isalpha(param[0])) {
                // if it is not value check index on the stack
                // and copy value that is indexed by this index
                // FF 75 FC           push        dword ptr [ebp-4]
                auto sym = symbolTable.findSymbol(param, 0);
                auto currentAllocationLevel = scopeId.top().first;

                unsigned int variablePosition = calculateVariablePositionOnStack(sym, currentAllocationLevel, allocs);
                i_vector.push_back({ std::byte(0xFF), std::byte(0x75), std::byte(variablePosition) });
            }
        }

        i_vector.push_back({ std::byte(0xB8) });  // \  mov eax, address of function
        
        symbolTable.findSymbol(fcall->name, 0);

        auto functionIterator = functionMap.find(fcall->name);

        if (functionIterator != functionMap.end())
        {
            i_vector.push_back(i_vector.get_address(reinterpret_cast<void*>(functionIterator->second)));
        }
        i_vector.push_back({ std::byte(0xFF), std::byte(0xD0) }); // call eax


        for (const auto& param : fcall->parameters)
        {
            i_vector.push_back({std::byte(0x83), std::byte(0xC4), std::byte(0x04)}); // add esp, 4 (clean stack)
        }

    }

    void visitPre(const IfStatement* ifstatement)
    {
        auto gotoStatement = cast<GotoStatement>(ifstatement->statements[0]);
        gotoLabelsFromIf.insert(gotoStatement->label);
    }

    void visitPost(const IfStatement* ifstatement)
    {
        auto gotoStatement = cast<GotoStatement>(ifstatement->statements[0]);
    	auto conditionVariable = cast<BasicExpression>(ifstatement->condition.getChilds()[1]);
        
        auto sym = symbolTable.findSymbol(conditionVariable->value, 0);
        auto currentAllocationLevel = scopeId.top().first;
        unsigned int variablePosition = calculateVariablePositionOnStack(sym, currentAllocationLevel, allocs);
        // pushf
        // i_vector.push_back({ std::byte(0x66), std::byte(0x9C) });

        // mov eax, 0
        i_vector.push_back({ std::byte(0xB8) });
        i_vector.push_back(i_vector.int_to_bytes(0));

        // cmp eax, dword ptr[ebp - ebpOffset]
        i_vector.push_back({ std::byte(0x3B), std::byte(0x45), std::byte(variablePosition) });

        insertJE(i_vector);

        // this is just a placeholder
        constexpr auto labelOffset = 0;
        i_vector.push_back({ i_vector.int_to_bytes(labelOffset) });
        auto pos = i_vector.size();

        jumpTable.insert(std::make_pair(gotoStatement->label, pos));        
    }

    void visitPre(const LabelStatement* stmt)
    {
	    labelToCodePosition[stmt->label] = i_vector.size() ;
    }

    void visitPost(const LabelStatement* stmt)
    {
        constexpr size_t addressSize = 4;
        // try to fix jumps
        for (auto& element : jumpTable)
        {   
            if (element.first != stmt->label) continue;
            auto jumpOffset = std::distance(i_vector.begin(), i_vector.begin() + i_vector.size() - element.second);
            auto bytes = i_vector.int_to_bytes(jumpOffset);
            
            for (size_t i = 0; i < addressSize; ++i)
            {
                *(i_vector.begin() + element.second - addressSize + i) = bytes[i];
            }           
            // popf
            // i_vector.push_back({ std::byte(0x66), std::byte(0x9D) });
        }

    }

    void visitPost(const GotoStatement* stmt)
    {
	    if(gotoLabelsFromIf.find(stmt->label) == gotoLabelsFromIf.end())
	    {

	        //std::cout << "current offset:" << i_vector.size() << std::endl;
	        //std::cout << "goto label:" << stmt->label << " << " << labelToCodePosition[stmt->label] << std::endl;
	        X86InstrVector tmp_vector;
		// jump is coded by e9 and four bytes for offset
                constexpr auto sizeofJump = 5;
	        int offset = labelToCodePosition[stmt->label] - sizeofJump - i_vector.size();

	        if (offset < 0)
	        {
	            // negative offset jmp e9
	            // std::cout << "negative offset:" << offset << std::endl;
	            i_vector.push_back({std::byte(0xe9)});
	            i_vector.push_back(i_vector.int_to_bytes(offset));
	            //std::cout << "temp vector : " << tmp_vector.size() << std::endl;
	        }
	        else
	        {

	        }
	    }
    }

    StatementList getStatements() const { return statements; }

private:
    StatementList statements;
    BasicSymbolTable& symbolTable;
    X86InstrVector& i_vector;
    size_t allocationLevel = 1;
    std::map<size_t, size_t> allocationLevelIndex;
    std::map<std::pair<size_t, size_t>, size_t> allocs;

    // jumpTable contains a label as key and list of jmp instruction pointers
    // these pointers point to placeholders at first and are fixed
    // during label traversal
    std::multimap<std::string, size_t> jumpTable;

    // this set contains labels that are connected to if statements
    // we have to omit them during visiting gotos as
    // code for them was already generated during
    // visiting if statement
    GotoLabelsFromIf gotoLabelsFromIf;

    // each label points to specific position in code
    LabelToCodePosition labelToCodePosition;

    std::stack<std::pair<size_t, size_t>> scopeId;

    // key is a pair that uniquely identifying scope
    std::map<std::pair<size_t, size_t>, size_t> variable_position_on_stack_map;

    std::map<std::string, void*> functionMap;

    void insertCmpVariable(unsigned int variablePosition)
    {
        // cmp eax, dword ptr[ebp - ebpOffset]
        i_vector.push_back({ std::byte(0x3B), std::byte(0x45), std::byte(variablePosition) });
    }
    void insertCmpValue(int value)
    {
        // cmp eax, rhsValue
        i_vector.push_back({ std::byte(0x3D) });
        i_vector.push_back(i_vector.int_to_bytes(value));
    }

    static void insertJG(X86InstrVector& i_vector)
    {
        i_vector.push_back({ std::byte(0x0F), std::byte(0x8F) });
    }
    static void insertJNG(X86InstrVector& i_vector)
    {
        i_vector.push_back({ std::byte(0x0F), std::byte(0x8E) });
    }
    static void insertJNL(X86InstrVector& i_vector)
    {
        i_vector.push_back({ std::byte(0x0F), std::byte(0x8D) });
    }
    static void insertJE(X86InstrVector& i_vector)
    {
        i_vector.push_back({ std::byte(0x0F), std::byte(0x84) });
    }
    static void insertJNE(X86InstrVector& i_vector)
    {
        i_vector.push_back({ std::byte(0x0F), std::byte(0x85) });
    }
    static void insertJNLE(X86InstrVector& i_vector)
    {
        i_vector.push_back({ std::byte(0x0F), std::byte(0x8F) });
    }
    static void insertJNGE(X86InstrVector& i_vector)
    {
        i_vector.push_back({ std::byte(0x0F), std::byte(0x8C) });
    }

    void comparisonOperatorVariable(unsigned int variablePosition, std::function<void(X86InstrVector& i_vector)> operatorOpcode)
    {
        // pushf
        i_vector.push_back({ std::byte(0x66), std::byte(0x9C) });

        insertCmpVariable(variablePosition);

        constexpr auto value0Offset = 10;
        operatorOpcode(i_vector);

        i_vector.push_back({ i_vector.int_to_bytes(value0Offset) }); // omit next 10 bytes
        // label_value_1:
        // mov eax,1
        i_vector.push_back({ std::byte(0xB8) });
        i_vector.push_back(i_vector.int_to_bytes(1));

        // jump 5 bytes
        constexpr auto endOffset = 5;
        i_vector.push_back({ std::byte(0xE9) });
        i_vector.push_back(i_vector.int_to_bytes(endOffset));
        // label_value_0 :
        i_vector.push_back({ std::byte(0xB8) });
        i_vector.push_back(i_vector.int_to_bytes(0));

        // popf
        i_vector.push_back({ std::byte(0x66), std::byte(0x9D) });
    }

    void comparisonOperatorValue(int value, std::function<void(X86InstrVector& i_vector)> operatorOpcode)
    {
        // pushf
        i_vector.push_back({ std::byte(0x66), std::byte(0x9C) });

        insertCmpValue(value);

        constexpr auto value0Offset = 10;
        operatorOpcode(i_vector);

        i_vector.push_back({ i_vector.int_to_bytes(value0Offset) }); // omit next 10 bytes
        // label_value_1:
        // mov eax,1
        i_vector.push_back({ std::byte(0xB8) });
        i_vector.push_back(i_vector.int_to_bytes(1));

        // jump 5 bytes
        constexpr auto endOffset = 5;
        i_vector.push_back({ std::byte(0xE9) });
        i_vector.push_back(i_vector.int_to_bytes(endOffset));
        // label_value_0 :
        i_vector.push_back({ std::byte(0xB8) });
        i_vector.push_back(i_vector.int_to_bytes(0));

        // popf
        i_vector.push_back({ std::byte(0x66), std::byte(0x9D) });
    }
};

auto emitMachineCode(const StatementList& statements, const std::map<std::string, void*>& functionMap)
{
    X86InstrVector i_vector;
    i_vector.push_function_prolog();
    BasicSymbolTable symbolTable;

    for(const auto& function : functionMap)
    {
        symbolTable.insertSymbol(function.first, "function");
    }

    PreAllocationPass preallocPass;
    traverse(statements, preallocPass);

    Basicx86Emitter visitor(i_vector, preallocPass.getAllocationVector(), symbolTable, functionMap);

    traverse(statements, visitor);
    
    i_vector.push_function_epilog();

    return i_vector;
}

