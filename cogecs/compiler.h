#ifndef LANGUAGE_COMPILER_H
#define LANGUAGE_COMPILER_H

#include <set>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <functional>
#include <cctype>
#include "dparse.h"
#include "ast.h"
#include "astvisitor.h"

constexpr size_t MAX_LINE_LENGTH = 44;  /* must be at least 4 */
constexpr size_t INDENT_SPACES = 4;
extern D_ParserTables parser_tables_gram;

typedef void (visit_node_fn_t)(const std::string& token_name, const std::string& token_value, StatementStack&, StatementList& statementList, size_t& scope, AstVisitor& visitor);

static void
traverse_tree(D_ParserTables pt, D_ParseNode *pn, visit_node_fn_t pre, visit_node_fn_t post, StatementStack& stmtStack, StatementList& statementList, size_t& scope, AstVisitor& visitor) {
    
    int len = pn->end - pn->start_loc.s;
    std::string val = std::string(pn->start_loc.s, pn->start_loc.s + len);
    std::string name = std::string(pt.symbols[pn->symbol].name);

    pre(name.c_str(), const_cast<char*>(val.c_str()), stmtStack, statementList, scope, visitor);
         
    int nch = d_get_number_of_children(pn);
    for (int i = 0; i < nch; i++) {
        D_ParseNode *xpn = d_get_child(pn, i);
        traverse_tree(pt, xpn, pre, post, stmtStack, statementList, scope, visitor);        
    }
    post(name, val, stmtStack, statementList, scope, visitor);
	
}
static char *
change_newline2space(char *s) {
    char *ss = s;
    while (*ss++)
        if (*ss == '\n')
            *ss = ' ';
    if (strlen(s)>MAX_LINE_LENGTH) {
        *(s + MAX_LINE_LENGTH - 3) = '.';
        *(s + MAX_LINE_LENGTH - 2) = '.';
        *(s + MAX_LINE_LENGTH - 1) = '.';
        *(s + MAX_LINE_LENGTH) = '\0';
    }
    return s;
}

void
visit_node(const std::string& name, const std::string& value, StatementStack&, StatementList&, size_t& scope) {
    printf("%*s", static_cast<int>(scope*INDENT_SPACES), "");
    printf("%s  %s.\n", name.c_str(), change_newline2space(const_cast<char*>(value.c_str())));
}


void
pre_visit_node(const std::string& name, const std::string& value, StatementStack& stmtStack, StatementList&, size_t& scope, AstVisitor&) {
	if (name == "id" || name == "op" || name == "number" || name == "not" || name == "addr" || name == "dereference")
	{
		std::string temp = value;
		auto it = std::remove_if(temp.begin(), temp.end(), isspace);
		temp.erase(it, temp.end());
		stmtStack.push_back(temp);
	}
	else 
	{
		std::set<std::string> rules = { 
			"var_statement", 
			"expr_statement",
			"if_statement",
			"block_statement",	
			"while_loop",
			"label",
			"goto_statement",
			"function_call",
			"return_statement",
			"function_decl"
		};
		if (rules.find(name) != rules.end()) stmtStack.push_back(name);
	}
	std::set<std::string> compound_rules = { 
		"if_statement",
		"block_statement",
		"while_loop",	
		"function_decl"
	};
	// increase scope number only for compound rules
	if (compound_rules.find(name) != compound_rules.end()) ++scope;

}

Expression::ElementsType moveExpressionFromStackToNode(StatementStack& stmtStack, const std::string& statementName, size_t scope, StatementList& statementList)
{
	Expression::ElementsType elements;
	StatementList fCalls;

	for (const auto& statement : statementList) {
		if (dynamic_cast<FunctionCall*>(statement.get())) {
			fCalls.push_back(statement);
		}
	}

	auto statementIt = std::find(stmtStack.rbegin(), stmtStack.rend(), statementName);
	std::for_each(statementIt.base(), stmtStack.rbegin().base(), [&](const std::string& s) {
		if (s == "function_call") {
			auto fcall = fCalls.begin();			
			elements.push_back(*fcall);
			fCalls.erase(fcall);			
			statementList.erase(--statementList.rbegin().base());
		}
		else {
			elements.push_back(makeNode(BasicExpression(scope, s)));
		}
		
	});
	return elements;
}

