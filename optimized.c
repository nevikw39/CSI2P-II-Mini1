#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#pragma GCC optimize("Ofast,unroll-loops,no-stack-protector,fast-math")
#pragma GCC target("tune=native")

/*
  For the language grammar, please refer to Grammar section on the github page:
  https://github.com/lightbulb12294/CSI2P-II-Mini1#grammar
*/

#pragma region TEMPL_DEF

#define MAX_LENGTH 200

typedef enum
{
	ASSIGN,
	ADD,
	SUB,
	MUL,
	DIV,
	REM,
	PREINC,
	PREDEC,
	POSTINC,
	POSTDEC,
	IDENTIFIER,
	CONSTANT,
	LPAR,
	RPAR,
	PLUS,
	MINUS,
	END
} Kind;

typedef enum
{
	STMT,
	EXPR,
	ASSIGN_EXPR,
	ADD_EXPR,
	MUL_EXPR,
	UNARY_EXPR,
	POSTFIX_EXPR,
	PRI_EXPR
} GrammarState;

typedef struct TokenUnit
{
	Kind kind;
	int val; // record the integer value or variable name
	struct TokenUnit *next;
} Token;

typedef struct ASTUnit
{
	Kind kind;
	int val; // record the integer value or variable name
	struct ASTUnit *lhs, *mid, *rhs;
} AST;

/// utility interfaces

// err marco should be used when a expression error occurs.
#define err(x)                                                \
	{                                                         \
		puts("Compile Error!");                               \
		if (DEBUG)                                            \
		{                                                     \
			fprintf(stderr, "Error at line: %d\n", __LINE__); \
			fprintf(stderr, "Error message: %s\n", x);        \
		}                                                     \
		exit(0);                                              \
	}

// You may set DEBUG=1 to debug. Remember setting back to 0 before submit.
#ifndef DEBUG
#define DEBUG 0
#endif
// Split the input char array into token linked list.
Token *lexer(const char *in);
// Create a new token.
Token *new_token(Kind kind, int val);
// Translate a token linked list into array, return its length.
size_t token_list_to_arr(Token **head);
// Parse the token array. Return the constructed AST.
AST *parser(Token *arr, size_t len);
// Parse the token array. Return the constructed AST.
AST *parse(Token *arr, int l, int r, GrammarState S);
// Create a new AST node.
AST *new_AST(Kind kind, int val);
// Find the location of next token that fits the condition(cond). Return -1 if not found. Search direction from start to end.
int findNextSection(Token *arr, int start, int end, int (*cond)(Kind));
// Return 1 if kind is ASSIGN.
int condASSIGN(Kind kind);
// Return 1 if kind is ADD or SUB.
int condADD(Kind kind);
// Return 1 if kind is MUL, DIV, or REM.
int condMUL(Kind kind);
// Return 1 if kind is RPAR.
int condRPAR(Kind kind);
// Check if the AST is semantically right. This function will call err() automatically if check failed.
void semantic_check(AST *now);
// Generate ASM code.
void codegen(AST *root);
// Free the whole AST.
void freeAST(AST *now);

/// debug interfaces

// Print token array.
void token_print(Token *in, size_t len);
// Print AST tree.
void AST_print(AST *head);

#pragma endregion TEMPL_DEF

#pragma region NEVIKW39_DEF

// return whether an AST is a constant
int isConstAST(AST *);
// return the value of an constant AST
int getConstAST(AST *);
// return a new constant AST
AST *newConstAST(int);
// determine whether an AST is not needeed
int notNeeded(AST *);
// basic optimize
AST *optimizeAST(AST *);

typedef struct _Symbol // the operand of an ASM
{
	enum SymbolType
	{
		SYMB_CONST,
		SYMB_REG,
		SYMB_MEM,
		SYMB_NIL
	} type;
	int val;
} Symbol;

// print the symbol
void printSymbol(Symbol);

typedef enum _Op // the type of an ASM
{
	OP_LOAD,
	OP_STORE,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_REM
} Op;

typedef struct _ASM
{
	Op op_code;
	Symbol d, s1, s2;
	int dead, status, val;
} ASM;

ASM asms[MAX_LENGTH * 15 << 3], // the ASM queue
	*asms_end = asms;			// the open-interval end of the ASM queue

