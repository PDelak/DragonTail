start : statement*;
statement : basic_statement ;
basic_statement : var_statement ';' | expr_statement ';' | if_statement | block | while_loop;
expr_statement : expr;
expr : expr op expr | 
		   expr op expr | 
		   expr op expr |
		   expr op expr |
		   id | 
		   number;
var_statement : 'var' id ;
if_statement : 'if' '(' expr ')' '{' statement* '}';
while_loop : 'while' '(' expr ')' '{' statement* '}';
block : '{' statement* '}';
op: '=' | '+' | '-' | '*' | '/';
id : "[a-zA-Z]*";
number              : "[0-9]*" ;