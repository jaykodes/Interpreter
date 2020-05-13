// This file is provided in support of the CS 146 W2020 final assessment.
// All other uses are prohibited unless permission is explicitly granted.
// Republication in any setting by unauthorized parties is prohibited.
//
// Author:  Brad Lushman
// Date:  April 9, 2020

// Place your solution to problem 1 here.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parsesimp.h"


int findOp(char op, int arg_one, int arg_two) {

	switch(op) {
		case '+':
			return (arg_one + arg_two);
		case '*':
			return (arg_one * arg_two);
		case '-':
			return (arg_one - arg_two);
		case '/':
			return (arg_one / arg_two);
		case '%':
			return (arg_one % arg_two);
	}
}

int find_value(char *var, struct DeclListNode *env) {

	for (env; env; env = env->next) {
		if (strcmp(env->decl.var, var) == 0) {
			return (env->decl.val);
		}
	}

	return -1;
}


int interpAE(struct AEAST ae, struct DeclListNode *env) {
	
	switch (ae.type) {
		case AEBIN: ;
			int arg_one = interpAE(ae.b->arg1, env);
			int arg_two = interpAE(ae.b->arg2, env);
			return (findOp(ae.b->op, arg_one, arg_two));
		case AENUM:
			return (ae.n->val);
		case AEID:
			return (find_value(ae.i->name, env));
	}
}

void change_env(char *var, int expr_val, struct DeclListNode *env) {
	
	struct DeclListNode *temp = env;

	for (env; env; env = env->next) {
		if (strcmp(env->decl.var, var) == 0) {
			env->decl.val = expr_val;
			return;
		}
	}

	if (temp != NULL) {
		while (temp->next != NULL) {
			temp = temp->next;
		}
	}

	struct Decl new_part;
	strcpy(new_part.var, var);
	new_part.val = expr_val;

	struct DeclListNode *new_elm = malloc(sizeof(struct DeclListNode));
	new_elm->decl = new_part;
	new_elm->next = NULL;

	if (temp == NULL) {
		env = NULL;
	}

	temp->next = new_elm;
}

int find_comp(char *op, int arg_one, int arg_two) {
	
	if (strcmp(op, "=") == 0) {return (arg_one == arg_two);}
	else if (strcmp(op, ">") == 0) {return (arg_one > arg_two);}
	else if (strcmp(op, "<") == 0) {return (arg_one < arg_two);}
	else if (strcmp(op, ">=") == 0) {return (arg_one >= arg_two);}
	else {return (arg_one <= arg_two);}
}

int interpBE(struct BEAST be, struct DeclListNode *env) {

	switch (be.type) {
		case BECOMPARE: ;
			int arg_one = interpAE(be.c->arg1, env);
			int arg_two = interpAE(be.c->arg2, env);
			return (find_comp(be.c->op, arg_one, arg_two));
		case BENOT: ;
			int arg = interpBE(be.n->arg, env);
			return (!arg);
		case BEAND: ;
			int and_one = interpBE(be.a->arg1, env);
			int and_two = interpBE(be.a->arg2, env);
			return (and_one && and_two);
		case BEOR: ;
			int or_one = interpBE(be.a->arg1, env);
			int or_two = interpBE(be.a->arg2, env);
			return (or_one || or_two);
		case BELIT: ;
			if (be.l->val) {return 1;}
			else {return 0;}
	}
}

void interpStmt(struct StmtAST stmt, struct DeclListNode *env) {

	switch (stmt.type) {
		case SKIP:
			return;
		case SEQ:
			interpStmt(stmt.seq->stmt1, env);
			interpStmt(stmt.seq->stmt2, env);
			return;
		case SET: ;
			int expr_val = interpAE(stmt.set->expr, env);
			change_env(stmt.set->var, expr_val, env);
			return;
		case PRINT: ;
			int print_val = interpAE(stmt.print->expr, env);
			printf("%d\n", print_val);
			return;
		case IIF:
			if (interpBE(stmt.iif->test, env)) {
				interpStmt(stmt.iif->tstmt, env);
			}
			else {
				interpStmt(stmt.iif->fstmt, env);
			}
			return;
		case WHILE:
			while (interpBE(stmt.loop->test, env)) {
				interpStmt(stmt.loop->body, env);
			}
			return;
	}
}

int main() {
	struct Program prog = parseProgram(stdin);
	interpStmt(prog.stmt, prog.decls);
	freeProgram(&prog);
}

