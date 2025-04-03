grammar MySimpleUI;

// PARSER RULES (Structure)

program
    : stateDecl* rootComponent EOF
    ;

stateDecl
    : STATE type=ID name=ID '=' value=expression ';'
    ;

rootComponent
    : componentCall
    ;

componentCall
    : componentName=ID '(' parameterList? ')' '{' element* '}'
    ;

parameterList
    : parameter (',' parameter)*
    ;

parameter
    : BIND? expression // 'bind' is optional before an expression (usually an identifier)
    ;

element
    : propertyAssignment
    | componentCall // Nested component
    ;

propertyAssignment
    : propertyName=ID ':' value=expression ';'
    ;

// Common rule for values/expressions in this simple subset
expression
    : literal
    | identifier // Includes variable names, delegate names, value keywords
    ;

literal
    : NUMBER
    | STRING
    ;

identifier // Allow '@' prefix for delegates specifically if needed, or handle in visitor
    : ID
    | AT_ID // Treat @delegateName as a specific token type
    ;


// LEXER RULES (Tokens)

// Keywords
STATE: 'state';
BIND: 'bind';

// Literals - Ensure NUMBER handles floats and ints
NUMBER: [0-9]+ ('.' [0-9]+)? ;
STRING: '"' ( ~["\r\n] )*? '"' ; // Simple string, doesn't handle escapes

// Identifiers
ID: [a-zA-Z_] [a-zA-Z_0-9]* ;
AT_ID: '@' [a-zA-Z_] [a-zA-Z_0-9]* ; // For delegate handles like @OnClick

// Punctuation
LPAREN: '(';
RPAREN: ')';
LBRACE: '{';
RBRACE: '}';
COLON: ':';
SEMI: ';';
ASSIGN: '=';
COMMA: ',';

// Comments and Whitespace - skip them
WS: [ \t\r\n]+ -> skip ;
LINE_COMMENT: '//' ~[\r\n]* -> skip ;
// BLOCK_COMMENT: '/*' .*? '*/' -> skip ; // Optional: Add if needed