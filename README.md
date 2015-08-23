Program: Compiler for a C like Langauge
Author: Anirudh Ravi

About:
This Compiler program is written in C and contains logic to tokenize (lexer - generates Symbol Tree), parse (parser - does Syntax based Error Detection and generates Parse Tree) and generate 3 address code for a given subset of C program

Executing:
cc Compiler.c (Ensure Program.c - the input file is in the same directory)

Grammar for the langauge:
program -> header function | function
header 	-> "#include<" ID ".h>" header | ε 
function-> "int main()" block
block	-> '{' decls stmts '}'
decls	-> decls decl | ε
decl 	-> type id_list ';'
id_list	-> ID | id_list, ID
type 	-> type '[' NUM ']' | char | int 
stmts	-> stmts stmt | ε 
stmt	-> "for(" assign_stmt ';' relstmt ';' assignstmt ')' stmt
	| "while(" rel_stmt ')' stmt
	| "if(" rel_stmt ')' stmt
	| "if(" rel_stmt ')' stmt "else" stmt
	| assign_stmt ';'
	| block
assign_stmt-> expr = expr
rel_stmt-> expr < expr | expr <= expr | expr >= expr | expr > expr | expr
expr 	-> expr + expr | expr - expr | expr * expr | expr / expr | expr % expr | NUM
