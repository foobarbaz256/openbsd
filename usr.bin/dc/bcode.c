/*	$OpenBSD: bcode.c,v 1.8 2003/10/11 18:31:18 otto Exp $	*/

/*
 * Copyright (c) 2003, Otto Moerbeek <otto@drijf.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef lint
static const char rcsid[] = "$OpenBSD: bcode.c,v 1.8 2003/10/11 18:31:18 otto Exp $";
#endif /* not lint */

#include <ssl/ssl.h>
#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "extern.h"

BIGNUM		zero;
static bool	trace = false;

#define MAX_ARRAY_INDEX		2048
#define MAX_RECURSION		100

struct bmachine {
	struct stack		stack;
	u_int			scale;
	u_int			obase;
	u_int			ibase;
	int			readsp;
	struct stack		reg[UCHAR_MAX];
	struct source		readstack[MAX_RECURSION];
};

static struct bmachine	bmachine;

static __inline int	readch(void);
static __inline int	unreadch(void);
static __inline char	*readline(void);
static __inline void	src_free(void);

static __inline u_int	max(u_int, u_int);
static u_long		get_ulong(struct number *);

static __inline void	push_number(struct number *);
static __inline void	push_string(char *);
static __inline void	push(struct value *);
static __inline struct value *tos(void);
static __inline struct number	*pop_number(void);
static __inline char	*pop_string(void);
static __inline void	clear_stack(void);
static __inline void	print_tos(void);
static __inline void	pop_print(void);
static __inline void	print_stack();
static __inline void	dup(void);

static void		get_scale(void);
static void		set_scale(void);
static void		get_obase(void);
static void		set_obase(void);
static void		get_ibase(void);
static void		set_ibase(void);
static void		stackdepth(void);
static void		push_scale(void);
static u_int		count_digits(const struct number *);
static void		num_digits(void);

static void		push_line(void);
static void		bexec(char *);
static void		badd(void);
static void		bsub(void);
static void		bmul(void);
static void		bdiv(void);
static void		bmod(void);
static void		bdivmod(void);
static void		bexp(void);
static bool		bsqrt_stop(const BIGNUM *, const BIGNUM *);
static void		bsqrt(void);
static void		equal(void);
static void		not_equal(void);
static void		less(void);
static void		not_less(void);
static void		greater(void);
static void		not_greater(void);
static void		not_compare(void);
static void		compare(enum bcode_compare);
static void		load(void);
static void		store(void);
static void		load_stack(void);
static void		store_stack(void);
static void		load_array(void);
static void		store_array(void);
static void		nop(void);
static void		quit(void);
static void		quitN(void);
static void		parse_number(void);
static void		unknown(void);
static void		eval_string(char *);
static void		eval_line(void);
static void		eval_tos(void);


typedef void		(*opcode_function)(void);

struct jump_entry {
	u_char		ch;
	opcode_function	f;
};

static opcode_function jump_table[UCHAR_MAX];