// add an ASM to the ASM queue
void addASM(Op, Symbol, Symbol, Symbol);
// generate ASMs from AST
Symbol genASM(AST *);
// print the ASM queue
void printASM(ASM);
// execute ASM in compile time to determine constant
void execASM(int arr[3]);

enum Reg // the status of the registers
{
	FREE,
	OCCUPIED,
	VAR
} regs[256];

// get a free register
int getReg();

int vars[3] = {-1, -1, -1}, // the register in which the variable stored
	modified[3] = {0};		// whether the variable modified

// optimize by excuting ASMs in Compile Time
void optimize();
// perform dead code elimination
void dead_code_eliminate(ASM *, int);

#pragma endregion NEVIKW39_DEF

char input[MAX_LENGTH];

int main()
{
	while (fgets(input, MAX_LENGTH, stdin) != NULL)
	{
		Token *content = lexer(input);
		size_t len = token_list_to_arr(&content);
		if (len == 0)
			continue;
		if (DEBUG)
			token_print(content, len);
		AST *ast_root = optimizeAST(parser(content, len));
		if (DEBUG)
			AST_print(ast_root);
		semantic_check(ast_root);
		codegen(ast_root);
		for (int i = 0; i < 256; i++)
			if (regs[i] == OCCUPIED)
				regs[i] = FREE;
		free(content);
		freeAST(ast_root);
	}
	optimize();
	for (int i = 0; i < 3; i++)
		if (modified[i])
			addASM(OP_STORE, (Symbol){SYMB_MEM, i << 2}, (Symbol){SYMB_REG, vars[i]}, (Symbol){SYMB_NIL, 0});
	for (ASM *ptr = asms; ptr != asms_end; ptr++)
		if (!ptr->dead || ptr->op_code == OP_STORE)
			printASM(*ptr);
	return 0;
}

#pragma region TEMPL_IMPL

Token *lexer(const char *in)
{
	Token *head = NULL;
	Token **now = &head;
	for (int i = 0; in[i]; i++)
	{
		if (isspace(in[i])) // ignore space characters
			continue;
		else if (isdigit(in[i]))
		{
			(*now) = new_token(CONSTANT, atoi(in + i));
			while (in[i + 1] && isdigit(in[i + 1]))
				i++;
		}
		else if ('x' <= in[i] && in[i] <= 'z') // variable
			(*now) = new_token(IDENTIFIER, in[i]);
		else
			switch (in[i])
			{
			case '=':
				(*now) = new_token(ASSIGN, 0);
				break;
			case '+':
				if (in[i + 1] && in[i + 1] == '+')
				{
					i++;
					// In lexer scope, all "++" will be labeled as PREINC.
					(*now) = new_token(PREINC, 0);
				}
				// In lexer scope, all single "+" will be labeled as PLUS.
				else
					(*now) = new_token(PLUS, 0);
				break;
			case '-':
				if (in[i + 1] && in[i + 1] == '-')
				{
					i++;
					// In lexer scope, all "--" will be labeled as PREDEC.
					(*now) = new_token(PREDEC, 0);
				}
				// In lexer scope, all single "-" will be labeled as MINUS.
				else
					(*now) = new_token(MINUS, 0);
				break;
			case '*':
				(*now) = new_token(MUL, 0);
				break;
			case '/':
				(*now) = new_token(DIV, 0);
				break;
			case '%':
				(*now) = new_token(REM, 0);
				break;
			case '(':
				(*now) = new_token(LPAR, 0);
				break;
			case ')':
				(*now) = new_token(RPAR, 0);
				break;
			case ';':
				(*now) = new_token(END, 0);
				break;
			default:
				err("Unexpected character.");
			}
		now = &((*now)->next);
	}
	return head;
}

Token *new_token(Kind kind, int val)
{
	Token *res = (Token *)malloc(sizeof(Token));
	res->kind = kind;
	res->val = val;
	res->next = NULL;
	return res;
}

size_t token_list_to_arr(Token **head)
{
	size_t res;
	Token *now = (*head), *del;
	for (res = 0; now != NULL; res++)
		now = now->next;
	now = (*head);
	if (res != 0)
		(*head) = (Token *)malloc(sizeof(Token) * res);
	for (int i = 0; i < res; i++)
	{
		(*head)[i] = (*now);
		del = now;
		now = now->next;
		free(del);
	}
	return res;
}

