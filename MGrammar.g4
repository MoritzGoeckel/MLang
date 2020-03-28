grammar MGrammar;

r                           : (statement | COMMENT)* EOF ;
COMMENT                     : '#' ~( '\r' | '\n' )* ;
statement                   : (expr | ret) ';' ; // TODO: seperate by \n

expr                        : nr_expr               |
                              infix_call            ;

nr_expr                     : '(' expr ')'          |
                              assignment            |
                              block                 |
                              call                  |
                              literal               |
                              identifier            ;

ret                         : 'ret' expr ;

assignment                  : assignment_left '=' assignment_right ;
assignment_left             : identifier    |
                              variable_decl |
                              function_decl ;
assignment_right            : expr;

variable_decl               : 'let' identifier ;

identifier_list             : (identifier ',')* identifier ;
function_decl               : 'let' identifier '(' identifier_list? ')' ;

infix_call                  : infix_call_left identifier infix_call_right ;
infix_call_left             : nr_expr ;
infix_call_right            : expr ;

IDENTIFIER                  : [a-zA-Z+\-><*/_]+[a-zA-Z+\-><*/_0-9]* ;
identifier                  : IDENTIFIER ;

argument_list               : (expr ',')* expr ;
call                        : identifier '(' argument_list? ')' ;

block                       : '{' statement* '}' ;

literal                     : type_float  |
                              type_int    |
                              type_string ;

type_float                  : FLOAT ;
type_int                    : INTEGER ;
type_string                 : STRING ;

FLOAT                       : [0-9]+ '.' [0-9]+ ;
INTEGER                     : [0-9]+ ;
STRING                      : '"' [a-zA-Z0-9]* '"' ;

WHITESPACE                  : [ \t\r\n]+ -> skip ;
