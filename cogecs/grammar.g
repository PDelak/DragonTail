start : statement*;
statement : var_statement ';';
var_statement : 'var' id ;
id : "[a-zA-Z]*";
number              : "[0-9]*" ;