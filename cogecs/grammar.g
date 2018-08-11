start : statement*;
statement : basic_statement ';';
basic_statement : var_statement | expr_statement;
expr_statement : expr;
expr : expr op expr | 
		   expr op expr | 
		   expr op expr |
		   expr op expr |
		   id | 
		   number;
var_statement : 'var' id ;
op: '=' | '+' | '-' | '*' | '/';
id : "[a-zA-Z]*";
number              : "[0-9]*" ;