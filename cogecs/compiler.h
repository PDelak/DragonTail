#ifndef LANGUAGE_COMPILER_H
#define LANGUAGE_COMPILER_H

const size_t MAX_LINE_LENGTH = 44;  /* must be at least 4 */
const size_t INDENT_SPACES = 4;
extern D_ParserTables parser_tables_gram;

typedef void (visit_node_fn_t)(int depth, const std::string& token_name, const std::string& token_value, StatementStack&, StatementList& statementList, size_t& scope);

static void
traverse_tree(D_ParserTables pt, D_ParseNode *pn, int depth, visit_node_fn_t pre, visit_node_fn_t post, StatementStack& stmtStack, StatementList& statementList, size_t& scope) {
    
    int len = pn->end - pn->start_loc.s;
    std::string val = std::string(pn->start_loc.s, pn->start_loc.s + len);
    std::string name = std::string(pt.symbols[pn->symbol].name);

    pre(depth, name.c_str(), const_cast<char*>(val.c_str()), stmtStack, statementList, scope);
    
    depth++;
    
    int nch = d_get_number_of_children(pn);
    for (int i = 0; i < nch; i++) {
        D_ParseNode *xpn = d_get_child(pn, i);
        traverse_tree(pt, xpn, depth, pre, post, stmtStack, statementList, scope);        
    }
    post(depth, name, val, stmtStack, statementList, scope);
	
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
visit_node(int depth, const std::string& name, const std::string& value, StatementStack& stmtStack, StatementList& statementList, size_t& scope) {
    printf("%*s", depth*INDENT_SPACES, "");
    printf("%s  %s.\n", name.c_str(), change_newline2space(const_cast<char*>(value.c_str())));
}

void
pre_visit_node(int depth, const std::string& name, const std::string& value, StatementStack& stmtStack, StatementList& statementList, size_t& scope) {
	if (name == "var_statement") stmtStack.push_back("var_statement");
	if (name == "expr_statement") stmtStack.push_back("expr_statement");
	if (name == "id" || name == "op" || name == "number") stmtStack.push_back(value);
	if (name == "if_statement") {
		stmtStack.push_back("if_statement");
		++scope;
	}
}

void
post_visit_node(int depth, const std::string& name, const std::string& value, StatementStack& stmtStack, StatementList& statementList, size_t& scope) {
	
	if (name == "var_statement") {
		auto begin = stmtStack.rbegin();
		auto var_name = *begin;
		auto i = stmtStack.erase(std::next(begin).base());
		stmtStack.erase(--i);
		auto node = std::make_shared<VarDecl>();
		node->scope = scope;
		node->var_name = var_name;
		statementList.push_back(node);
	}
	if (name == "expr_statement") {
		auto node = std::make_shared<Expression>();
		node->scope = scope;
		auto begin = stmtStack.rbegin();
		auto it = std::next(begin).base();
		while (*it != "expr_statement") {			
			node->elements.insert(node->elements.begin(), *it);
			--it;			
		}
		statementList.push_back(node);
		stmtStack.erase(it, begin.base());
	}
	if (name == "if_statement") {
		// move all statements with if scope to if 
		auto begin = statementList.rbegin();
		auto it = std::next(begin).base();
		while (it != statementList.begin()) {
			if ((*it)->scope != scope) break;
			--it;
		}
		std::advance(it, 1);
		auto node = std::make_shared<IfStatement>();	
		node->scope = scope;
		std::copy(it, begin.base(), std::back_inserter(node->statements));
		statementList.erase(it, begin.base());
		statementList.push_back(node);
		--scope;
	}
}


void
print_parsetree(D_ParserTables pt, D_ParseNode *pn, visit_node_fn_t pre, visit_node_fn_t post, StatementList& statementList, size_t& scope) {        
	StatementStack stmtStack;
    traverse_tree(pt, pn, 0, pre, post, stmtStack, statementList, scope);
}


class FileNotFoundException : public std::runtime_error
{
public:
    FileNotFoundException(const char* what) :std::runtime_error(what) {}
};

StatementList parse(D_Parser *p, char* begin, char* end)
{
    auto pn = dparse(p, begin, std::distance(begin, end));
    if (p->syntax_errors) printf("compilation failure %d %s\n", p->loc.line, p->loc.pathname);
    StatementList statementList;
	size_t scope = 0;
    print_parsetree(parser_tables_gram, pn, pre_visit_node, post_visit_node, statementList, scope);
    return statementList;
}

StatementList compile(const std::string& file, D_Parser *p)
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
    return parse(p, b, e);
}

typedef std::unique_ptr<D_Parser, std::function<void(D_Parser*)>> ParserPtr;

ParserPtr initialize_parser(const std::string& filename)
{
    ParserPtr parser(new_D_Parser(&parser_tables_gram, 1), [](D_Parser* p) { free_D_Parser(p);});
    parser->save_parse_tree = 1;
    parser->loc.pathname = const_cast<char*>(filename.c_str());
    return parser;
}

#endif
