grammar MyceliumUI;

// Options
options {
	language = Cpp;
} // Specify C++ target for generated code

// --- PARSER RULES ---

program: definition* mainFunction? EOF;

definition:
	componentDefinition
	| styleOverride; // | globalStateDecl // Future: Maybe add top-level state?

// == Component Definition ==
componentDefinition:
	COMPONENT componentName = ID (':' baseComponentName = ID)? LBRACE componentMember* RBRACE;

componentMember:
	propDecl
	| getPropDecl
	| stateBlock
	| eventDecl
	| styleBlock // Base style
	| scriptBlock
	| instanceDefinition; // Allow nested component instances as children

propDecl: PROP type ID ('=' expression)? SEMI;

getPropDecl: GET PROP type ID ARROW expression SEMI;

stateBlock: STATE LBRACE stateVarDecl* RBRACE;

stateVarDecl: type ID ('=' expression)? SEMI;

eventDecl: EVENT ID (LPAREN paramList? RPAREN)? SEMI;

paramList: param (COMMA param)*;

param: type ID;

styleBlock: // Semicolons between assignments are now optional (last one also optional)
	STYLE LBRACE (styleAssignment SEMI?)* RBRACE;
// Allows { prop:val; prop2:val2 } or { prop:val prop2:val2 }

scriptBlock: SCRIPT LBRACE scriptBody RBRACE;

// == Style Override Definition ==
styleOverride:
	targetType = ID OVERRIDE STYLE LBRACE (styleAssignment SEMI?)* RBRACE;
// Changed from styleProperty, allow optional SEMI

// == Style Properties & Values ==
styleAssignment: // Represents one key: value pair
	propertyName = styleIdentifier COLON styleValue;
// Semicolon handled in styleBlock/styleOverride rules

styleIdentifier:
	ID (MINUS ID)*; // Allows hyphens, e.g., background-color

// A style value can be a sequence of simple primaries or a single complex expression
styleValue:
	primary (primary)*	# StyleValuePrimarySequence // Sequence like '0.1s ease', '10px', 'center'
	| expression		# StyleValueFullExpression;
// Complex single value: select{..}, calc{..}, color(), a * b, this.prop

// No longer needed: styleValueAtom
selectExpression: // Now also part of expression rule below
	SELECT LBRACE selectCase* (defaultCase)? RBRACE;

selectCase:
	expression COLON expression COMMA?; // Allow optional trailing comma

defaultCase:
	DEFAULT COLON expression COMMA?; // Allow optional trailing comma

calculateBlock: CALCULATE LBRACE scriptBody RBRACE;

// == Main Function (Entry Point) ==
mainFunction:
	VOID MAIN LPAREN RPAREN LBRACE programStatement* RBRACE; // Allow definitions or statements

programStatement: // Can be an instance or potentially other top-level statements later
	instanceDefinition;

// == Component Usage / Instantiation ==
instanceDefinition: // Represents using a component like `Button { ... }` or `Text {}`
	typeName = ID (instanceId = ID)? LBRACE instanceMember* RBRACE;

instanceMember:
	propAssignment SEMI?
	// Allow optional semicolon after prop assignment for consistency? Let's try optional.
	| eventHandler SEMI? // Allow optional semicolon after event handler? Let's try optional.
	| inlineStyle = styleBlock // Reuse styleBlock rule for inline styles
	| instanceDefinition; // Explicitly allow nested instances as members

propAssignment: // Property assignment within an instance block
	ID COLON expression; // Semicolon handled optionally in instanceMember rule

eventHandler: // Event handler assignment within an instance block
	ON ID COLON functionLiteral; // Semicolon handled optionally in instanceMember rule

// Function literal defined separately (reusable)
functionLiteral:
	FUNCTION (LPAREN paramList? RPAREN)? LBRACE scriptBody RBRACE;

// == Expressions == Define precedence later if needed. Start simple.
expression: // Order matters for precedence (higher precedence later)
	// Lowest precedence
	expression QUESTION expression COLON expression	# TernaryExpr
	| expression OR expression						# LogicalOrExpr
	| expression AND expression						# LogicalAndExpr
	| expression (EQ | NEQ) expression				# EqualityExpr
	| expression (LT | LTE | GT | GTE) expression	# ComparisonExpr
	| expression (ADD | MINUS) expression			# AddSubExpr
	| expression (MUL | DIV) expression				# MulDivExpr
	| NOT expression								# LogicalNotExpr
	// Higher precedence
	| expression DOT ID								# MemberAccessExpr // e.g., this.prop, state.value, input.PressDown
	| expression LPAREN argumentList? RPAREN		# FunctionCallExpr // e.g., color(), clamp()
	| ID DBL_COLON ID LPAREN argumentList? RPAREN	# StaticCallExpr // e.g., MasterVolume::setVolume()
	| selectExpression								# SelectExpr
	| calculateBlock								# CalculateExpr
	| functionLiteral								# FunctionLiteralExpr // Allow functions as expressions
	| primary										# PrimaryExpr // literal, identifier, THIS
	| LPAREN expression RPAREN						# ParenExpr // Highest precedence override
	| THIS											# ThisExpr; // THIS is a keyword token, covered by primary