AST *parser(Token *arr, size_t len)
{
	for (int i = 1; i < len; i++)
	{
		// correctly identify "ADD" and "SUB"
		if (arr[i].kind == PLUS || arr[i].kind == MINUS)
		{
			switch (arr[i - 1].kind)
			{
			case PREINC:
			case PREDEC:
			case IDENTIFIER:
			case CONSTANT:
			case RPAR:
				arr[i].kind = arr[i].kind - PLUS + ADD;
			default:
				break;
			}
		}
	}
	return parse(arr, 0, len - 1, STMT);
}

AST *parse(Token *arr, int l, int r, GrammarState S)
{
	AST *now = NULL;
	if (l > r)
		err("Unexpected parsing range.");
	int nxt;
	switch (S)
	{
	case STMT:
		if (l == r && arr[l].kind == END)
			return NULL;
		else if (arr[r].kind == END)
			return parse(arr, l, r - 1, EXPR);
		else
			err("Expected \';\' at the end of line.");
	case EXPR:
		return parse(arr, l, r, ASSIGN_EXPR);
	case ASSIGN_EXPR:
		if ((nxt = findNextSection(arr, l, r, condASSIGN)) != -1)
		{
			now = new_AST(arr[nxt].kind, 0);
			now->lhs = parse(arr, l, nxt - 1, UNARY_EXPR);
			now->rhs = parse(arr, nxt + 1, r, ASSIGN_EXPR);
			return now;
		}
		return parse(arr, l, r, ADD_EXPR);
	case ADD_EXPR:
		if ((nxt = findNextSection(arr, r, l, condADD)) != -1)
		{
			now = new_AST(arr[nxt].kind, 0);
			now->lhs = parse(arr, l, nxt - 1, ADD_EXPR);
			now->rhs = parse(arr, nxt + 1, r, MUL_EXPR);
			return now;
		}
		return parse(arr, l, r, MUL_EXPR);
	case MUL_EXPR:
		// TODO: Implement MUL_EXPR.
		// hint: Take ADD_EXPR as reference.
		if (~(nxt = findNextSection(arr, r, l, condMUL)))
		{
			now = new_AST(arr[nxt].kind, 0);
			now->lhs = parse(arr, l, nxt - 1, MUL_EXPR);
			now->rhs = parse(arr, nxt + 1, r, UNARY_EXPR);
			return now;
		}
		return parse(arr, l, r, UNARY_EXPR);
	case UNARY_EXPR:
		// TODO: Implement UNARY_EXPR.
		// hint: Take POSTFIX_EXPR as reference.
		if (arr[l].kind == PREINC || arr[l].kind == PREDEC || arr[l].kind == PLUS || arr[l].kind == MINUS)
		{
			now = new_AST(arr[l].kind, 0);
			now->mid = parse(arr, l + 1, r, UNARY_EXPR);
			return now;
		}
		return parse(arr, l, r, POSTFIX_EXPR);
	case POSTFIX_EXPR:
		if (arr[r].kind == PREINC || arr[r].kind == PREDEC)
		{
			// translate "PREINC", "PREDEC" into "POSTINC", "POSTDEC"
			now = new_AST(arr[r].kind - PREINC + POSTINC, 0);
			now->mid = parse(arr, l, r - 1, POSTFIX_EXPR);
			return now;
		}
		return parse(arr, l, r, PRI_EXPR);
	case PRI_EXPR:
		if (findNextSection(arr, l, r, condRPAR) == r)
		{
			now = new_AST(LPAR, 0);
			now->mid = parse(arr, l + 1, r - 1, EXPR);
			return now;
		}
		if (l == r)
		{
			if (arr[l].kind == IDENTIFIER || arr[l].kind == CONSTANT)
				return new_AST(arr[l].kind, arr[l].val);
			err("Unexpected token during parsing.");
		}
		err("No token left for parsing.");
	default:
		err("Unexpected grammar state.");
	}
}

AST *new_AST(Kind kind, int val)
{
	AST *res = (AST *)malloc(sizeof(AST));
	res->kind = kind;
	res->val = val;
	res->lhs = res->mid = res->rhs = NULL;
	return res;
}

int findNextSection(Token *arr, int start, int end, int (*cond)(Kind))
{
	int par = 0;
	int d = (start < end) ? 1 : -1;
	for (int i = start; (start < end) ? (i <= end) : (i >= end); i += d)
	{
		if (arr[i].kind == LPAR)
			par++;
		if (arr[i].kind == RPAR)
			par--;
		if (par == 0 && cond(arr[i].kind) == 1)
			return i;
	}
	return -1;
}