static const struct jump_entry jump_table_data[] = {
	{ '0',	parse_number	},
	{ '1',	parse_number	},
	{ '2',	parse_number	},
	{ '3',	parse_number	},
	{ '4',	parse_number	},
	{ '5',	parse_number	},
	{ '6',	parse_number	},
	{ '7',	parse_number	},
	{ '8',	parse_number	},
	{ '9',	parse_number	},
	{ 'A',	parse_number	},
	{ 'B',	parse_number	},
	{ 'C',	parse_number	},
	{ 'D',	parse_number	},
	{ 'E',	parse_number	},
	{ 'F',	parse_number	},
	{ '_',	parse_number	},
	{ '.',	parse_number	},
	{ '+',	badd		},
	{ '-',	bsub		},
	{ '*',	bmul		},
	{ '/',	bdiv		},
	{ '%',	bmod		},
	{ '~',	bdivmod		},
	{ '^',	bexp		},
	{ 's',	store		},
	{ 'S',	store_stack	},
	{ 'l',	load		},
	{ 'L',	load_stack	},
	{ 'd',	dup		},
	{ 'p',	print_tos	},
	{ 'P',	pop_print	},
	{ 'f',	print_stack	},
	{ 'x',	eval_tos	},
	{ 'X',	push_scale	},
	{ '[',	push_line	},
	{ 'q',	quit		},
	{ 'Q',	quitN		},
	{ '<',	less		},
	{ '>',	greater		},
	{ '=',	equal		},
	{ '!',	not_compare	},
	{ 'v',	bsqrt		},
	{ 'c',	clear_stack	},
	{ 'i',	set_ibase	},
	{ 'I',	get_ibase	},
	{ 'o',	set_obase	},
	{ 'O',	get_obase	},
	{ 'k',	set_scale	},
	{ 'K',	get_scale	},
	{ 'z',	stackdepth	},
	{ 'Z',	num_digits	},
	{ '?',	eval_line	},
	{ ';',	load_array	},
	{ ':',	store_array	},
	{ ' ',	nop		},
	{ '\t',	nop		},
	{ '\n',	nop		},
	{ '\f',	nop		},
	{ '\r',	nop		}
};

#define JUMP_TABLE_DATA_SIZE \
	(sizeof(jump_table_data)/sizeof(jump_table_data[0]))

void
init_bmachine(void)
{
	int i;

	for (i = 0; i < UCHAR_MAX; i++)
		jump_table[i] = unknown;
	for (i = 0; i < JUMP_TABLE_DATA_SIZE; i++)
		jump_table[jump_table_data[i].ch] = jump_table_data[i].f;

	stack_init(&bmachine.stack);

	for (i = 0; i < UCHAR_MAX; i++)
		stack_init(&bmachine.reg[i]);

	bmachine.obase = bmachine.ibase = 10;
	BN_init(&zero);
	bn_check(BN_zero(&zero));
}

/* Reset the things needed before processing a (new) file */
void
reset_bmachine(struct source *src)
{
	bmachine.readsp = 0;
	bmachine.readstack[0] = *src;
}

static __inline int
readch(void)
{
	struct source *src = &bmachine.readstack[bmachine.readsp];

	return src->vtable->readchar(src);
}

static __inline int
unreadch(void)
{
	struct source *src = &bmachine.readstack[bmachine.readsp];

	return src->vtable->unreadchar(src);
}

static __inline char *
readline(void)
{
	struct source *src = &bmachine.readstack[bmachine.readsp];

	return src->vtable->readline(src);
}

static __inline void
src_free(void)
{
	struct source *src = &bmachine.readstack[bmachine.readsp];

	src->vtable->free(src);
}

#if 1
void
pn(const char * str, const struct number *n)
{
	char *p = BN_bn2dec(n->number);
	if (p == NULL)
		err(1, "BN_bn2dec failed");
	fputs(str, stderr);
	fprintf(stderr, " %s (%u)\n" , p, n->scale);
	OPENSSL_free(p);
}

void
pbn(const char * str, const BIGNUM *n)
{
	char *p = BN_bn2dec(n);
	if (p == NULL)
		err(1, "BN_bn2dec failed");
	fputs(str, stderr);
	fprintf(stderr, " %s\n", p);
	OPENSSL_free(p);
}

#endif

static __inline u_int
max(u_int a, u_int b)
{
	return a > b ? a : b;
}

static unsigned long factors[] = {
	0, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
	100000000, 1000000000
};

void
scale_number(BIGNUM *n, int s)
{
	int abs_scale;

	if (s == 0)
		return;

	abs_scale = s > 0 ? s : -s;

	if (abs_scale < sizeof(factors)/sizeof(factors[0])) {
		if (s > 0)
			bn_check(BN_mul_word(n, factors[abs_scale]));
		else
			BN_div_word(n, factors[abs_scale]);
	} else {
		BIGNUM *a, *p;
		BN_CTX *ctx;

		a = BN_new();
		bn_checkp(a);
		p = BN_new();
		bn_checkp(p);
		ctx = BN_CTX_new();
		bn_checkp(ctx);

		bn_check(BN_set_word(a, 10));
		bn_check(BN_set_word(p, abs_scale));
		bn_check(BN_exp(a, a, p, ctx));
		if (s > 0)
			bn_check(BN_mul(n, n, a, ctx));
		else
			bn_check(BN_div(n, NULL, n, a, ctx));
		BN_CTX_free(ctx);
		BN_free(a);
		BN_free(p);
	}
}