// Removed: | EMIT ID (LPAREN argumentList? RPAREN)? SEMI # EmitExpr // Emit should be handled within script parsing if scriptBody is enhanced

primary: // Defined as a separate rule now
	literal // Includes NUMBER, STRING, booleanLiteral, colorLiteral, lengthLiteral, timeLiteral
	| identifier // Includes simple names like 'fit', 'ease', 'black' etc.
	| THIS; // 'this' is a primary value

argumentList: expression (COMMA expression)*;

// == Literals & Basic Types ==
literal:
	NUMBER
	| STRING
	| booleanLiteral
	//| colorLiteral // Parsed as FunctionCallExpr now: color(...)
	| lengthLiteral
	| timeLiteral; // Added time literal | NULL // If needed

booleanLiteral: TRUE | FALSE;

// colorLiteral removed as parser rule - parsed as a FunctionCallExpr: color(r,g,b) or
// color(r,g,b,a) Make sure 'color' is NOT a keyword if it's used as a function name identifier
// lengthLiteral: NUMBER (PX | PERCENT)?; // e.g., 10px, 100%, 5 timeLiteral: NUMBER SECONDS; //
// Defined in lexer rules now

identifier: ID; // Simple ID used for variables, types, etc.

type: // Used in declarations (prop, state, param)
	ID; // For now, assume types are simple IDs like 'string', 'float', 'Color'

// == Script Body Capture == WARNING: This rule is fragile. It consumes tokens until a matching
// RBRACE, which can fail with nested comments or strings containing braces. A lexer mode is the
// recommended robust solution.
scriptBody: codeItem*?;
codeItem: // Consume tokens until matching brace is found
	LBRACE scriptBody RBRACE // Handle nested braces
	| ~RBRACE; // Consume any token other than RBRACE

// --- LEXER RULES ---

// Keywords
COMPONENT: 'component';
PROP: 'prop';
GET: 'get';
STATE: 'state';
EVENT: 'event';
// EMIT removed as parser keyword, handled as ID in scriptBody
STYLE: 'style';
OVERRIDE: 'override';
SELECT: 'select';
DEFAULT: 'default';
SCRIPT: 'script';
THIS: 'this';
FUNCTION: 'function';
VOID: 'void';
MAIN: 'Main'; // Treat as keyword for the entry function
TRUE: 'true';
FALSE: 'false';
CALCULATE: 'calculate';
ON: 'on'; // For event handlers like onClicked:
// COLOR removed as keyword, allow it as function ID

// Identifiers (must come after keywords)
ID: [a-zA-Z_] [a-zA-Z_0-9]*;

// Literals
NUMBER:
	[0-9]+ ('.' [0-9]+)? [fF]?; // Allow optional 'f' suffix for floats
STRING:
	'"' ('\\' . | ~["\\\r\n])*? '"' // Handles basic escapes like \"
	| '\'' ('\\' . | ~['\\\r\n])*? '\''; // Allow single quotes too

// Units & Time combined into parser rules using lexer tokens
PX: 'px';
PERCENT: '%';
SECONDS: 's'; // Added for time values like 0.1s

// Parser rule combinations for specific literal types using lexer tokens above
lengthLiteral: NUMBER (PX | PERCENT)?; // e.g., 10px, 100%, 5
timeLiteral: NUMBER SECONDS; // e.g., 0.1s

// Operators and Punctuation
LPAREN: '(';
RPAREN: ')';
LBRACE: '{';
RBRACE: '}';
// LBRACK: '['; // Not used in example, keep commented out if not needed RBRACK: ']';
SEMI: ';';
COLON: ':';
QUESTION: '?'; // Added for ternary operator
ASSIGN: '='; // Used for default values
COMMA: ',';
DOT: '.';
ARROW: '=>'; // For get prop
DBL_COLON:
	'::'; // For static access like MasterVolume::setVolume
MINUS:
	'-'; // For style identifiers like background-color, also used in AddSubExpr

// Arithmetic Operators
ADD: '+';
// SUB removed, use MINUS
MUL: '*';
DIV: '/';

// Logical Operators
AND: '&&';
OR: '||';
NOT: '!';

// Comparison Operators
EQ: '==';
NEQ: '!=';
LT: '<';
LTE: '<=';
GT: '>';
GTE: '>=';

// Comments and Whitespace - skip them
WS: [ \t\r\n]+ -> skip;
LINE_COMMENT: '//' ~[\r\n]* -> skip;
BLOCK_COMMENT: '/*' .*? '*/' -> skip; // Non-greedy block comment

// Error handling (optional, basic example) UNMATCHED: . ; // Catch any other character - might be
// too broad