void clearStmtStackFor(const std::string& statementName, StatementStack& stmtStack)
{
	auto ifBegin = std::find(stmtStack.rbegin(), stmtStack.rend(), statementName);
	stmtStack.erase(--ifBegin.base(), stmtStack.rbegin().base());
}

void moveStatementsToNode(StatementList& src, StatementList& dst, size_t scope)
{
	auto begin = src.rbegin();

	auto it = std::find_if(src.rbegin(), src.rend(), [&](const StatementPtr& statement) {
		return statement->scope != scope;
	});

	std::copy(it.base(), begin.base(), std::back_inserter(dst));
	src.erase(it.base(), begin.base());
}

template<typename NodeType>
void addAstCompoundNode(StatementList& statementList, StatementStack& stmtStack, size_t scope, const std::string& nodeName, const std::shared_ptr<NodeType>& node)
{
	moveStatementsToNode(statementList, node->statements, scope);
	statementList.push_back(node);

	clearStmtStackFor(nodeName, stmtStack);
}

void
post_visit_node(const std::string& name, const std::string&, StatementStack& stmtStack, StatementList& statementList, size_t& scope, AstVisitor& visitor) {
	
	if (name == "goto_statement") {
		auto begin = stmtStack.rbegin();
		auto label = *begin;
		auto i = stmtStack.erase(std::next(begin).base());
		stmtStack.erase(--i);
		auto node = std::make_shared<GotoStatement>(scope);
		node->label = label;
		statementList.push_back(node);
		visitor.visitPost(node.get());
	}

	if (name == "label") {
		auto begin = stmtStack.rbegin();
		auto label = *begin;
		auto i = stmtStack.erase(std::next(begin).base());
		stmtStack.erase(--i);
		auto node = std::make_shared<LabelStatement>(scope);	
		node->label = label;
		statementList.push_back(node);
		visitor.visitPost(node.get());		
	}

	if (name == "var_statement") {
		auto begin = stmtStack.rbegin();
		auto var_name = *begin;
		auto i = stmtStack.erase(std::next(begin).base());
		stmtStack.erase(--i);
		auto node = std::make_shared<VarDecl>(scope);
		node->var_name = var_name;
		statementList.push_back(node);
		visitor.visitPost(node.get());
	}
	if (name == "expr_statement") {
		auto node = std::make_shared<Expression>(scope);		
		auto elems = moveExpressionFromStackToNode(stmtStack, "expr_statement", scope, statementList);
		node->setElements(elems);
		statementList.push_back(node);
		clearStmtStackFor("expr_statement", stmtStack);
		visitor.visitPost(node.get());
	}
	if (name == "if_statement") {
		auto node = std::make_shared<IfStatement>(scope - 1);
		node->condition.isPartOfCompoundStmt = true;
		auto elems = moveExpressionFromStackToNode(stmtStack, "if_statement", scope, statementList);
		node->condition.setElements(elems);
		addAstCompoundNode<IfStatement>(statementList, stmtStack, scope, "if_statement", node);
		visitor.visitPost(node.get());
		--scope;
	}
	if (name == "block_statement") {
		auto node = std::make_shared<BlockStatement>(scope - 1);
		addAstCompoundNode<BlockStatement>(statementList, stmtStack, scope, "block_statement", node);
		visitor.visitPost(node.get());
		--scope;
	}
	if (name == "while_loop") {
		auto node = std::make_shared<WhileLoop>(scope - 1);
		node->condition.isPartOfCompoundStmt = true;
		auto elems = moveExpressionFromStackToNode(stmtStack, "while_loop", scope, statementList);
		node->condition.setElements(elems);
		addAstCompoundNode<WhileLoop>(statementList, stmtStack, scope, "while_loop", node);
		visitor.visitPost(node.get());
		--scope;
	}
	if (name == "function_call") {
		auto node = std::make_shared<FunctionCall>(scope);		
		auto lastParam = stmtStack.rbegin();
		auto functionCall = std::find(lastParam, stmtStack.rend(), "function_call");
		auto functionName = functionCall.base();
		node->name = *functionName;
		if (distance(functionName, lastParam.base()) > 0) {
			auto firstParam = std::next(functionName);
			std::copy(firstParam, lastParam.base(), std::back_inserter(node->parameters));
		}
		stmtStack.erase(functionName, lastParam.base());
		statementList.push_back(node);
		visitor.visitPost(node.get());
	}
	
	if (name == "return_statement") {
		auto begin = stmtStack.rbegin();
		auto param = *begin;
		auto i = stmtStack.erase(std::next(begin).base());
		stmtStack.erase(--i);
		auto node = std::make_shared<ReturnStatement>(scope);
		node->param = param;
		statementList.push_back(node);
		visitor.visitPost(node.get());
	}

	if (name == "function_decl") {		
		auto node = std::make_shared<FunctionDecl>(scope - 1);		
		auto lastParam = stmtStack.rbegin();
		auto functionDecl = std::find(lastParam, stmtStack.rend(), "function_decl");
		auto functionName = functionDecl.base();
		node->name = *functionName;
		if (distance(functionName, lastParam.base()) > 0) {
			auto firstParam = std::next(functionName);
			std::copy(firstParam, lastParam.base(), std::back_inserter(node->parameters));
		}
		stmtStack.erase(functionName, lastParam.base());
		addAstCompoundNode<FunctionDecl>(statementList, stmtStack, scope, "function_decl", node);
		visitor.visitPost(node.get());
		--scope;
	}
}