void
split_number(const struct number *n, BIGNUM *i, BIGNUM *f)
{
	u_long rem;

	bn_checkp(BN_copy(i, n->number));

	if (n->scale == 0 && f != NULL)
		BN_zero(f);
	else if (n->scale < sizeof(factors)/sizeof(factors[0])) {
		rem = BN_div_word(i, factors[n->scale]);
		if (f != NULL)
			BN_set_word(f, rem);
	} else {
		BIGNUM *a, *p;
		BN_CTX *ctx;

		a = BN_new();
		bn_checkp(a);
		p = BN_new();
		bn_checkp(p);
		ctx = BN_CTX_new();
		bn_checkp(ctx);

		bn_check(BN_set_word(a, 10));
		bn_check(BN_set_word(p, n->scale));
		bn_check(BN_exp(a, a, p, ctx));
		bn_check(BN_div(i, f, n->number, a, ctx));
		BN_CTX_free(ctx);
		BN_free(a);
		BN_free(p);
	}
}

__inline void
normalize(struct number *n, u_int s)
{
	scale_number(n->number, s - n->scale);
	n->scale = s;
}

static u_long
get_ulong(struct number *n)
{
	normalize(n, 0);
	return BN_get_word(n->number);
}

void
negate(struct number *n)
{
	bn_check(BN_sub(n->number, &zero, n->number));
}

static __inline void
push_number(struct number *n)
{
	stack_pushnumber(&bmachine.stack, n);
}

static __inline void
push_string(char *string)
{
	stack_pushstring(&bmachine.stack, string);
}

static __inline void
push(struct value *v)
{
	stack_push(&bmachine.stack, v);
}

static __inline struct value *
tos(void)
{
	return stack_tos(&bmachine.stack);
}

static __inline struct value *
pop(void)
{
	return stack_pop(&bmachine.stack);
}

static __inline struct number *
pop_number(void)
{
	return stack_popnumber(&bmachine.stack);
}

static __inline char *
pop_string(void)
{
	return stack_popstring(&bmachine.stack);
}

static __inline void
clear_stack(void)
{
	stack_clear(&bmachine.stack);
}

static __inline void
print_stack(void)
{
	stack_print(stdout, &bmachine.stack, "", bmachine.obase);
}

static __inline void
print_tos(void)
{
	struct value *value = tos();
	if (value != NULL) {
		print_value(stdout, value, "", bmachine.obase);
		putchar('\n');
	}
	else
		warnx("stack empty");
}

static __inline void
pop_print(void)
{
	struct value *value = pop();
	if (value != NULL) {
		switch (value->type) {
		case BCODE_NONE:
			break;
		case BCODE_NUMBER:
			normalize(value->u.num, 0);
			print_ascii(stdout, value->u.num);
			fflush(stdout);
			break;
		case BCODE_STRING:
			fputs(value->u.string, stdout);
			fflush(stdout);
			break;
		}
		stack_free_value(value);
	}
}

static __inline void
dup(void)
{
	stack_dup(&bmachine.stack);
}

static void
get_scale(void)
{
	struct number	*n;

	n = new_number();
	bn_check(BN_set_word(n->number, bmachine.scale));
	push_number(n);
}

static void
set_scale(void)
{
	struct number	*n;
	u_long		scale;

	n = pop_number();
	if (n != NULL) {
		if (BN_cmp(n->number, &zero) < 0)
			warnx("scale must be a nonnegative number");
		else {
			scale = get_ulong(n);
			if (scale != BN_MASK2)
				bmachine.scale = scale;
			else
				warnx("scale too large");
			}
		free_number(n);
	}
}

