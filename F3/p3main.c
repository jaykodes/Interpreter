// This file is provided in support of the CS 146 W2020 final assessment.
// All other uses are prohibited unless permission is explicitly granted.
// Republication in any setting by unauthorized parties is prohibited.
//
// Author:  Brad Lushman
// Date:  April 9, 2020

// Place your solution to problem 3 in this file.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parsefr.h"

enum CONTType {APPL = 0, APPR, BINL, BINR, MT};

struct CONT;
struct Mt;
struct AppL;
struct AppR;
struct BinL;
struct BinR;

struct CONT {
	enum CONTType type;
	union {
		struct AppL *al;
		struct AppR *ar;
		struct BinL *bl;
		struct BinR *br;
	};
};

struct AppL {
	struct FRAST arg;
	struct CONT cxt;
};

struct AppR {
	struct FRAST fn;
	struct CONT cxt;
};

struct BinL {
	char op;
	struct FRAST arg2;
	struct CONT cxt;
};

struct BinR {
	char op;
	struct FRAST arg1;
	struct CONT cxt;
};

void *interp();
void *applyCont();

struct CONT new_cxt;
struct FRAST new_expr;

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

void *applyCont() {

	switch(new_cxt.type) {
		case APPL: ;
			struct CONT temp_cxt_al;
			temp_cxt_al.type = APPR;
			temp_cxt_al.ar = malloc(sizeof(struct AppR));
			temp_cxt_al.ar->fn = copy_frast(new_expr);
			temp_cxt_al.ar->cxt = new_cxt.al->cxt;
			
			struct FRAST temp_val_al = copy_frast(new_cxt.al->arg);

			freeFR(new_cxt.al->arg);
			free(new_cxt.al);
			new_cxt = temp_cxt_al;

			freeFR(new_expr);
			new_expr = temp_val_al;

			return interp;
		case APPR: ;
			struct FRAST temp_val_ar = subst(new_expr, (new_cxt.ar->fn).f->var, (new_cxt.ar->fn).f->body);
			freeFR(new_expr);
			new_expr = copy_frast(temp_val_ar);
			freeFR(temp_val_ar);

			struct CONT temp_cxt_ar;
			temp_cxt_ar = new_cxt.ar->cxt;
			freeFR(new_cxt.ar->fn);
           	free(new_cxt.ar);
			new_cxt = temp_cxt_ar;

			return interp;
		case BINL: ;
			struct CONT temp_cxt_bl;
			temp_cxt_bl.type = BINR;
			temp_cxt_bl.br = malloc(sizeof(struct BinR));
			temp_cxt_bl.br->op = new_cxt.bl->op;
			temp_cxt_bl.br->arg1 = copy_frast(new_expr);
			temp_cxt_bl.br->cxt = new_cxt.bl->cxt;
			freeFR(new_cxt.bl->arg2);
            free(new_cxt.bl);
			new_cxt = temp_cxt_bl;

			struct FRAST temp_val_bl = copy_frast(new_cxt.bl->arg2);
			freeFR(new_expr);
			new_expr = temp_val_bl;

			return interp;
		case BINR: ;
			struct FRAST temp_val_br;
			temp_val_br.type = NUMBER;
			temp_val_br.n = malloc(sizeof(struct Num));
			temp_val_br.n->val = find_op(new_cxt.br->op, (new_cxt.br->arg1).n->val, new_expr.n->val);
			freeFR(new_expr);
			new_expr = temp_val_br;

			struct CONT temp_cxt_br;
			temp_cxt_br = new_cxt.br->cxt;
			freeFR(new_cxt.br->arg1);
            free(new_cxt.br);
			new_cxt = temp_cxt_br;

			return applyCont;
		case MT:
			return NULL;
	}
}

void *interp() {

	switch(new_expr.type) {
		case VAR:
			return applyCont;
		case FUN:
			return applyCont;
		case APP: ;
			struct CONT temp_cxt_a;
			temp_cxt_a.type = APPL;
			temp_cxt_a.al = malloc(sizeof(struct AppL));
			temp_cxt_a.al->arg = copy_frast(new_expr.a->arg);
			temp_cxt_a.al->cxt = new_cxt;
			new_cxt = temp_cxt_a;

			struct FRAST temp_expr_a = copy_frast(new_expr.a->fn);
			freeFR(new_expr);
			new_expr = temp_expr_a;

			return interp;
		case BIN: ;
			struct CONT temp_cxt_b;
			temp_cxt_b.type = BINL;
			temp_cxt_b.bl = malloc(sizeof(struct BinL));
			temp_cxt_b.bl->op = new_expr.b->op;
			temp_cxt_b.bl->arg2 = copy_frast(new_expr.b->arg2);
			temp_cxt_b.bl->cxt = new_cxt;
			new_cxt = temp_cxt_b;

			struct FRAST temp_expr_b = copy_frast(new_expr.b->arg1);
			freeFR(new_expr);
			new_expr = temp_expr_b;

			return interp;
		case NUMBER:
			return applyCont;
	}
}

void trampoline(void *(*f)()) {
	while (f) {
		void *g = f();
		f = (void *(*)())g;
	}
}

int main() {
	struct FRAST expr = parseFR(stdin);
	
	new_expr = copy_frast(expr);
	new_cxt.type = MT;

	trampoline(interp);
	printFR(new_expr);
	printf("\n");
	freeFR(new_expr);
	freeFR(expr);
}