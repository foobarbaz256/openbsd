
; test all ops

a1	.EQU	4+10
a2	.EQU	4-10
a3	.EQU	4&10
a4	.EQU	4|2
a5	.EQU	4~2
a6	.EQU	4*10
a7	.EQU	40/10
a8	.EQU	+7
a9	.EQU	-7
a10	.EQU	~7


	a1 a2 a3 a4 a5 a6 a7 a8 a9 a10

; test the priorities

b1	.EQU	1|2~3&4+5-8*7/2
b2	.EQU	(1|2~(3&(4+5-(8*(7/2)))))
b3	.EQU	10*2/3*4
b4	.EQU	(((10*2)/3)*4)
b5	.EQU	10+2-3+4
b6	.EQU	(((10+2)-3)+4)

	b1 b2 b3 b4

; test association

c1	.EQU	-~3
c2	.EQU	~-3
c3	.EQU	-(~3)
c4	.EQU	~(-3)

	c1 c2 c3 c4

; test rules for symbols

ok1	.EQU	FOO
ok2	.EQU	FOO+10
ok3	.EQU	10+FOO
ok4	.EQU	FOO-10

	ok1
	ok2 
	ok3 
	ok4

ok5	.EQU	FOO+3+4+5+6
ok6	.EQU	FOO-BAR

	ok5
	ok6

bad1	.EQU	FOO+FOO
bad2	.EQU	FOO*2
bad3	.EQU	FOO/2
bad4	.EQU	FOO|2
bad5	.EQU	FOO&2
bad6	.EQU	FOO~2
bad7	.EQU	FOO*2

; test spacing

space1	.EQU	1 +	2	+3+FOO + 3
space2

; from the SH manual

	.DATA.L	1+(2-(3+(4-5))),1

	.DATA.L	-H'fffffff1+H'000000f0*H'00000010|H'000000f0&H'0000ffff,H'00000fff

	.DATA.L	-~-~H'0000000f,H'00


	

	.END