static void
get_obase(void)
{
	struct number	*n;

	n = new_number();
	bn_check(BN_set_word(n->number, bmachine.obase));
	push_number(n);
}

static void
set_obase(void)
{
	struct number	*n;
	u_long		base;

	n = pop_number();
	if (n != NULL) {
		base = get_ulong(n);
		if (base != BN_MASK2 && base > 1)
			bmachine.obase = base;
		else
			warnx("output base must be a number greater than 1");
		free_number(n);
	}
}

static void
get_ibase(void)
{
	struct number *n;

	n = new_number();
	bn_check(BN_set_word(n->number, bmachine.ibase));
	push_number(n);
}

static void
set_ibase(void)
{
	struct number	*n;
	u_long		base;

	n = pop_number();
	if (n != NULL) {
		base = get_ulong(n);
		if (base != BN_MASK2 && 2 <= base && base <= 16)
			bmachine.ibase = base;
		else
			warnx("input base must be a number between 2 and 16 "
			    "(inclusive)");
		free_number(n);
	}
}

static void
stackdepth(void)
{
	u_int i;
	struct number *n;

	i = stack_size(&bmachine.stack);
	n = new_number();
	bn_check(BN_set_word(n->number, i));
	push_number(n);
}

static void
push_scale(void)
{
	struct value	*value;
	u_int		scale = 0;
	struct number	*n;


	value = pop();
	if (value != NULL) {
		switch (value->type) {
		case BCODE_NONE:
			return;
		case BCODE_NUMBER:
			scale = value->u.num->scale;
			break;
		case BCODE_STRING:
			break;
		}
		stack_free_value(value);
		n = new_number();
		bn_check(BN_set_word(n->number, scale));
		push_number(n);
	}
}

static u_int
count_digits(const struct number *n)
{
	struct number	*int_part, *fract_part;
	u_int		i;

	if (BN_is_zero(n->number))
		return 1;

	int_part = new_number();
	fract_part = new_number();
	fract_part->scale = n->scale;
	split_number(n, int_part->number, fract_part->number);

	i = 0;
	while (!BN_is_zero(int_part->number)) {
		BN_div_word(int_part->number, 10);
		i++;
	}
	free_number(int_part);
	free_number(fract_part);
	return i + n->scale;
}

static void
num_digits(void)
{
	struct value	*value;
	u_int		digits;
	struct number	*n;

	value = pop();
	if (value != NULL) {
		switch (value->type) {
		case BCODE_NONE:
			break;
		case BCODE_NUMBER:
			digits = count_digits(value->u.num);
			n = new_number();
			bn_check(BN_set_word(n->number, digits));
			/* free first, then reassign */
			BN_free(value->u.num->number);
			push_number(n);
			break;
		case BCODE_STRING:
			digits = strlen(value->u.string);
			n = new_number();
			bn_check(BN_set_word(n->number, digits));
			/* free first, then reassign */
			free(value->u.string);
			push_number(n);
			break;
		}
	}
}

static void
load(void)
{
	int		index;
	struct value	*v, copy;
	struct number	*n;

	index = readch();
	if (0 <= index && index < UCHAR_MAX) {
		v = stack_tos(&bmachine.reg[index]);
		if (v == NULL) {
			n = new_number();
			bn_check(BN_zero(n->number));
			push_number(n);
		} else
			push(stack_dup_value(v, &copy));
	} else
		warnx("internal error: reg num = %d", index);
}

static void
store(void)
{
	int		index;
	struct value	*val;

	index = readch();
	if (0 <= index && index < UCHAR_MAX) {
		val = pop();
		if (val == NULL) {
			return;
		}
		stack_set_tos(&bmachine.reg[index], val);
	} else
		warnx("internal error: reg num = %d", index);
}