void
print_parsetree(D_ParserTables pt, D_ParseNode *pn, visit_node_fn_t pre, visit_node_fn_t post, StatementList& statementList, size_t& scope, AstVisitor& visitor) {        
	StatementStack stmtStack;
	traverse_tree(pt, pn, pre, post, stmtStack, statementList, scope, visitor);
}


class FileNotFoundException : public std::runtime_error
{
public:
    FileNotFoundException(const char* what) :std::runtime_error(what) {}
};

StatementList parse(D_Parser *p, char* begin, char* end, AstVisitor& visitor)
{
	StatementList statementList;
	auto pn = dparse(p, begin, std::distance(begin, end));
	if (p->syntax_errors) {
		printf("compilation failure %d %s\n", p->loc.line, p->loc.pathname);
		return statementList;
	}
	size_t scope = 0;
	print_parsetree(parser_tables_gram, pn, pre_visit_node, post_visit_node, statementList, scope, visitor);
	return statementList;
}

StatementList compile(const std::string& file, D_Parser *p, AstVisitor& visitor)
{
	std::ifstream in(file);
	if (!in.is_open()) throw FileNotFoundException("FileNotFound");
	in.unsetf(std::ios::skipws);
	std::istream_iterator<char> begin(in);
	std::istream_iterator<char> end;

	std::vector<char> buffer;
	std::copy(begin, end, std::back_inserter(buffer));
	char* b = &buffer[0];
	char* e = &buffer[0] + buffer.size();
	return parse(p, b, e, visitor);
}

typedef std::unique_ptr<D_Parser, std::function<void(D_Parser*)>> ParserPtr;

ParserPtr initialize_parser(const std::string& filename)
{
	ParserPtr parser(new_D_Parser(&parser_tables_gram, 1), [](D_Parser* p) { free_D_Parser(p);});
	parser->save_parse_tree = 1;
	parser->loc.pathname = const_cast<char*>(filename.c_str());
	return parser;
}

ParserPtr initialize_parser()
{
	ParserPtr parser(new_D_Parser(&parser_tables_gram, 1), [](D_Parser* p) { free_D_Parser(p); });
	parser->save_parse_tree = 1;
	return parser;
}

#endif
