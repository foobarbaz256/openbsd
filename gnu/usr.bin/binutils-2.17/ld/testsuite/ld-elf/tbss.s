	.file	"x.c"
	.globl bss
	.section	.bss,"aw",%nobits
	.p2align 12
	.type	bss,%object
	.size	bss,4096
bss:
	.zero	4096
	.globl tbss
	.section	.tbss,"awT",%nobits
	.p2align 12
	.type	tbss,%object
	.size	tbss,4096
tbss:
	.zero	4096