static void
load_stack(void)
{
	int		index;
	struct stack	*stack;
	struct value	*value, copy;

	index = readch();
	if (0 <= index && index < UCHAR_MAX) {
		stack = &bmachine.reg[index];
		value = NULL;
		if (stack_size(stack) > 0) {
			value = stack_pop(stack);
		}
		if (value != NULL)
			push(stack_dup_value(value, &copy));
		else
			warnx("stack register '%c' (0%o) is empty",
			    index, index);
	} else
		warnx("internal error: reg num = %d", index);
}

static void
store_stack(void)
{
	int		index;
	struct value	*value;

	index = readch();
	if (0 <= index && index < UCHAR_MAX) {
		value = pop();
		if (value == NULL)
			return;
		stack_push(&bmachine.reg[index], value);
	} else
		warnx("internal error: reg num = %d", index);
}

static void
load_array(void)
{
	int			reg;
	struct number		*inumber, *n;
	u_long			index;
	struct stack		*stack;
	struct value		*v, copy;

	reg = readch();
	if (0 <= reg && reg < UCHAR_MAX) {
		inumber = pop_number();
		if (inumber == NULL)
			return;
		index = get_ulong(inumber);
		if (BN_cmp(inumber->number, &zero) < 0)
			warnx("negative index");
		else if (index == BN_MASK2 || index > MAX_ARRAY_INDEX)
			warnx("index too big");
		else {
			stack = &bmachine.reg[reg];
			v = frame_retrieve(stack, index);
			if (v == NULL) {
				n = new_number();
				bn_check(BN_zero(n->number));
				push_number(n);
			}
			else
				push(stack_dup_value(v, &copy));
		}
		free_number(inumber);
	} else
		warnx("internal error: reg num = %d", reg);
}

static void
store_array(void)
{
	int			reg;
	struct number		*inumber;
	u_long			index;
	struct value		*value;
	struct stack		*stack;

	reg = readch();
	if (0 <= reg && reg < UCHAR_MAX) {
		inumber = pop_number();
		if (inumber == NULL)
			return;
		value = pop();
		if (value == NULL) {
			free_number(inumber);
			return;
		}
		index = get_ulong(inumber);
		if (BN_cmp(inumber->number, &zero) < 0) {
			warnx("negative index");
			stack_free_value(value);
		} else if (index == BN_MASK2 || index > MAX_ARRAY_INDEX) {
			warnx("index too big");
			stack_free_value(value);
		} else {
			stack = &bmachine.reg[reg];
			frame_assign(stack, index, value);
		}
		free_number(inumber);
	} else
		warnx("internal error: reg num = %d", reg);
}

static void
push_line(void)
{
	push_string(read_string(&bmachine.readstack[bmachine.readsp]));
}

static void
bexec(char *line)
{
	system(line);
	free(line);
}

static void
badd(void)
{
	struct number	*a, *b;
	struct number	*r;

	a = pop_number();
	if (a == NULL) {
		return;
	}
	b = pop_number();
	if (b == NULL) {
		push_number(a);
		return;
	}

	r = new_number();
	r->scale = max(a->scale, b->scale);
	if (r->scale > a->scale)
		normalize(a, r->scale);
	else if (r->scale > b->scale)
		normalize(b, r->scale);
	bn_check(BN_add(r->number, a->number, b->number));
	push_number(r);
	free_number(a);
	free_number(b);
}

static void
bsub(void)
{
	struct number	*a, *b;
	struct number	*r;

	a = pop_number();
	if (a == NULL) {
		return;
	}
	b = pop_number();
	if (b == NULL) {
		push_number(a);
		return;
	}

	r = new_number();

	r->scale = max(a->scale, b->scale);
	if (r->scale > a->scale)
		normalize(a, r->scale);
	else if (r->scale > b->scale)
		normalize(b, r->scale);
	bn_check(BN_sub(r->number, b->number, a->number));
	push_number(r);
	free_number(a);
	free_number(b);
}

