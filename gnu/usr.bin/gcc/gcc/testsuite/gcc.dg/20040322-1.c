/* PR c/14069 */
/* { dg-do compile } */
struct S { int a; char b[]; char *c; }; /* { dg-error "error" "flexible array member not" } */
struct S s = { .b = "foo", .c = .b }; /* { dg-error "error" "parse error before" } */