int condASSIGN(Kind kind) { return kind == ASSIGN; }

int condADD(Kind kind) { return kind == ADD || kind == SUB; }

int condMUL(Kind kind) { return kind == MUL || kind == DIV || kind == REM; }

int condRPAR(Kind kind) { return kind == RPAR; }

void semantic_check(AST *now)
{
	if (now == NULL)
		return;
	// Left operand of '=' must be an identifier or identifier with one or more parentheses.
	if (now->kind == ASSIGN)
	{
		AST *tmp = now->lhs;
		while (tmp->kind == LPAR)
			tmp = tmp->mid;
		if (tmp->kind != IDENTIFIER)
			err("Lvalue is required as left operand of assignment.");
	}
	// Operand of INC/DEC must be an identifier or identifier with one or more parentheses.
	// TODO: Implement the remaining semantic_check code.
	// hint: Follow the instruction above and ASSIGN-part code to implement.
	if (now->kind == PREINC || now->kind == POSTINC || now->kind == PREDEC || now->kind == POSTDEC)
	{
		AST *tmp = now->mid;
		while (tmp->kind == LPAR)
			tmp = tmp->mid;
		if (tmp->kind != IDENTIFIER)
			err("perand of INC/DEC must be an identifier or identifier with one or more parentheses.");
	}
	// hint: Semantic of each node needs to be checked recursively (from the current node to lhs/mid/rhs node).
	semantic_check(now->lhs);
	semantic_check(now->mid);
	semantic_check(now->rhs);
}

void codegen(AST *root)
{
	// TODO: Implement your codegen in your own way.
	// You may modify the function parameter or the return type, even the whole structure as you wish.
	if (!root)
		return;
	switch (root->kind)
	{
	case ASSIGN:
	case PREINC:
	case POSTINC:
	case PREDEC:
	case POSTDEC:
		genASM(root);
		return;
	}
	codegen(root->lhs);
	codegen(root->mid);
	codegen(root->rhs);
}

void freeAST(AST *now)
{
	if (now == NULL)
		return;
	freeAST(now->lhs);
	freeAST(now->mid);
	freeAST(now->rhs);
	free(now);
}

void token_print(Token *in, size_t len)
{
	const static char KindName[][20] = {
		"Assign", "Add", "Sub", "Mul", "Div", "Rem", "Inc", "Dec", "Inc", "Dec", "Identifier", "Constant", "LPar", "RPar", "Plus", "Minus", "End"};
	const static char KindSymbol[][20] = {
		"'='", "'+'", "'-'", "'*'", "'/'", "'%'", "\"++\"", "\"--\"", "\"++\"", "\"--\"", "", "", "'('", "')'", "'+'", "'-'"};
	const static char format_str[] = "<Index = %3d>: %-10s, %-6s = %s\n";
	const static char format_int[] = "<Index = %3d>: %-10s, %-6s = %d\n";
	for (int i = 0; i < len; i++)
	{
		switch (in[i].kind)
		{
		case LPAR:
		case RPAR:
		case PREINC:
		case PREDEC:
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case REM:
		case ASSIGN:
		case PLUS:
		case MINUS:
			fprintf(stderr, format_str, i, KindName[in[i].kind], "symbol", KindSymbol[in[i].kind]);
			break;
		case CONSTANT:
			fprintf(stderr, format_int, i, KindName[in[i].kind], "value", in[i].val);
			break;
		case IDENTIFIER:
			fprintf(stderr, format_str, i, KindName[in[i].kind], "name", (char *)(&(in[i].val)));
			break;
		case END:
			fprintf(stderr, "<Index = %3d>: %-10s\n", i, KindName[in[i].kind]);
			break;
		default:
			fputs("=== unknown token ===", stderr);
		}
	}
}