void
bmul_number(struct number *r, struct number *a, struct number *b)
{
	BN_CTX		*ctx;

	/* Create copies of the scales, since r might be equal to a or b */
	u_int ascale = a->scale;
	u_int bscale = b->scale;
	u_int rscale = ascale + bscale;

	ctx = BN_CTX_new();
	bn_checkp(ctx);
	bn_check(BN_mul(r->number, a->number, b->number, ctx));
	BN_CTX_free(ctx);

	if (rscale > bmachine.scale && rscale > ascale && rscale > bscale) {
		r->scale = rscale;
		normalize(r, max(bmachine.scale, max(ascale, bscale)));
	} else
		r->scale = rscale;
}

static void
bmul(void)
{
	struct number	*a, *b;
	struct number	*r;

	a = pop_number();
	if (a == NULL) {
		return;
	}
	b = pop_number();
	if (b == NULL) {
		push_number(a);
		return;
	}

	r = new_number();
	bmul_number(r, a, b);

	push_number(r);
	free_number(a);
	free_number(b);
}

static void
bdiv(void)
{
	struct number	*a, *b;
	struct number	*r;
	u_int		scale;
	BN_CTX		*ctx;

	a = pop_number();
	if (a == NULL) {
		return;
	}
	b = pop_number();
	if (b == NULL) {
		push_number(a);
		return;
	}

	r = new_number();
	r->scale = bmachine.scale;
	scale = max(a->scale, b->scale);

	if (BN_is_zero(a->number))
		warnx("divide by zero");
	else {
		normalize(a, scale);
		normalize(b, scale + r->scale);

		ctx = BN_CTX_new();
		bn_checkp(ctx);
		bn_check(BN_div(r->number, NULL, b->number, a->number, ctx));
		BN_CTX_free(ctx);
	}
	push_number(r);
	free_number(a);
	free_number(b);
}

static void
bmod(void)
{
	struct number	*a, *b;
	struct number	*r;
	u_int		scale;
	BN_CTX		*ctx;

	a = pop_number();
	if (a == NULL) {
		return;
	}
	b = pop_number();
	if (b == NULL) {
		push_number(a);
		return;
	}

	r = new_number();
	scale = max(a->scale, b->scale);
	r->scale = max(b->scale, a->scale + bmachine.scale);

	if (BN_is_zero(a->number))
		warnx("remainder by zero");
	else {
		normalize(a, scale);
		normalize(b, scale + bmachine.scale);

		ctx = BN_CTX_new();
		bn_checkp(ctx);
		bn_check(BN_mod(r->number, b->number, a->number, ctx));
		BN_CTX_free(ctx);
	}
	push_number(r);
	free_number(a);
	free_number(b);
}

static void
bdivmod(void)
{
	struct number	*a, *b;
	struct number	*rdiv, *rmod;
	u_int		scale;
	BN_CTX		*ctx;

	a = pop_number();
	if (a == NULL) {
		return;
	}
	b = pop_number();
	if (b == NULL) {
		push_number(a);
		return;
	}

	rdiv = new_number();
	rmod = new_number();
	rdiv->scale = bmachine.scale;
	rmod->scale = max(b->scale, a->scale + bmachine.scale);
	scale = max(a->scale, b->scale);

	if (BN_is_zero(a->number))
		warnx("divide by zero");
	else {
		normalize(a, scale);
		normalize(b, scale + bmachine.scale);

		ctx = BN_CTX_new();
		bn_checkp(ctx);
		bn_check(BN_div(rdiv->number, rmod->number,
		    b->number, a->number, ctx));
		BN_CTX_free(ctx);
	}
	push_number(rdiv);
	push_number(rmod);
	free_number(a);
	free_number(b);
}

