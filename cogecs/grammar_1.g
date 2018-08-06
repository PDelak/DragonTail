start : statement*;
statement : (add_call | mult_call | sort_call | filter_call) ';';
add_call : 'add' number;
mult_call : 'mult' number;
sort_call : 'sort';
filter_call : 'filter' logical_expression number;
logical_expression : ( '==' | '<=' | '>=');
number              : "[0-9]*" ;