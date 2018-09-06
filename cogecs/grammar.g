start : statement*;
statement : label? basic_statement;
basic_statement : var_statement ';' | expr_statement ';' | if_statement | block_statement | while_loop | goto_statement ';' ;
expr_statement : expr;
expr : expr op expr | 
		   expr op expr | 
		   expr op expr |
		   expr op expr |		   
		   not expr |
		   function_call |
		   id | 
		   number;
var_statement : 'var' id;
if_statement : 'if' '(' expr ')' statement;
while_loop : 'while' '(' expr ')' statement;
block_statement : '{' statement* '}';
label : id ':' ;
goto_statement : 'goto' id;
function_call : id '(' param* ')';
param : id | number;
op: '=' | '+' | '-' | '*' | '/' | '==' | '!=' | '<' | '<=' | '>';
id : "[a-zA-Z]*" "[_0-9]*";
number : "[0-9]*";
not : '!';