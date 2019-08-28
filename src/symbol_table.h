#pragma once

#include <unordered_map>
#include <list>
#include <string>
#include <utility>

struct symbol
{
    symbol(const std::string& id, const std::string& type, unsigned char stack_pos = -1, size_t s = 0):id(id), type(type), stack_position(stack_pos), scope(s) {}
    std::string id;
    std::string type;
    unsigned char stack_position;
    size_t scope = 0;
    size_t allocation_level = 0;
    size_t level_index = 0;
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
  
  void insertSymbol(const std::string& id, const std::string& type, unsigned char position_on_stack = -1, size_t level = 0, size_t index = 0)
  {
    auto new_symbol = symbol(id, type, position_on_stack, symbol_table_id);
    new_symbol.allocation_level = level;
    new_symbol.level_index = index;
    symbol_table[symbol_table_id].push_back(new_symbol);
  }
  
  void dump() const
  {
      for (const auto& bucket : symbol_table)
      {
          for (const auto& symbol : bucket.second)
          {
              std::cout << bucket.first << " : " << "(" << symbol.id << "," << symbol.type << "," << static_cast<int>(symbol.stack_position) << ")" << std::endl;
          }
      }
  }

  size_t numberOfVariablesPerScope(int scope) const 
  {
      auto it = symbol_table.find(scope);
      if (it == symbol_table.end()) throw 1;
      return it->second.size();
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
      } while (local_table_id > -1);
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
    } while (local_table_id > -1);
    std::stringstream ss;
    ss << lineno;
    throw SymbolNotFound(id, ss.str());
  }

  int symbol_table_id;
  basic_symbol_table_type symbol_table;

};