static void
bexp(void)
{
	struct number	*a, *p;
	struct number	*r;
	bool		neg;
	u_int		scale;

	p = pop_number();
	if (p == NULL) {
		return;
	}
	a = pop_number();
	if (a == NULL) {
		push_number(p);
		return;
	}

	if (p->scale != 0)
		warnx("Runtime warning: non-zero scale in exponent");
	normalize(p, 0);

	neg = false;
	if (BN_cmp(p->number, &zero) < 0) {
		neg = true;
		negate(p);
		scale = bmachine.scale;
	} else {
		/* Posix bc says min(a.scale * b, max(a.scale, scale) */
		u_long	b;
		u_int	m;

		b = BN_get_word(p->number);
		m = max(a->scale, bmachine.scale);
		scale = a->scale * b;
		if (scale > m || b == BN_MASK2)
			scale = m;
	}

	if (BN_is_zero(p->number)) {
		r = new_number();
		bn_check(BN_one(r->number));
		normalize(r, scale);
	} else {
		while (!BN_is_bit_set(p->number, 0)) {
			bmul_number(a, a, a);
			bn_check(BN_rshift1(p->number, p->number));
		}

		r = dup_number(a);
		normalize(r, scale);
		bn_check(BN_rshift1(p->number, p->number));

		while (!BN_is_zero(p->number)) {
			bmul_number(a, a, a);
			if (BN_is_bit_set(p->number, 0))
				bmul_number(r, r, a);
			bn_check(BN_rshift1(p->number, p->number));
		}

		if (neg) {
			BN_CTX	*ctx;
			BIGNUM	*one;

			one = BN_new();
			bn_checkp(one);
			BN_one(one);
			ctx = BN_CTX_new();
			bn_checkp(ctx);
			r->scale = scale;
			scale_number(one, r->scale);
			bn_check(BN_div(r->number, NULL, one, r->number, ctx));
			BN_free(one);
			BN_CTX_free(ctx);
		}
	}
	push_number(r);
	free_number(a);
	free_number(p);
}

static bool
bsqrt_stop(const BIGNUM *x, const BIGNUM *y)
{
	BIGNUM *r;
	bool ret;

	r = BN_new();
	bn_checkp(r);
	bn_check(BN_sub(r, x, y));
	ret = BN_is_one(r) || BN_is_zero(r);
	BN_free(r);
	return ret;
}

static void
bsqrt(void)
{
	struct number	*n;
	struct number	*r;
	BIGNUM		*x, *y;
	u_int		scale;
	BN_CTX		*ctx;

	n = pop_number();
	if (n == NULL) {
		return;
	}
	if (BN_is_zero(n->number)) {
		r = new_number();
		push_number(r);
	} else if (BN_cmp(n->number, &zero) < 0)
		warnx("square root of negative number");
	else {
		scale = max(bmachine.scale, n->scale);
		normalize(n, 2*scale);
		x = BN_dup(n->number);
		bn_checkp(x);
		bn_check(BN_rshift(x, x, BN_num_bits(x)/2));
		y = BN_new();
		bn_checkp(y);
		ctx = BN_CTX_new();
		bn_checkp(ctx);
		for (;;) {
			bn_checkp(BN_copy(y, x));
			bn_check(BN_div(x, NULL, n->number, x, ctx));
			bn_check(BN_add(x, x, y));
			bn_check(BN_rshift1(x, x));
			if (bsqrt_stop(x, y))
				break;
		}
		r = bmalloc(sizeof(*r));
		r->scale = scale;
		r->number = y;
		BN_free(x);
		BN_CTX_free(ctx);
		push_number(r);
	}

	free_number(n);
}

static void
equal(void)
{
	compare(BCODE_EQUAL);
}

static void
not_equal(void)
{
	compare(BCODE_NOT_EQUAL);
}

static void
less(void)
{
	compare(BCODE_LESS);
}

static void
not_compare(void)
{
	switch (readch()) {
	case '<':
		not_less();
		break;
	case '>':
		not_greater();
		break;
	case '=':
		not_equal();
		break;
	default:
		unreadch();
		bexec(readline());
		break;
	}
}

static void
not_less(void)
{
	compare(BCODE_NOT_LESS);
}

static void
greater(void)
{
	compare(BCODE_GREATER);
}

static void
not_greater(void)
{
	compare(BCODE_NOT_GREATER);
}

