#pragma once

#include <unordered_map>
#include <list>
#include <string>
#include <utility>

struct symbol
{
	symbol(const std::string& id, const std::string& type, unsigned char stack_pos = -1):id(id), type(type), stack_position(stack_pos) {}
	std::string id;
	std::string type;
	unsigned char stack_position;
};

typedef std::list<symbol> symbol_list;

typedef std::list<symbol>::iterator symbol_iterator;

typedef std::unordered_map<size_t, symbol_list> basic_symbol_table_type;

typedef std::unordered_map<size_t, symbol_list>::iterator symbol_table_iterator;

struct SymbolNotFound : public std::runtime_error
{
  SymbolNotFound(const std::string& symbol, const std::string lineno) 
  :std::runtime_error("Undefined symbol : " + symbol + " in " + lineno + " line ") {}
};


struct BasicSymbolTable
{
  BasicSymbolTable():symbol_table_id(0) {}

  void enterScope()
  {
    ++symbol_table_id;
  }

  void exitScope()
  {
    symbol_table_iterator table_iterator = symbol_table.find(symbol_table_id);
    if (table_iterator != symbol_table.end()) symbol_table.erase(table_iterator);
    --symbol_table_id;
  }
  
  void insertSymbol(const std::string& id, const std::string& type, unsigned char position_on_stack = -1)
  {
    symbol_table[symbol_table_id].push_back(symbol(id, type, position_on_stack));
  }
  
  void dump()
  {
	  for (const auto& bucket : symbol_table)
	  {
		  for (const auto& symbol : bucket.second)
		  {
			  std::cout << bucket.first << " : " << "(" << symbol.id << "," << symbol.type << ")" << std::endl;
		  }
	  }
  }

  bool exists(const std::string& id)
  {
	  int local_table_id = symbol_table_id;
	  do {
		  symbol_iterator it = symbol_table[local_table_id].begin();
		  while (it != symbol_table[local_table_id].end()) {
			  if (it->id.compare(id) == 0) return true;
			  it++;
		  }
		  --local_table_id;
	  } while (local_table_id > 0);
	  return false;
  }

  symbol findSymbol(const std::string& id, size_t lineno)
  {
    int local_table_id = symbol_table_id;
    do {
      symbol_iterator it = symbol_table[local_table_id].begin();
      while (it != symbol_table[local_table_id].end()) {
        if (it->id.compare(id) == 0) return *it;
        it++;
      }
      --local_table_id;
	} while (local_table_id > 0);
    std::stringstream ss;
    ss << lineno;
    throw SymbolNotFound(id, ss.str());
  }

  int symbol_table_id;
  basic_symbol_table_type symbol_table;

};

