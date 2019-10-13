start : statement*;
statement : label? basic_statement;
basic_statement : var_statement ';' | expr_statement ';' | if_statement | block_statement | while_loop | function_decl | goto_statement ';' | return_statement ';';
expr_statement : expr;
expr : expr op expr  
       | not expr 
       | function_call 
       | addr id 
       | dereference id 
       | id
       | number;
var_statement : 'var' id (':' type)?;
if_statement : 'if' '(' expr ')' statement;
while_loop : 'while' '(' expr ')' statement;
block_statement : '{' statement* '}';
label : id ':' ;
goto_statement : 'goto' id;
function_call : id '(' param* ')';
function_decl : 'function' id '(' id* ')' block_statement ;
param : id | number;
op: '=' | '+' | '-' | '*' | '/' | '==' | '!=' | '<' | '<=' | '>=' | '>' | '&&' | '||';
id : "[@a-zA-Z]" "[a-zA-Z0-9_]*";
number : "[0-9]*";
not : '!';
addr : '&';
dereference : '*';
return_statement : 'return' param;
primitive_type : 'i8' | 'i16' | 'i32';
pointer_type : ('^')+ primitive_type;
type : pointer_type | primitive_type;