static void
compare(enum bcode_compare type)
{
	int		index;
	struct number	*a, *b;
	u_int		scale;
	int		cmp;
	bool		ok;
	struct value	*v;

	index = readch();

	a = pop_number();
	if (a == NULL) {
		return;
	}
	b = pop_number();
	if (b == NULL) {
		push_number(a);
		return;
	}

	scale = max(a->scale, b->scale);
	if (scale > a->scale)
		normalize(a, scale);
	else if (scale > scale)
		normalize(b, scale);

	cmp = BN_cmp(a->number, b->number);

	free_number(a);
	free_number(b);

	ok = false;
	switch (type) {
	case BCODE_EQUAL:
		ok = cmp == 0;
		break;
	case BCODE_NOT_EQUAL:
		ok = cmp != 0;
		break;
	case BCODE_LESS:
		ok = cmp < 0;
		break;
	case BCODE_NOT_LESS:
		ok = cmp >= 0;
		break;
	case BCODE_GREATER:
		ok = cmp > 0;
		break;
	case BCODE_NOT_GREATER:
		ok = cmp <= 0;
		break;
	}

	if (ok) {
		v = stack_tos(&bmachine.reg[index]);
		if (v == NULL)
			warn("stack empty");
		else {
			switch(v->type) {
			case BCODE_NONE:
				warnx("register '%c' (0%o) is empty",
				    index, index);
				break;
			case BCODE_NUMBER:
				warn("eval called with non-string argument");
				break;
			case BCODE_STRING:
				eval_string(bstrdup(v->u.string));
				break;
			}
		}
	}
}


static void
nop(void)
{
}

static void
quit(void)
{
	if (bmachine.readsp < 2)
		exit(0);
	src_free();
	bmachine.readsp--;
	src_free();
	bmachine.readsp--;
}

static void
quitN(void)
{
	struct number	*n;
	u_long		i;

	n = pop_number();
	if (n == NULL)
		return;
	i = get_ulong(n);
	if (i == BN_MASK2 || i == 0)
		warnx("Q command requires a number >= 1");
	else if (bmachine.readsp < i)
		warnx("Q command argument exceeded string execution depth");
	else {
		while (i-- > 0) {
			src_free();
			bmachine.readsp--;
		}
	}
}

static void
parse_number(void)
{
	unreadch();
	push_number(readnumber(&bmachine.readstack[bmachine.readsp],
	    bmachine.ibase));
}

static void
unknown(void)
{
	int ch = bmachine.readstack[bmachine.readsp].lastchar;
	warnx("%c (0%o) is unimplemented", ch, ch);
}

static void
eval_string(char *p)
{
	int ch;

	if (bmachine.readsp > 0) {
		/* Check for tail call. Do not recurse in that case. */
		ch = readch();
		if (ch == EOF) {
			src_free();
			src_setstring(&bmachine.readstack[bmachine.readsp], p);
			return;
		} else
			unreadch();
	}
	if (bmachine.readsp == MAX_RECURSION)
		errx(1, "recursion too deep");
	src_setstring(&bmachine.readstack[++bmachine.readsp], p);
}

static void
eval_line(void)
{
	/* Always read from stdin */
	struct source	in;
	char		*p;

	src_setstream(&in, stdin);
	p = (*in.vtable->readline)(&in);
	eval_string(p);
}

static void
eval_tos(void)
{
	char *p;

	p = pop_string();
	if (p == NULL)
		return;
	eval_string(p);
}

void
eval(void)
{
	int	ch;

	for (;;) {
		ch = readch();
		if (ch == EOF) {
			if (bmachine.readsp == 0)
				exit(0);
			src_free();
			bmachine.readsp--;
			continue;
		}
		if (trace) {
			fprintf(stderr, "# %c\n", ch);
			stack_print(stderr, &bmachine.stack, "* ",
			    bmachine.obase);
			fprintf(stderr, "%d =>\n", bmachine.readsp);
		}

		if (0 <= ch && ch < UCHAR_MAX)
			(*jump_table[ch])();
		else
			warnx("internal error: opcode %d", ch);

		if (trace) {
			stack_print(stderr, &bmachine.stack, "* ",
			    bmachine.obase);
			fprintf(stderr, "%d ==\n", bmachine.readsp);
		}
	}
}
