	dog2=15
	dog3=-16
	dog4=7
	dog5=-8
	;; idx - idx
	ldaa #0
	movb 15,x,dog2,x
	movb 15,x,cat2,x
	movb 15,x,15,x
	ldaa #1
	movb dog2,x,15,x
	movb cat2,x,15,x
	movb 15,x,15,x
	ldaa #2
	movb 15,x,dog3,x
	movb 15,x,cat3,x	
	movb 15,x,-16,x
	ldaa #3		
	movb dog3,x,15,x
	movb cat3,x,15,x	
	movb -16,x,15,x
	ldaa #4
	movw 15,x,dog2,x
	movw 15,x,cat2,x
	movw 15,x,15,x
	ldaa #5
	movw dog2,x,15,x
	movw cat2,x,15,x
	movw 15,x,15,x
	ldaa #6
	movw 15,x,dog3,x
	movw 15,x,cat3,x
	movw 15,x,-16,x
	ldaa #7
	movw dog3,x,15,x
	movw cat3,x,15,x
	movw -16,x,15,x
	ldaa #8
	movb 15,y,dog2,y
	movb 15,y,cat2,y
	movb 15,y,15,y
	ldaa #9
	movb dog2,y,15,y
	movb cat2,y,15,y
	movb 15,y,15,y
	ldaa #10
	movb 15,y,dog3,y
	movb 15,y,cat3,y
	movb 15,y,-16,y
	ldaa #11
	movb dog3,y,15,y
	movb cat3,y,15,y
	movb -16,y,15,y
	ldaa #12
	movw 15,y,dog2,y
	movw 15,y,cat2,y
	movw 15,y,15,y
	ldaa #13
	movw dog2,y,15,y
	movw cat2,y,15,y
	movw 15,y,15,y
	ldaa #14
	movw 15,y,dog3,y
	movw 15,y,cat3,y
	movw 15,y,-16,y
	ldaa #15
	movw dog3,y,15,y
	movw cat3,y,15,y
	movw -16,y,15,y
	ldaa #16
	movb 15,y,dog2,pc
	movb 15,y,cat2,pc
	movb 15,y,15,pc
	ldaa #17
	movb dog2,y,15,pc
	movb cat2,y,15,pc
	movb 15,y,15,pc
	ldaa #18
	movb 15,y,dog3,pc
	movb 15,y,cat3,pc
	movb 15,y,-16,pc
	ldaa #19
	movb dog3,y,15,pc
	movb cat3,y,15,pc
	movb -16,y,15,pc
	ldaa #20
	movw 15,y,dog2,pc
	movw 15,y,cat2,pc
	movw 15,y,15,pc
	ldaa #21
	movw dog2,y,15,pc
	movw cat2,y,15,pc
	movw 15,y,15,pc
	ldaa #22
	movw 15,y,dog3,pc
	movw 15,y,cat3,pc
	movw 15,y,-16,pc
	ldaa #23
	movw dog3,y,15,pc
	movw cat3,y,15,pc
	movw -16,y,15,pc
	ldaa #24
	movb 15,sp,dog2,pc
	movb 15,sp,cat2,pc
	movb 15,sp,15,pc
	ldaa #25
	movb dog2,sp,15,pc
	movb cat2,sp,15,pc
	movb 15,sp,15,pc
	ldaa #26
	movb 15,sp,dog3,pc
	movb 15,sp,cat3,pc
	movb 15,sp,-16,pc
	ldaa #27
	movb dog3,sp,15,pc
	movb cat3,sp,15,pc
	movb -16,sp,15,pc
	ldaa #28
	movw 15,sp,dog2,pc
	movw 15,sp,cat2,pc
	movw 15,sp,15,pc
	ldaa #29
	movw dog2,sp,15,pc
	movw cat2,sp,15,pc
	movw 15,sp,15,pc
	ldaa #30
	movw 15,sp,dog3,pc
	movw 15,sp,cat3,pc
	movw 15,sp,-16,pc
	ldaa #31
	movw dog3,sp,15,pc
	movw cat3,sp,15,pc
	movw -16,sp,15,pc
	ldaa #32
	;; ext - idx
	;; idx - ext
	movb 0x1000,dog2,x
	movb 0x1000,cat2,x
	movb 0x1000,15,x
	ldaa #33
	movb dog2,x,0x1000
	movb cat2,x,0x1000
	movb 15,x,0x1000
	ldaa #34
	movb 0x1000,dog3,x
	movb 0x1000,cat3,x
	movb 0x1000,-16,x
	ldaa #35
	movb dog3,x,0x1000
	movb cat3,x,0x1000
	movb -16,x,0x1000
	ldaa #36
	movw 0x1002,dog2,x
	movw 0x1002,cat2,x
	movw 0x1002,15,x
	ldaa #37
	movw dog2,x,0x1002
	movw cat2,x,0x1002
	movw 15,x,0x1002
	ldaa #38
	movw 0x1002,dog3,x
	movw 0x1002,cat3,x
	movw 0x1002,-16,x
	ldaa #39
	movw dog3,x,0x1002
	movw cat3,x,0x1002
	movw -16,x,0x1002
	ldaa #40
	movb 0x1000,dog2,y
	movb 0x1000,cat2,y
	movb 0x1000,15,y
	ldaa #41
	movb dog2,y,0x1000
	movb cat2,y,0x1000
	movb 15,y,0x1000
	ldaa #42
	movb 0x1000,dog3,y
	movb 0x1000,cat3,y
	movb 0x1000,-16,y
	ldaa #43
	movb dog3,y,0x1000
	movb cat3,y,0x1000
	movb -16,y,0x1000
	ldaa #44
	movw 0x1002,dog2,y
	movw 0x1002,cat2,y
	movw 0x1002,15,y
	ldaa #45
	movw dog2,y,0x1002
	movw cat2,y,0x1002
	movw 15,y,0x1002
	ldaa #46
	movw 0x1002,dog3,y
	movw 0x1002,cat3,y
	movw 0x1002,-16,y
	ldaa #47
	movw dog3,y,0x1002
	movw cat3,y,0x1002
	movw -16,y,0x1002
	ldaa #48
	movb 0x1000,dog2,pc
	movb 0x1000,cat2,pc
	movb 0x1000,15,pc
	ldaa #49
	movb dog2,pc,0x1000
	movb cat2,pc,0x1000
	movb 15,pc,0x1000
	ldaa #50
	movb 0x1000,dog3,pc
	movb 0x1000,cat3,pc
	movb 0x1000,-16,pc
	ldaa #51
	movb dog3,pc,0x1000
	movb cat3,pc,0x1000
	movb -16,pc,0x1000
	ldaa #52
	movw 0x1002,dog2,pc
	movw 0x1002,cat2,pc
	movw 0x1002,15,pc
	ldaa #53
	movw dog2,pc,0x1002
	movw cat2,pc,0x1002
	movw 15,pc,0x1002
	ldaa #54
	movw 0x1002,dog3,pc
	movw 0x1002,cat3,pc
	movw 0x1002,-16,pc
	ldaa #55
	movw dog3,pc,0x1002
	movw cat3,pc,0x1002
	movw -16,pc,0x1002
	ldaa #56
	movb 0x1000,dog2,sp
	movb 0x1000,cat2,sp
	movb 0x1000,15,sp
	ldaa #57
	movb dog2,sp,0x1000
	movb cat2,sp,0x1000
	movb 15,sp,0x1000
	ldaa #58
	movb 0x1000,dog3,sp
	movb 0x1000,cat3,sp
	movb 0x1000,-16,sp
	ldaa #59
	movb dog3,sp,0x1000
	movb cat3,sp,0x1000
	movb -16,sp,0x1000
	ldaa #60
	movw 0x1002,dog2,sp
	movw 0x1002,cat2,sp
	movw 0x1002,15,sp
	ldaa #61
	movw dog2,sp,0x1002
	movw cat2,sp,0x1002
	movw 15,sp,0x1002
	ldaa #62
	movw 0x1002,dog3,sp
	movw 0x1002,cat3,sp
	movw 0x1002,-16,sp
	ldaa #63
	movw dog3,sp,0x1002
	movw cat3,sp,0x1002
	movw -16,sp,0x1002
	ldaa #64
	;; imm - idx
	movb #0xaa,dog4,x
	movb #0xaa,cat4,x
	movb #0xaa,7,x
	ldaa #65
	movb #0xaa,dog5,x
	movb #0xaa,cat5,x
	movb #0xaa,-8,x
	ldaa #66
	movw #0x44,dog4,x
	movw #0x44,cat4,x
	movw #0x44,7,x
	ldaa #67
	movw #0x44,dog5,x
	movw #0x44,cat5,x
	movw #0x44,-8,x
	ldaa #68
	movb #0xaa,dog4,y
	movb #0xaa,cat4,y
	movb #0xaa,7,y
	ldaa #69
	movb #0xaa,dog5,y
	movb #0xaa,cat5,y
	movb #0xaa,-8,y
	ldaa #70
	movw #0x44,dog4,y
	movw #0x44,cat4,y
	movw #0x44,7,y
	ldaa #71
	movw #0x44,dog5,y
	movw #0x44,cat5,y
	movw #0x44,-8,y
	ldaa #72
	movb #0xaa,dog4,pc
	movb #0xaa,cat4,pc
	movb #0xaa,7,pc
	ldaa #73
	movb #0xaa,dog5,pc
	movb #0xaa,cat5,pc
	movb #0xaa,-8,pc
	ldaa #74
	movw #0x44,dog4,pc
	movw #0x44,cat4,pc
	movw #0x44,7,pc
	ldaa #75
	movw #0x44,dog5,pc
	movw #0x44,cat5,pc
	movw #0x44,-8,pc
	ldaa #76
	movb #0xaa,dog4,sp
	movb #0xaa,cat4,sp
	movb #0xaa,7,sp
	ldaa #77
	movb #0xaa,dog5,sp
	movb #0xaa,cat5,sp
	movb #0xaa,-8,sp
	ldaa #78
	movw #0x44,dog4,sp
	movw #0x44,cat4,sp
	movw #0x44,7,sp
	ldaa #79
	movw #0x44,dog5,sp
	movw #0x44,cat5,sp
	movw #0x44,-8,sp
	ldaa #80
	cat2=15
	cat3=-16
	cat4=7
	cat5=-8