void AST_print(AST *head)
{
	static char indent_str[MAX_LENGTH] = "";
	static int indent = 0;
	char *indent_now = indent_str + indent;
	const static char KindName[][20] = {
		"Assign", "Add", "Sub", "Mul", "Div", "Rem", "PreInc", "PreDec", "PostInc", "PostDec", "Identifier", "Constant", "Parentheses", "Parentheses", "Plus", "Minus"};
	const static char format[] = "%s\n";
	const static char format_str[] = "%s, <%s = %s>\n";
	const static char format_val[] = "%s, <%s = %d>\n";
	if (head == NULL)
		return;
	indent_str[indent - 1] = '-';
	fprintf(stderr, "%s", indent_str);
	indent_str[indent - 1] = ' ';
	if (indent_str[indent - 2] == '`')
		indent_str[indent - 2] = ' ';
	switch (head->kind)
	{
	case ASSIGN:
	case ADD:
	case SUB:
	case MUL:
	case DIV:
	case REM:
	case PREINC:
	case PREDEC:
	case POSTINC:
	case POSTDEC:
	case LPAR:
	case RPAR:
	case PLUS:
	case MINUS:
		fprintf(stderr, format, KindName[head->kind]);
		break;
	case IDENTIFIER:
		fprintf(stderr, format_str, KindName[head->kind], "name", (char *)&(head->val));
		break;
	case CONSTANT:
		fprintf(stderr, format_val, KindName[head->kind], "value", head->val);
		break;
	default:
		fputs("=== unknown AST type ===", stderr);
	}
	indent += 2;
	strcpy(indent_now, "| ");
	AST_print(head->lhs);
	strcpy(indent_now, "` ");
	AST_print(head->mid);
	AST_print(head->rhs);
	indent -= 2;
	(*indent_now) = '\0';
}

#pragma endregion TEMPL_IMPL

#pragma region NEVIKW39_FUNC_IMPL

int isConstAST(AST *root)
{
	return root && (root->kind == CONSTANT || root->kind == MINUS && root->mid->kind == CONSTANT);
}

int getConstAST(AST *root)
{
	if (root->kind == CONSTANT)
		return root->val;
	return -root->mid->val;
}

AST *newConstAST(int val)
{
	if (val >= 0)
		return new_AST(CONSTANT, val);
	AST *root = new_AST(MINUS, 0);
	root->mid = new_AST(CONSTANT, -val);
	return root;
}

int notNeeded(AST *root)
{
	if (!root)
		return 1;
	switch (root->kind)
	{
	case ASSIGN:
	case PREINC:
	case POSTINC:
	case PREDEC:
	case POSTDEC:
		return 0;
	}
	return notNeeded(root->lhs) && notNeeded(root->mid) && notNeeded(root->rhs);
}

AST *optimizeAST(AST *root)
{
	if (!root)
		return NULL;
	root->lhs = optimizeAST(root->lhs);
	root->mid = optimizeAST(root->mid);
	root->rhs = optimizeAST(root->rhs);
	if (root->kind == PLUS || root->kind == LPAR)
	{
		AST *tmp = root;
		root = root->mid;
		free(tmp);
	}
	else if (root->kind == MINUS && root->mid->kind == MINUS)
	{
		AST *tmp = root;
		root = root->mid->mid;
		free(tmp);
	}
	else if (root->lhs && root->rhs && (root->lhs->kind == IDENTIFIER && root->rhs->kind == IDENTIFIER && root->lhs->val == root->rhs->val && (root->kind == SUB || root->kind == REM) || notNeeded(root) && (isConstAST(root->lhs) && !getConstAST(root->lhs) && condMUL(root->kind) || isConstAST(root->rhs) && (!getConstAST(root->rhs) && root->kind == MUL || getConstAST(root->rhs) == 1 && root->kind == REM))))
	{
		AST *tmp = root;
		root = newConstAST(0);
		freeAST(tmp);
	}
	else if (root->kind == DIV && root->lhs->kind == IDENTIFIER && root->rhs->kind == IDENTIFIER && root->lhs->val == root->rhs->val)
	{
		AST *tmp = root;
		root = newConstAST(1);
		freeAST(tmp);
	}
	else if (isConstAST(root->lhs))
	{
		if (isConstAST(root->rhs))
		{
			AST *tmp = root;
			switch (tmp->kind)
			{
			case ADD:
				root = newConstAST(getConstAST(tmp->lhs) + getConstAST(tmp->rhs));
				break;
			case SUB:
				root = newConstAST(getConstAST(tmp->lhs) - getConstAST(tmp->rhs));
				break;
			case MUL:
				root = newConstAST(getConstAST(tmp->lhs) * getConstAST(tmp->rhs));
				break;
			case DIV:
				root = newConstAST(getConstAST(tmp->lhs) / getConstAST(tmp->rhs));
				break;
			case REM:
				root = newConstAST(getConstAST(tmp->lhs) % getConstAST(tmp->rhs));
			}
			freeAST(tmp);
		}
		else if (root->kind == ADD && !getConstAST(root->lhs) || root->kind == MUL && getConstAST(root->lhs) == 1)
		{
			AST *tmp = root;
			root = root->rhs;
			freeAST(tmp->lhs);
			free(tmp);
		}
		else if (root->kind == SUB && !getConstAST(root->lhs) || root->kind == MUL && !~getConstAST(root->lhs))
		{
			AST *tmp = root;
			root = new_AST(MINUS, 0);
			root->mid = tmp->rhs;
			freeAST(tmp->lhs);
			free(tmp);
		}
	}
	else if (isConstAST(root->rhs))
	{
		if (condADD(root->kind) && !getConstAST(root->rhs) || root->kind == MUL && getConstAST(root->rhs) == 1)
		{
			AST *tmp = root;
			root = root->lhs;
			freeAST(tmp->rhs);
			free(tmp);
		}
		else if (root->kind == MUL && !~getConstAST(root->rhs))
		{
			AST *tmp = root;
			root = new_AST(MINUS, 0);
			root->mid = tmp->lhs;
			freeAST(tmp->rhs);
			free(tmp);
		}
	}
	return root;
}

