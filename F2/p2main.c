// This file is provided in support of the CS 146 W2020 final assessment.
// All other uses are prohibited unless permission is explicitly granted.
// Republication in any setting by unauthorized parties is prohibited.
//
// Author:  Brad Lushman
// Date:  April 9, 2020

// Place your solution to problem 2 in this file.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parsefr.h"

struct FRAST copy_frast(struct FRAST orig) {

	struct FRAST new_frast;
	new_frast.type = orig.type;

	switch(orig.type) {
		case VAR: ;
			new_frast.v = malloc(sizeof(struct Var));
			strcpy(new_frast.v->name, orig.v->name);
			return new_frast;
		case FUN: ;
			new_frast.f = malloc(sizeof(struct Fun));
			strcpy(new_frast.f->var, orig.f->var);
			new_frast.f->body = copy_frast(orig.f->body);
			return new_frast;
		case APP: ;
			new_frast.a = malloc(sizeof(struct App));
			new_frast.a->fn = copy_frast(orig.a->fn);
			new_frast.a->arg = copy_frast(orig.a->arg);
			return new_frast;
		case BIN: ;
			new_frast.b = malloc(sizeof(struct Bin));
			new_frast.b->op = orig.b->op;
			new_frast.b->arg1 = copy_frast(orig.b->arg1);
			new_frast.b->arg2 = copy_frast(orig.b->arg2);
			return new_frast;
		case NUMBER: ;
			new_frast.n = malloc(sizeof(struct Num));
			new_frast.n->val = orig.n->val;
			return new_frast;
	}
}

int find_op(char op, int arg_one, int arg_two) {

	switch(op) {
		case '+':
			return (arg_one + arg_two);
		case '*':
			return (arg_one * arg_two);
		case '-':
			return (arg_one - arg_two);
		case '/':
			return (arg_one / arg_two);
	}
}

struct FRAST subst(struct FRAST val, char *var, struct FRAST expr) {

	switch(expr.type) {
		case VAR:
			if (strcmp(expr.v->name, var) == 0) {
				struct FRAST new_val = copy_frast(val);
				return new_val;
			}
			else {
				struct FRAST new_var = copy_frast(expr);
				return new_var;
			}
		case FUN:
			if (strcmp(expr.f->var, var) == 0) {
				struct FRAST new_fun = copy_frast(expr);
				return new_fun;
			}
			else {
				struct FRAST temp_body = subst(val, var, expr.f->body);
				
				struct FRAST new_fun;
				new_fun.type = FUN;
				new_fun.f = malloc(sizeof(struct Fun));
				strcpy(new_fun.f->var, expr.f->var);
				new_fun.f->body = copy_frast(temp_body);
				
				freeFR(temp_body);
				return new_fun;
			}
		case APP: ;
			struct FRAST temp_fn = subst(val, var, expr.a->fn);
			struct FRAST temp_arg = subst(val, var, expr.a->arg);
			
			struct FRAST new_app;
			new_app.type = APP;
			new_app.a = malloc(sizeof(struct App));
			new_app.a->fn = copy_frast(temp_fn);
			new_app.a->arg = copy_frast(temp_arg);
			
			freeFR(temp_fn);
			freeFR(temp_arg);
			return new_app;
		case BIN: ;
			struct FRAST temp_arg1 = subst(val, var, expr.b->arg1);
			struct FRAST temp_arg2 = subst(val, var, expr.b->arg2);

			struct FRAST new_bin;
			new_bin.type = BIN;
			new_bin.b = malloc(sizeof(struct Bin));
			new_bin.b->op = expr.b->op;
			new_bin.b->arg1 = copy_frast(temp_arg1);
			new_bin.b->arg2 = copy_frast(temp_arg2);
			
			freeFR(temp_arg1);
			freeFR(temp_arg2);
			return new_bin;
		case NUMBER: ;
			struct FRAST new_num = copy_frast(expr);
			return new_num;
	}
}

struct FRAST interp(struct FRAST expr) {

	switch(expr.type) {
		case VAR: ;
			struct FRAST new_var = copy_frast(expr);
			return new_var;
		case FUN: ;
			struct FRAST new_fun = copy_frast(expr);
			return new_fun;
		case APP: ;
			struct FRAST temp_fn = interp(expr.a->fn);
			struct FRAST temp_arg = interp(expr.a->arg);
			struct FRAST temp_subst = subst(temp_arg, temp_fn.f->var, temp_fn.f->body);
			struct FRAST temp_interp = interp(temp_subst);
			struct FRAST new_interp = copy_frast(temp_interp);
			
			freeFR(temp_fn);
			freeFR(temp_arg);
			freeFR(temp_subst);
			freeFR(temp_interp);
			return new_interp;
		case BIN: ;
			struct FRAST temp_arg1 = interp(expr.b->arg1);
			struct FRAST temp_arg2 = interp(expr.b->arg2);
			
			struct FRAST new_bin_num;
			new_bin_num.type = NUMBER;
			new_bin_num.n = malloc(sizeof(struct Num));
			new_bin_num.n->val = find_op(expr.b->op, temp_arg1.n->val, temp_arg2.n->val);
			
			freeFR(temp_arg1);
			freeFR(temp_arg2);
			return new_bin_num;
		case NUMBER: ;
			struct FRAST new_num = copy_frast(expr);
			return new_num;
	}
}

int main() {
	struct FRAST expr = parseFR(stdin);
	struct FRAST new_expr = interp(expr);
	printFR(new_expr);
	printf("\n");
	freeFR(expr);
	freeFR(new_expr);
}
