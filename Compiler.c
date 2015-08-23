#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<stdlib.h>

#define KEYWORD  "Keyword"
#define ID 	 "Identifier"
#define NLITERAL "Number"
#define SLITERAL "String"
#define SS 	 "SpecialSymbol"
#define ROP 	 "RelationalOp"

#define FLOOP_S "for_loop_start"
#define FLOOP_E "for_loop_end"
#define WLOOP_S "while_loop_start"
#define WLOOP_E "while_loop_end"
#define IF_S "if_start"
#define IF_E "if_end"

#define SEMICOLON ';'
#define RIGHTBRACKET ')'
#define LEFTBRACE '{'
#define RIGHTBRACE '}'

#define KEYLEN 13
#define SYMLEN 24
#define SYMLEN2 4

#define SOURCE 		"Program.c"		//Source File: Program that is to be parsed
#define OUTPUT 		"SymbolTable.txt"	//Output: Symbol Table, Errors and Parse Tree
#define INTERMEDIATE 	"3AddressCode.asm"	//3 address code generated for the program

//Global Variables
char keywords[][10] = {"while", "if", "do", "for", "int", "char", "float", "double", "include", "switch", "case", "return", "main"};
char symbols[] = {'<', '>', '=', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', '=', '+', '{', '}', '.', '[', ']', ';', ',', '/'};
char symbols2[][3] = {"<=", ">=", "==", "!="};
char declarations[20][15];
int declarationCounter = 0;
int loopNumber = 0;
int ifNumber = 0;
int braceCount;
int globalLine;
int countError;
int temporaryNo = 0;

//Token structure to keep track of tokens in the program
struct token
{
    	char name[20];
    	char tokenType[20];
    	char strVal[20];
    	int numVal;
    	int lineNum;
    	struct token *lPtr;
    	struct token *rPtr;
} *localPtr;
struct token *current, *start = NULL, *temp;

struct parsetree
{
	char value[40];
	struct parsetree *rPtr;
	struct parsetree *lPtr;
} *pt, *str = NULL, *t, *crt;

//Stack for If-Else statements
struct stackIfElse
{
    int bracketNum[100];
    int ifNum;                   //Position gives the count of IF of that bracket
} ifElseStack;

//Function Definitions
void lexer(); void parser(); void codegenerator();
void ifElsepush(int); int ifElsepop(); int ifElsecurrentVal(); int doesIfExistForThisElse();
int mygetc(FILE *);
int isNum(char); int isOp(char); int isOperator(int); int isRelOperatorType1(int); 
int isAlreadyDeclared();
struct token* createList(struct token *, char *, char *, int, char *);
struct parsetree* addRule(struct parsetree *,char *);     
void getToken();
int isMatching(char *,int); int isMatchingType(char *, int); int isMatchingType2(struct token *, char *);
int checkExp(int, int); int checkExpWithoutEQUALTO(int); int checkExpNoAssh(int); int checkRelExp(int); 
int checkOpeningBrace(int); int checkClosingBrace(int);
int checkWhile(int); int checkFor(int);
int checkIf(int); int checkElse(int);
void printError();
int isIdentifierWithArraySupport(int, int);
void printStatement(char, char *,char *);
void printMove(struct token *, char *, int);
void writeRelExp(int, char *, int);
int writeAssiExp(char, struct token *, int);
int writeFor(int);
int writeWhile(int);
int writeIf(int);

//Driver function to run the lexer and parser
int main()
{  
	freopen(OUTPUT, "w", stdout);	//Redirecting output to SymbolTable.txt
	lexer();			//Stores all the tokens of the program in the form of a linked list
    	parser();			//Uses token linked list to find errors in the program
	codegenerator();
  
    	return 0;
}
			//LEXER
//Lexer to read from the source program and store tokens in the form of a linked list
void lexer()
{
	start = createList(start, "NAME", "TOKENTYPE", 0, "STRINGVAL");
    	current = start;
    	int k, i, num;
    	char c, need;
    	int gotKeyword = 0; //Flag to set if keyword found or not
	int got2SymOp; 	//Flag for Relation Operators
	char buffer[100];
    	globalLine = 1;

	FILE *fileToBeOpened;
	
    	fileToBeOpened = fopen(SOURCE,"r"); //Open the c program to parse

    	do
    	{
        	c = mygetc(fileToBeOpened);

		//String Detection
        	if(c == '"')
        	{
            		k=0;
            		c = mygetc(fileToBeOpened); //Get next character and increment if character = newline
            		for( ; ; )
            		{
                		if(c == '"')
                    			break;
                		buffer[k++] = c;
                		c = mygetc(fileToBeOpened);
            		}
            		buffer[k] = '\0';
            		current = createList(current," ",SLITERAL,0,buffer);
        	}
        	
		//Keyword and Identifier Detection
		if(isalpha(c))
        	{
            		k = 0;
            		gotKeyword = 0;
            		buffer[k++] = c;
            		for( ; ; )
            		{
                		c = mygetc(fileToBeOpened);
                		if(!isalpha(c))
                    			break;
                		buffer[k++] = c;
            		}
            		buffer[k]='\0';
            		for(i = 0; i < KEYLEN; i++)
            		{
                		if(!strcmp(keywords[i],buffer))
                		{
					//Save as keyword in token linked list
                    			current = createList(current,buffer,KEYWORD,0," ");
                    			gotKeyword = 1;
                    			break;
                		}
            		}
			//Save as identifier in token linked list
            		if(!gotKeyword)
            			current = createList(current,buffer,ID,0," ");
        	}
        
		//Numerals Detection
        	num = 0;
        	if(isNum(c))
        	{
            		num = c - '0';
            		for( ; ; )
            		{
                		c = mygetc(fileToBeOpened);
                		if(!isNum(c))
                    			break;
                		num *= 10;
                		num = num + (c -'0');
            		}
			//Add number with value to token linked list
            		current = createList(current," ",NLITERAL,num," ");
        	}
		
		//Operators - Relational, or Special - Detected
               	if(isOp(c))
        	{
            		k = 0;
            		got2SymOp = 0;
            		buffer[k++] = c;
            		c = mygetc(fileToBeOpened);
            		if(isOp(c))
            		{
                		buffer[k++] = c;
                		buffer[k] = '\0';
                		for(i = 0; i < SYMLEN2; i++)
                			if(!strcmp(symbols2[i],buffer))
                    			{
						//Add relational operator to token linked list
                        			current = createList(current,buffer,ROP,0," ");
                        			got2SymOp = 1;
                        			break;
                    			}
                	}
            		else
            		{
                		if(c == '\n')
                    			globalLine--;
                		fseek(fileToBeOpened,-1,SEEK_CUR);
            		}

            		if(!got2SymOp)
            		{
		                need = buffer[1];
                		buffer[1] = '\0';
                		current = createList(current,buffer,SS,0," ");
                		if(k != 1)
                		{
                    			buffer[0] = need;
                    			buffer[1] = '\0';
                    			current = createList(current,buffer,SS,0," ");
                		}
            		}
        	}
    	}while(c != EOF);

	//Print out the Symbol Table using the token linked list
	printf("\t \t \t SYMBOL TABLE\n");
	printf("\t \t \t ____________ \n \n"); 
	printf("\nLexeme \t \t Type \t \t Num Value \t Str Value \t Line");
	printf("\n______ \t \t ____ \t \t _________ \t _________ \t ____");
    	for(temp = start->rPtr; temp!=NULL; temp = temp->rPtr)
		printf("\n%s \t \t %s \t %d \t \t %s \t \t %d", temp->name, temp->tokenType, temp->numVal, temp->strVal, temp->lineNum);
	printf("\n\n\n\n");
}

			//PARSER
//Reads tokens from the token linked list and finds errors using Grammar Rules
void parser()
{
	localPtr  = start;
    	countError = 0;
	str = addRule(str, "RULE");
	crt = str;

	/* Grammar for the language:
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
	*/
	
	printf("\n\n\t \t \t ERROR DETECTION\n");
	printf("\t \t \t _______________ \n \n");

    	crt = addRule(crt, "\n <PROGRAM>");
	//Checking syntax of Header Files
    	while(isMatching("#", 1))
    	{
        	if(!isMatching("include", 1))	//Check for #include
            		printError();
		else
			crt = addRule(crt, "\n\t <HEADER>");
        	if(!isMatching("<", 1))		//Check for #include <
            		printError();
		else
			crt = addRule(crt, "\n\t\t <tag '#include <' >");
        	getToken();   			// identifier ex: stdlib
        	getToken();   			// Special Symbol ex : .
        	getToken();   			// identifier ex : h
		if(!isMatching(">", 1))		//Check for #include <identifier.h>
            		printError();
		else
			crt = addRule(crt, "\n \t\t\t <tag 'ID >' >"); 
    	}
		
	//Checking for int  or void main() {
        if(!isMatching("int", 0) && !isMatching("void", 0))
        	printError();
    	if(!isMatching("main", 1))
        	printError();
	else
		crt = addRule(crt, "\n\t <FUNCTION>");
    	if(!isMatching("(", 1))
        	printError();
    	if(!isMatching(")", 1))
        	printError();
	else
		crt = addRule(crt, "\n\t\t <tag 'int/void main()'>");
	if(!checkOpeningBrace(1))
		printError();
	else
	{
		crt = addRule(crt, "\n\t\t <BLOCK>");
		crt = addRule(crt, "\n\t\t <tag '{'>");
	}

    	//Checking For Declarations -- Next lookForNextToken shld be 0
    	while(isMatching("int", 1) || isMatching("char", 0))
    	{
		crt = addRule(crt, "\n\t\t\t <DECLS>");
		crt = addRule(crt, "\n\t\t\t\t <DECL>");
		crt = addRule(crt, "\n\t\t\t\t\t <TYPE>");
		crt = addRule(crt, "\n\t\t\t\t\t\t <tag KEYWORD = 'int/char'>");
		crt = addRule(crt, "\n\t\t\t\t\t\t <ID_LIST>");
		while(isIdentifierWithArraySupport(1, 1))
        	{
            		if(!isMatching(",", 1)  && !isMatching(";", 0))
            			printError();
			if(isMatching(";", 0))
			{
				crt = addRule(crt, "\n\t\t\t\t\t\t\t <tag 'ID'>");
				crt = addRule(crt, "\n\t\t\t\t\t\t\t <tag ';'>");
            			break;
			}
			else
			{
				crt = addRule(crt, "\n\t\t\t\t\t\t\t <tag 'ID'>");
			}				
        	}
    	}

    	//Using Doubly Link-List Finally!
    	temp = localPtr;
    	localPtr = localPtr->lPtr;
	crt = addRule(crt, "\n\t\t\t <STMTS>");
    	while(localPtr->rPtr != NULL)
    	{
        	if(checkFor(1)) 
		{
			crt = addRule(crt, "\n\t\t\t\t <STMT>");
			crt = addRule(crt, "\n\t\t\t\t\t <tag KEYWORD = 'for'>");
			crt = addRule(crt, "\n\t\t\t\t\t <tag '('>");
			crt = addRule(crt, "\n\t\t\t\t\t\t <ASSIGN_STMT>");
			crt = addRule(crt, "\n\t\t\t\t\t\t\t <tag ';'>");	
			crt = addRule(crt, "\n\t\t\t\t\t\t <REL_STMT>");
			crt = addRule(crt, "\n\t\t\t\t\t\t\t <tag ';'>");	
			crt = addRule(crt, "\n\t\t\t\t\t\t <ASSIGN_STMT>");
			crt = addRule(crt, "\n\t\t\t\t\t <tag ')'>");	
			crt = addRule(crt, "\n\t\t\t\t <BLOCK>");
			crt = addRule(crt, "\n\t\t\t\t <tag '{'>");			
		}
        	else if(checkClosingBrace(0)) 
		{
			crt = addRule(crt, "\n\t\t\t\t <tag '}'>");		
		}
        	else if(checkWhile(0)) 
		{
			crt = addRule(crt, "\n\t\t\t\t <STMT>");
			crt = addRule(crt, "\n\t\t\t\t\t <tag KEYWORD = 'while'>");
			crt = addRule(crt, "\n\t\t\t\t\t <tag '('>");
			crt = addRule(crt, "\n\t\t\t\t\t\t <REL_STMT>");
			crt = addRule(crt, "\n\t\t\t\t\t <tag ')'>");	
			crt = addRule(crt, "\n\t\t\t\t <BLOCK>");
			crt = addRule(crt, "\n\t\t\t\t <tag '{'>");		
		}
        	else if(checkIf(0)) 
		{
			crt = addRule(crt, "\n\t\t\t\t <STMT>");
			crt = addRule(crt, "\n\t\t\t\t\t <tag KEYWORD = 'if'>");
			crt = addRule(crt, "\n\t\t\t\t\t <tag '('>");
			crt = addRule(crt, "\n\t\t\t\t\t\t <REL_STMT>");
			crt = addRule(crt, "\n\t\t\t\t\t <tag ')'>");	
			crt = addRule(crt, "\n\t\t\t\t <BLOCK>");
			crt = addRule(crt, "\n\t\t\t\t <tag '{'>");	
		}
        	else if(checkElse(0)) 
		{
			crt = addRule(crt, "\n\t\t\t\t <STMT>");
			crt = addRule(crt, "\n\t\t\t\t\t <tag KEYWORD = 'else'>");
			crt = addRule(crt, "\n\t\t\t\t <BLOCK>");
			crt = addRule(crt, "\n\t\t\t\t <tag '{'>");	
		}
        	else if(checkExp(0, 1)) 
		{
			crt = addRule(crt, "\n\t\t\t\t\t\t\t <EXPR>");
			crt = addRule(crt, "\n\t\t\t\t\t\t\t\t <tag '='>");
			crt = addRule(crt, "\n\t\t\t\t\t\t\t <tag ';'>");
		}
        	else printError();
    	}
    
	if(braceCount)
        	printError();
    	printf("\n\n%d Errors are present",countError);
	//Print out the Symbol Table using the token linked list
	printf("\n\n\t \t \t PARSE TREE\n");
	printf("\t \t \t __________\n \n"); 
	for(t = str->rPtr; t!=NULL; t = t->rPtr)
		printf("%s", t->value);
}

			//3 ADDRESS CODE GENERATOR
void codegenerator()
{
	freopen(INTERMEDIATE, "w", stdout);
    	int i = 0;
    	
	printf("\t \t \t 3 ADDRESS CODE \n");
	printf("\t \t \t ______________ \n \n");
	//Declarations --- Currently declares Integers Inititated to 0
    	for(i=0;i<declarationCounter;i++)
        	printf("%s = 0;\n",declarations[i]);
	printf("\n");

    	//Making the local Pointer Point to token after int main(){ --
    	localPtr = temp;
    	localPtr = localPtr->lPtr;

    	while(localPtr->rPtr!=NULL)
    	{
        	if(writeFor(1)){printf("\n");}
        	else if(writeWhile(0)){printf("\n");}
        	else if(writeIf(0)){printf("\n");}
        	if(writeAssiExp(SEMICOLON,localPtr,0)){printf("\n");}
    	}
}

void ifElsepush(int x)
{
        ifElseStack.bracketNum[ifElseStack.ifNum++] = x;
}

int ifElsepop()                  
{
        if(ifElseStack.ifNum)
            	return ifElseStack.bracketNum[--ifElseStack.ifNum];
        else
            	return -1;
}

int ifElsecurrentVal()
{
        if(ifElseStack.ifNum > -1)
            	return ifElseStack.bracketNum[ifElseStack.ifNum];
        else
            	return -1;
}

int doesIfExistForThisElse()
{
    	while(ifElsepop() != braceCount){}
    	if(ifElsecurrentVal() == -1)
        	return 0;
    	return 1;
}

int mygetc(FILE *stream)
{
    	int x=getc(stream);
    	if(x == '\n')
        	globalLine++;
    	return x;
}

int isNum(char c)
{
    	if(c >= '0' && c <= '9')
        	return 1;
    	return 0;
}

int isOp(char c)   //For Use in Scanner
{
    	int i;
    	for(i = 0; i < SYMLEN; i++)
    	if(!(c-symbols[i]))
        	return 1;
    	return 0;
}

int isOperator(int lookForNextToken)   //For Use in Parser
{
	if(lookForNextToken)
		getToken();

	switch(localPtr->name[0])
	{
		case '+':
		case '-':
		case '%':
		case '/':
		case '*':return 1;
	}
	return 0;
}

int isRelOperatorType1(int lookForNextToken)   //For Use in Parser -- Single Symbol Operators
{
	if(lookForNextToken)
		getToken();

	switch(localPtr->name[0])
	{
		case '<':
		case '>':return 1;
	}
	return 0;
}

//Assumes check is on current position of localPtr
int isAlreadyDeclared()
{
    	int i;
    	for(i = 0; i < declarationCounter; i++)
        	if(isMatching(declarations[i], 0))
            		return 1;
    	return 0;
}

struct token* createList(struct token *crt, char *name, char *tokenType, int num, char *str)
{
    	struct token *tmp = (struct token *) malloc(sizeof(struct token));
    	strcpy(tmp->name,name);
    	strcpy(tmp->tokenType,tokenType);
    	strcpy(tmp->strVal,str);
    	tmp->lineNum = globalLine;
    	tmp->numVal = num;
    	tmp->rPtr = NULL;
    	if(crt == NULL)
        	return tmp;
    	tmp->lPtr = crt;
    	crt->rPtr = tmp;
	
	return tmp;
}

struct parsetree* addRule(struct parsetree *ct, char *value)
{
    	struct parsetree *tp = (struct parsetree *) malloc(sizeof(struct parsetree));
    	strcpy(tp->value, value);
    	
    	tp->rPtr = NULL;
    	if(crt == NULL)
        	return tp;
    	tp->lPtr = ct;
    	ct->rPtr = tp;
		return tp;
}

void getToken()
{
	localPtr = localPtr->rPtr;
}

//Automatically matches string with the name of the next token
int isMatching(char *str,int lookForNextToken)            
{
	if(lookForNextToken)
		getToken();

	if(strcmp(str,localPtr->name) == 0)
		return 1;
	return 0;
}

int isMatchingType(char *str,int lookForNextToken)
{
	if(lookForNextToken)
		getToken();

	if(strcmp(str,localPtr->tokenType) == 0)
		return 1;
	return 0;
}

int isMatchingType2(struct token *ptr,char *str)
{
	if(strcmp(str,ptr->tokenType) == 0)
		return 1;
	return 0;
}

//Checking for Assignment statement. Not checking for brackets
int checkExp(int lookForNextToken, int matchSemiColonPlz)
{
	if(!isIdentifierWithArraySupport(lookForNextToken, 0))
		return 0;
	if(!isMatching("=", 1))
		return 0;
	for( ; ; )
	{
		if(!isIdentifierWithArraySupport(1, 0) && !isMatchingType(NLITERAL, 0))
			return 0;
		if(!isOperator(1))
			break;
	}
	if(matchSemiColonPlz)
		if(!isMatching(";", 0))
		{
			localPtr = localPtr->lPtr;
			return 0;
	    	}
	return 1;
}

//Check for Expressions with no equalto, and no relational operators
//next lookForNextToken should be 0
int checkExpWithoutEQUALTO(int lookForNextToken)
{
    	for( ; ; )
	{
		if(!isIdentifierWithArraySupport(lookForNextToken,0) && !isMatchingType(NLITERAL,0))
			return 0;
		if(!isOperator(1))
			break;
		lookForNextToken = 1;
	}
	return 1;
}

//Check for expression which are not assign. Expressions but may be relational expression like 199+b%3-d<=f+76-h , or ,  a+b%c-d
//Next lookForNextToken should be 0
int checkExpNoAssh(int lookForNextToken)
{
    	if(!checkExpWithoutEQUALTO(lookForNextToken))
		return 0;
    	if(isMatchingType(ROP, 0) || isRelOperatorType1(0))
    	{
		if(!checkExpWithoutEQUALTO(1))
	    		return 0;
    	}
    	return 1;
}

//Checks For Simple  Relation Expressions with any number of tokens Like a+b%c-d<=f+g-h
//Not checking for brackets though
int checkRelExp(int lookForNextToken)
{
	if(!checkExpWithoutEQUALTO(lookForNextToken))
		return 0;
	if(!isMatchingType(ROP, 0) && !isRelOperatorType1(0))
		return 0;
	if(!checkExpWithoutEQUALTO(1))
		return 0;
	return 1;
}

int checkOpeningBrace(int lookForNextToken)
{
   	if(!isMatching("{",lookForNextToken))
		return 0;
    	braceCount++;
    	return  1;
}

int checkClosingBrace(int lookForNextToken)
{
    	if(braceCount == 0)
    	{
		printError();
		return 0;
    	}
    	if(!isMatching("}",lookForNextToken))
		return 0;
    	braceCount--;
    	return  1;
}

int checkFor(int lookForNextToken)
{
	if(!isMatching("for", lookForNextToken))
		return 0;
	if(!isMatching("(", 1))
		return 0;
	if(!checkExp(1, 1))
		return 0;
	if(!checkRelExp(1))
		return 0;
	if(!isMatching(";", 0))
		return 0;
	if(!checkExp(1, 0))
		return 0;
	if(!isMatching(")", 0))
		return 0;
	if(!checkOpeningBrace(1))
		return 0;
    	return 1;
}

int checkWhile(int lookForNextToken)
{
	if(!isMatching("while", lookForNextToken))
		return 0;
    	if(!isMatching("(", 1))
		return 0;
    	if(!checkExpNoAssh(1))
        	return 0;
    	if(!isMatching(")", 0))
        	return 0;
	if(!checkOpeningBrace(1))
		return 0;
    	return 1;
}

int checkIf(int lookForNextToken)
{
    	if(!isMatching("if", lookForNextToken))
		return 0;
    	if(!isMatching("(", 1))
		return 0;
    	if(!checkExpNoAssh(1))
        	return 0;
    	if(!isMatching(")", 0))
        	return 0;
    	ifElsepush(braceCount);
	if(!checkOpeningBrace(1))
		return 0;
        return 1;
}

int checkElse(int lookForNextToken)
{

    	if(!isMatching("else", lookForNextToken))
		return 0;
    	if(!doesIfExistForThisElse())
        	return 0;
    	if(!checkOpeningBrace(1))
        	return 0;
    	return 1;
}

void printError() //Prints error corresponding to the current value of localPtr
{
	countError++;
	printf("\nError in Line Number %d: %s",localPtr->lineNum,localPtr->name);
}

//Checks Whether Token is identifier May be Array. Example---> i,arr[234]
int isIdentifierWithArraySupport(int lookForNextToken,int declareIdentifierPlz)
{
    	int i;
    	if(!isMatchingType(ID,lookForNextToken))
        	return 0;
    	if(declareIdentifierPlz)
	    	strcpy(declarations[declarationCounter++],localPtr->name);
    	if(!isAlreadyDeclared())
    	{
        	printf("\n%s not declared.",localPtr->name);
        	return 0;
    	}
    	if(!isMatching("[",1))
    	{
        	localPtr = localPtr->lPtr;
        	return 1;
    	}
    	if(!isMatchingType(NLITERAL,1))
        	return 0;
    	if(!isMatching("]",1))
        	return 0;
    	return 1;
}

			//CODE GENERATION FUNCTIONS//

void printStatement(char operation,char *temp1,char *temp2)
{
    	switch(operation)
    	{
        	case '+':printf("\nt%d = %s%d + %s%d",temporaryNo++, temp1, temporaryNo-2, temp2, temporaryNo-1); return;
        	case '-':printf("\nt%d = %s%d - %s%d",temporaryNo++, temp1, temporaryNo-2, temp2, temporaryNo-1); return;
        	case '*':printf("\nt%d = %s%d * %s%d",temporaryNo++, temp1, temporaryNo-2, temp2, temporaryNo-1); return;
        	case '/':printf("\nt%d = %s%d / %s%d",temporaryNo++, temp1, temporaryNo-2, temp2, temporaryNo-1); return;
    	}
}

void printMove(struct token *ptr,char *tempName,int reverse)
{
    	if(reverse)
        	if(strcmp(ptr->tokenType,ID)) // IF tokenType is not ID ...... it gives true
            		printf("\n%d = %s%d",ptr->numVal, tempName, temporaryNo);
        	else
            		printf("\n%s = %s%d",ptr->name, tempName, temporaryNo-1);
    	else
        	if(strcmp(ptr->tokenType,ID)) // IF tokenType is not ID ...... it gives true
            		printf("\n%s%d = %d",tempName, temporaryNo++, ptr->numVal);
        	else
            		printf("\n%s%d = %s",tempName, temporaryNo++, ptr->name);
}

//Works only for 3 token relative expressions.... ex 2<3 , a<=5 , i>=j
void writeRelExp(int lookForNextToken,char *lableString,int num)
{
    	char operation[3];
    	if(lookForNextToken)        //Expecting ID or Num
        	getToken();
    	printMove(localPtr,"t",0);
    	getToken();                 //Expecting Relative Operators
    	strcpy(operation,localPtr->name);
    	getToken();                 //Expecting ID or Num
    	printMove(localPtr,"t",0);
    	printf("\nCMP t%d, t%d", temporaryNo-2, temporaryNo-1);
    	if(strcmp(operation,"<=") == 0)
    	{
        	printf("\nJG %s%d",lableString,num);
        	return;
    	}
    	if(strcmp(operation,">=") == 0)
    	{
        	printf("\nJL %s%d",lableString,num);
        	return;
    	}
    	if(strcmp(operation,"==") == 0)
    	{
        	printf("\nJNE %s%d",lableString,num);
        	return;
    	}
    	if(strcmp(operation,"!=") == 0)
    	{
        	printf("\nJE %s%d",lableString,num);
        	return;
   	 }
    	switch(operation[0])
    	{
       	 	case '<':
            		printf("\nJGE %s%d",lableString,num);
            		return;
        	case '>':
            		printf("\nJLE %s%d",lableString,num);
            		return;
    	}
}

// Writes any assignment expression ... does not check precedence.
// If pointer passed is global ..... it will change the global automatically
// If pointer passed isnt global ... lookAheadForNextToken doesnt matter
int writeAssiExp(char endChar, struct token *curPtr, int lookAheadForNextToken)
{
    	int globalPassed = 0;
    	if(localPtr == curPtr)
    	{
        	if(lookAheadForNextToken)
        	{
            		getToken();
            		curPtr = curPtr->rPtr;
        	}
        	globalPassed = 1;
    	}

    	if(!isMatchingType2(curPtr, ID))
        	return 0;
	
	struct token *temp;
	temp = curPtr;
	curPtr = curPtr->rPtr;         //Expecting '='
	curPtr = curPtr->rPtr;         //Expecting ID or Num
	printMove(curPtr,"t",0);
	curPtr = curPtr->rPtr;         //Expecting a operator or endChar
    
	while(curPtr->name[0]!=endChar)
    	{
        	curPtr = curPtr->rPtr;      //Expecting Id or Num
        	printMove(curPtr,"t",0);
        	printStatement(curPtr->lPtr->name[0],"t","t");
		curPtr = curPtr->rPtr;      //Expecting Operator or endChar
    	}
    	
	printMove(temp, "t", 1);
    	if(globalPassed)
        	localPtr = curPtr;
    	
	return 1;
}

int writeFor(int lookForNextToken)
{
    	struct token *temp;
    	if(!isMatching("for",lookForNextToken))
        	return 0;
    	getToken();                      //expecting '('
    	writeAssiExp(SEMICOLON,localPtr,1);
    	printf("\n%s%d:", FLOOP_S, ++loopNumber);
    	writeRelExp(1, FLOOP_E, loopNumber);                  //expecting localPtr as semicolon
    	getToken();                      //expecting semicolon
    	getToken();                      //expecting ID

    	temp = localPtr;                  // Any Assignment Expression
    	while(localPtr->name[0]!=LEFTBRACE)
        	getToken();
    	getToken();
    	while(localPtr->name[0]!=RIGHTBRACE)
    	{
       	 	writeAssiExp(SEMICOLON,localPtr,0);
        	getToken();   //Writing commands inside for loop
   	 }
    	writeAssiExp(RIGHTBRACKET,temp,0);        //writing increment,decrement etc
    	printf("\nJMP %s%d", FLOOP_S, loopNumber);
    	printf("\n%s%d:", FLOOP_E, loopNumber);
    
	return 1;
}

int writeWhile(int lookForNextToken)
{
    	if(!isMatching("while",lookForNextToken))
        	return 0;
    	printf("\n%s%d:",WLOOP_S,++loopNumber);
    	getToken();         //Expecting (
    	writeRelExp(1,WLOOP_E,loopNumber);
    	getToken();         // Expecting )
    	getToken();         // Expecting {
    	while(localPtr->name[0]!=RIGHTBRACE)
    	{
        	writeAssiExp(SEMICOLON,localPtr,1);
    	}
    	printf("\nJMP %s%d",WLOOP_S,loopNumber);
    	printf("\n%s%d:",WLOOP_E,loopNumber);

    	return 1;
}

int writeIf(int lookForNextToken)
{
    	if(!isMatching("if",lookForNextToken))
        	return 0;
    	getToken();     //Expecting (
    	writeRelExp(1,IF_E,++ifNumber);
    	getToken();     //Expecting )
    	getToken();     //Expecting {
    	while(localPtr->name[0]!=RIGHTBRACE)
   	{
        	writeAssiExp(SEMICOLON,localPtr,1);
    	}
    	printf("\n%s%d:",IF_E,ifNumber);
    
	return 1;
}