void printSymbol(Symbol s)
{
	if (s.type == SYMB_NIL)
		return;
	const static char *format[] = {"%d", "r%d", "[%d]"};
	printf(format[s.type], s.val);
}

void addASM(Op op_code, Symbol d, Symbol s1, Symbol s2)
{
	asms_end->op_code = op_code;
	asms_end->d = d;
	asms_end->s1 = s1;
	asms_end->s2 = s2;
	asms_end->dead = 1;
	asms_end->status = -1;
	++asms_end;
}

int LOCK = 0;

Symbol genASM(AST *root)
{
	const static Symbol ZERO = (Symbol){SYMB_CONST, 0}, ONE = (Symbol){SYMB_CONST, 1}, NIL_SYMB = (Symbol){SYMB_NIL, 0};
	Symbol lhs, rhs, res;
	if (!root)
		return NIL_SYMB;
	switch (root->kind)
	{
	case ASSIGN:
		rhs = genASM(root->rhs);
		if (rhs.type == SYMB_CONST || regs[rhs.val] == VAR)
		{
			res = (Symbol){SYMB_REG, getReg()};
			addASM(OP_ADD, res, ZERO, rhs);
		}
		else
			res = rhs;
		LOCK = 1;
		lhs = genASM(root->lhs);
		LOCK = 0;
		if (lhs.type != SYMB_NIL)
			regs[lhs.val] = FREE;
		vars[root->lhs->val - 'x'] = res.val;
		modified[root->lhs->val - 'x'] = 1;
		regs[res.val] = VAR;
		return res;
	case ADD:
	case SUB:
	case MUL:
	case DIV:
	case REM:
		lhs = genASM(root->lhs);
		rhs = genASM(root->rhs);
		if (lhs.type == SYMB_REG && regs[lhs.val] != VAR)
			res = lhs;
		else if (rhs.type == SYMB_REG && regs[rhs.val] != VAR)
			res = rhs;
		else
			res = (Symbol){SYMB_REG, getReg()};
		addASM(root->kind - ADD + OP_ADD, res, lhs, rhs);
		if (lhs.type == SYMB_REG && regs[lhs.val] != VAR)
			regs[lhs.val] = FREE;
		if (rhs.type == SYMB_REG && regs[rhs.val] != VAR)
			regs[rhs.val] = FREE;
		regs[res.val] = OCCUPIED;
		return res;
	case PREINC:
	case PREDEC:
	case POSTINC:
	case POSTDEC:
		lhs = genASM(root->mid);
		if (root->kind == POSTINC || root->kind == POSTDEC)
		{
			res = (Symbol){SYMB_REG, getReg()};
			regs[res.val] = OCCUPIED;
			addASM(OP_ADD, res, ZERO, lhs);
		}
		else
			res = lhs;
		addASM(root->kind == POSTINC || root->kind == PREINC ? OP_ADD : OP_SUB, lhs, lhs, ONE);
		modified[root->mid->val - 'x'] = 1;
		return res;
	case IDENTIFIER:
		if (~vars[root->val - 'x'])
			return (Symbol){SYMB_REG, vars[root->val - 'x']};
		if (LOCK)
			return NIL_SYMB;
		res = (Symbol){SYMB_REG, getReg()};
		regs[res.val] = VAR;
		vars[root->val - 'x'] = res.val;
		addASM(OP_LOAD, res, (Symbol){SYMB_MEM, root->val - 'x' << 2}, NIL_SYMB);
		return res;
	case CONSTANT:
		return (Symbol){SYMB_CONST, root->val};
	case MINUS:
		rhs = genASM(root->mid);
		if (rhs.type == SYMB_CONST || regs[rhs.val] == VAR)
		{
			res = (Symbol){SYMB_REG, getReg()};
			regs[res.val] = OCCUPIED;
		}
		else
			res = rhs;
		addASM(OP_SUB, res, ZERO, rhs);
		return res;
	}
}

