	.SPACE $PRIVATE$
	.SUBSPA $DATA$,QUAD=1,ALIGN=8,ACCESS=31
	.SUBSPA $BSS$,QUAD=1,ALIGN=8,ACCESS=31,ZERO,SORT=82
	.SPACE $TEXT$
	.SUBSPA $LIT$,QUAD=0,ALIGN=8,ACCESS=44
	.SUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY

	.SPACE $TEXT$
	.SUBSPA $CODE$

	.align 4
; Basic immediate instruction tests.  
;
; We could/should test some of the corner cases for register and 
; immediate fields.  We should also check the assorted field
; selectors to make sure they're handled correctly.
	pdtlb %r4(%sr0,%r5)
	pdtlb,m %r4(%sr0,%r5)
	pitlb %r4(%sr4,%r5)
	pitlb,m %r4(%sr4,%r5)
	pdtlbe %r4(%sr0,%r5)
	pdtlbe,m %r4(%sr0,%r5)
	pitlbe %r4(%sr4,%r5)
	pitlbe,m %r4(%sr4,%r5)
	pdc %r4(%sr0,%r5)
	pdc,m %r4(%sr0,%r5)
	fdc %r4(%sr0,%r5)
	fdc,m %r4(%sr0,%r5)
	fic %r4(%sr4,%r5)
	fic,m %r4(%sr4,%r5)
	fdce %r4(%sr0,%r5)
	fdce,m %r4(%sr0,%r5)
	fice %r4(%sr4,%r5)
	fice,m %r4(%sr4,%r5)

