grammar MGrammar;

r                           : statement* EOF ;
statement                   : expression ';' ;

expression                  : assignment |
                              operation  |
                              call       |
                              literal    |
                              IDENTIFIER ;

assignment                  : (IDENTIFIER | variable_declaration) '=' expression ;
operation                   : (IDENTIFIER | literal) OPERATOR expression ;

literal                     : type_float  |
                              type_int |
                              type_string ;

type_float                  : FLOAT ;
type_int                    : INTEGER ;
type_string                 : STRING ;

FLOAT                       : [0-9]+ '.' [0-9]+ ;
INTEGER                     : [0-9]+ ;
STRING                      : '"' [a-zA-Z0-9]* '"' ;

IDENTIFIER                  : [a-zA-Z]+[0-9]* ;

OPERATOR                    : '+' |
                              '-' |
                              '/' |
                              '*' |
                              '.' ;

brackets                    : '(' expression ')' ; // TODO

argument_list               : (expression ',')* expression ;

call                        : IDENTIFIER '(' argument_list ')' |
                              IDENTIFIER '()' ;

type                        : 'string' |
                              'int'    |
                              'void'   |
                              'float'  ;

variable_declaration        : type IDENTIFIER ; // Remove explicit types

variable_declaration_list   : (variable_declaration ',')* variable_declaration ;

method_declaration          : type IDENTIFIER '(' variable_declaration_list ')' block ; // Remove explicit types
block                       : '{' statement* '}' ;

WHITESPACE                  : [ \t\r\n]+ -> skip ;