void printASM(ASM a)
{
	const static char *OPS[] = {"load", "store", "add", "sub", "mul", "div", "rem"};
	printf(OPS[a.op_code]);
	putchar(' ');
	printSymbol(a.d);
	putchar(' ');
	printSymbol(a.s1);
	if (a.op_code != OP_LOAD && a.op_code != OP_STORE)
	{
		putchar(' ');
		printSymbol(a.s2);
	}
	putchar('\n');
}

void execASM(int vars[3])
{
	int regs[256];
	memset(regs, 0, sizeof(regs));
	for (ASM *ptr = asms; ptr != asms_end; ptr++)
	{
		int lhs, rhs, res;
		switch (ptr->s1.type)
		{
		case SYMB_CONST:
			lhs = ptr->s1.val;
			break;
		case SYMB_REG:
			lhs = regs[ptr->s1.val];
			break;
		case SYMB_MEM:
			lhs = vars[ptr->s1.val >> 2];
			break;
		}
		switch (ptr->s2.type)
		{
		case SYMB_CONST:
			rhs = ptr->s2.val;
			break;
		case SYMB_REG:
			rhs = regs[ptr->s2.val];
			break;
		case SYMB_MEM:
			rhs = vars[ptr->s2.val >> 2];
			break;
		}
		switch (ptr->op_code)
		{
		case OP_LOAD:
			res = lhs;
			break;
		case OP_ADD:
			res = lhs + rhs;
			break;
		case OP_SUB:
			res = lhs - rhs;
			break;
		case OP_MUL:
			res = lhs * rhs;
			break;
		case OP_DIV:
			res = lhs / rhs;
			break;
		case OP_REM:
			res = lhs % rhs;
		}
		regs[ptr->d.val] = res;
		if (!~ptr->status)
		{
			ptr->val = res;
			ptr->status = 1;
		}
		else if (ptr->val != res)
			ptr->status = 0;
	}
}

int getReg()
{
	for (int i = 0; i < 256; i++)
		if (regs[i] == FREE)
			return i;
	return 256;
}

void optimize()
{
	srand(time(NULL));
	for (int x = -100; x <= 100; x += 1 + rand() % 3)
		for (int y = -100; y <= 100; y += 1 + rand() % 3)
			for (int z = -100; z <= 100; z += 1 + rand() % 3)
			{
				int vars[] = {x, y, z};
				execASM(vars);
			}
	for (ASM *ptr = asms; ptr != asms_end; ptr++)
		if (ptr->status)
		{
			if (ptr->val >= 0)
			{
				ptr->op_code = OP_ADD;
				ptr->s1 = (Symbol){.type = SYMB_CONST, .val = 0};
				ptr->s2 = (Symbol){.type = SYMB_CONST, .val = ptr->val};
			}
			else
			{
				ptr->op_code = OP_SUB;
				ptr->s1 = (Symbol){.type = SYMB_CONST, .val = 0};
				ptr->s2 = (Symbol){.type = SYMB_CONST, .val = -ptr->val};
			}
		}
	for (int i = 0; i < 3; i++)
		if (modified[i])
			dead_code_eliminate(asms_end - 1, vars[i]);
}

void dead_code_eliminate(ASM *ptr, int r)
{
	while (ptr >= asms && ptr->d.val != r)
		--ptr;
	if (ptr < asms)
		return;
	ptr->dead = 0;
	if (ptr->s1.type == SYMB_REG)
		dead_code_eliminate(ptr - 1, ptr->s1.val);
	if (ptr->s2.type == SYMB_REG)
		dead_code_eliminate(ptr - 1, ptr->s2.val);
}

#pragma endregion NEVIKW39_FUNC_IMPL
