	@AREA |.text|,CODE,READONLY
	@EXPORT argb8888toyuv420
	@EXPORT argb8888toyuv420_test
	@0:hght
	@4:width
	@8:i
	@12:line

.section .text, "ax"
.align 4
.code 32
.global argb8888toyuv420
.global ayuv444toyuv420

ayuv444toyuv420:
	stmfd r13!,{r4-r11,r14}
	pld [r0,#0]
	pld [r1,#0]
	sub r13,r13,#8
	str r3,[r13] @hght
	mov r9,r3 @r9=hght
	mul r4,r2,r3
	str r2,[r13,#4] @wdth
	add r5,r4,r4,asr #2
	add r2,r0,r4 @u_pos
	add r3,r0,r5 @v_pos
Loop_1:
	ldr r12,[r13,#4] @wdth
	ands r11,r9,#1
	bne Loop_3
Loop_2:
	pld [r0,#8]
	pld [r1,#32]
	ldmia r1!,{r4-r7}
	mov r8,r4,lsr #8
	mov r10,r4,lsr #16
	strb r10,[r0],#1
	strb r8,[r2],#1
	strb r4,[r3],#1
	mov r8,r5,lsr #16
	strb r8,[r0],#1
	mov r8,r6,lsr #8
	mov r10,r6,lsr #16
	strb r10,[r0],#1
	strb r8,[r2],#1
	strb r6,[r3],#1
	mov r8,r7,lsr #16
	strb r8,[r0],#1
	
	ldmia r1!,{r4-r7}
	mov r8,r4,lsr #8
	mov r10,r4,lsr #16
	strb r10,[r0],#1
	strb r8,[r2],#1
	strb r4,[r3],#1
	mov r8,r5,lsr #16
	strb r8,[r0],#1
	mov r8,r6,lsr #8
	mov r10,r6,lsr #16
	strb r10,[r0],#1
	strb r8,[r2],#1
	strb r6,[r3],#1
	mov r8,r7,lsr #16
	strb r8,[r0],#1

	subs r12,r12,#8
	bne Loop_2

	subs r9,r9,#1
	bgt Loop_1
	add r13,r13,#8

	ldmfd r13!,{r4-r11,r15}

Loop_3:
	pld [r0,#8]
	pld [r1,#32]
	ldmia r1!,{r4-r7}
	mov r4,r4,lsr #16
	mov r5,r5,lsr #16
	mov r6,r6,lsr #16
	mov r7,r7,lsr #16
	strb r4,[r0],#1
	strb r5,[r0],#1
	strb r6,[r0],#1
	strb r7,[r0],#1

	ldmia r1!,{r4-r7}
	mov r4,r4,lsr #16
	mov r5,r5,lsr #16
	mov r6,r6,lsr #16
	mov r7,r7,lsr #16
	strb r4,[r0],#1
	strb r5,[r0],#1
	strb r6,[r0],#1
	strb r7,[r0],#1

	subs r12,r12,#8
	bne Loop_3

	subs r9,r9,#1
	bgt Loop_1
	add r13,r13,#8

	ldmfd r13!,{r4-r11,r15}

argb8888toyuv420:
	stmfd r13!,{r4-r11,r14}
	sub r13,r13,#16
	str r3,[r13] @hght
	mul r4,r2,r3
	str r2,[r13,#4] @width
  @mrc p15,0,r3,c0,c0,1
  @bic r3,r3,#0x00038000
  @orr r3,r3,#0x00033000
  @mcr p15,0,r3,c0,c0,1
	add r5,r4,r4,asr #2
	add r2,r0,r4 @upos
	add r3,r0,r5 @vpos
	mov r4,#0
	str r4,[r13,#12] @line=0
	ldr r4,=0xffa2ffee @{-94,-18}
	ldr r5,=0x00ff00ff
	ldr r7,=0x00190042 @{25,66}
	ldr r9,=0xffda0081 @{-38,129}
	ldr r14,=0xffb60070 @{-74,112}
L1:
	ldr r6,[r13,#12] @r6=line
	ldr r12,[r13,#4] @r12=width
	ands r6,r6,#1
	bne L3
L2:
  @pld [r0,#32]
  @pld [r1,#32]
  @pld [r2,#32]
  @pld [r3,#32]
	ldr r6,[r1],#4 @r6={a0,r0,g0,b0}
	mov r8,#4096
	and r10,r5,r6,lsr #8 @r10={0,a0,0,g0}
	and r6,r6,r5 @r6={0,r0,0,b0}
	smlabt r8,r6,r7,r8
	smlatb r8,r6,r7,r8
	smlabb r8,r9,r10,r8
	mov r11,#0x8000
	mov r8,r8,asr #8
	strb r8,[r0],#1

	smlatt r11,r6,r9,r11 @r0*-38
	smlabb r11,r6,r14,r11 @+=b0*112
	smlabt r11,r10,r14,r11 @+=g0*-74
	mov r8,#0x8000
	mov r11,r11,asr #8
	strb r11,[r2],#1
	smlatb r8,r6,r14,r8 @r0*112
	smlabb r8,r4,r6,r8 @+=b0*-18
	smlatb r8,r4,r10,r8 @+=g0*-94
	mov r11,#4096
	mov r8,r8,asr #8
	ldr r6,[r1],#4 @r6={a1,r1,g1,b1}
	strb r8,[r3],#1
	and r10,r5,r6,lsr #8
	and r6,r6,r5 @r6={0,r1,0,b1}
	smlabt r11,r6,r7,r11
	smlatb r11,r6,r7,r11
	smlabb r11,r9,r10,r11
  ldr r6,[r1],#4 @r6={a0,r0,g0,b0}
	mov r11,r11,asr #8
	strb r11,[r0],#1

	mov r8,#4096
	and r10,r5,r6,lsr #8 @r10={0,a0,0,g0}
	and r6,r6,r5 @r6={0,r0,0,b0}
	smlabt r8,r6,r7,r8
	smlatb r8,r6,r7,r8
	smlabb r8,r9,r10,r8
	mov r11,#0x8000
	mov r8,r8,asr #8
	strb r8,[r0],#1

	smlatt r11,r6,r9,r11 @r0*-38
	smlabb r11,r6,r14,r11 @+=b0*112
	smlabt r11,r10,r14,r11 @+=g0*-74
	mov r8,#0x8000
	mov r11,r11,asr #8
	strb r11,[r2],#1
	smlatb r8,r6,r14,r8 @r0*112
	smlabb r8,r4,r6,r8 @+=b0*-18
	smlatb r8,r4,r10,r8 @+=g0*-94
	mov r11,#4096
	mov r8,r8,asr #8
	ldr r6,[r1],#4 @r6={a1,r1,g1,b1}
	strb r8,[r3],#1
	and r10,r5,r6,lsr #8
	and r6,r6,r5 @r6={0,r1,0,b1}
	smlabt r11,r6,r7,r11
	smlatb r11,r6,r7,r11
	smlabb r11,r9,r10,r11
  ldr r6,[r1],#4 @r6={a0,r0,g0,b0}
	mov r11,r11,asr #8
	strb r11,[r0],#1

	mov r8,#4096
	and r10,r5,r6,lsr #8 @r10={0,a0,0,g0}
	and r6,r6,r5 @r6={0,r0,0,b0}
	smlabt r8,r6,r7,r8
	smlatb r8,r6,r7,r8
	smlabb r8,r9,r10,r8
	mov r11,#0x8000
	mov r8,r8,asr #8
	strb r8,[r0],#1

	smlatt r11,r6,r9,r11 @r0*-38
	smlabb r11,r6,r14,r11 @+=b0*112
	smlabt r11,r10,r14,r11 @+=g0*-74
	mov r8,#0x8000
	mov r11,r11,asr #8
	strb r11,[r2],#1
	smlatb r8,r6,r14,r8 @r0*112
	smlabb r8,r4,r6,r8 @+=b0*-18
	smlatb r8,r4,r10,r8 @+=g0*-94
	mov r11,#4096
	mov r8,r8,asr #8
	ldr r6,[r1],#4 @r6={a1,r1,g1,b1}
	strb r8,[r3],#1
	and r10,r5,r6,lsr #8
	and r6,r6,r5 @r6={0,r1,0,b1}
	smlabt r11,r6,r7,r11
	smlatb r11,r6,r7,r11
	smlabb r11,r9,r10,r11
  ldr r6,[r1],#4 @r6={a0,r0,g0,b0}
	mov r11,r11,asr #8
	strb r11,[r0],#1

	mov r8,#4096
	and r10,r5,r6,lsr #8 @r10={0,a0,0,g0}
	and r6,r6,r5 @r6={0,r0,0,b0}
	smlabt r8,r6,r7,r8
	smlatb r8,r6,r7,r8
	smlabb r8,r9,r10,r8
	mov r11,#0x8000
	mov r8,r8,asr #8
	strb r8,[r0],#1

	smlatt r11,r6,r9,r11 @r0*-38
	smlabb r11,r6,r14,r11 @+=b0*112
	smlabt r11,r10,r14,r11 @+=g0*-74
	mov r8,#0x8000
	mov r11,r11,asr #8
	strb r11,[r2],#1
	smlatb r8,r6,r14,r8 @r0*112
	smlabb r8,r4,r6,r8 @+=b0*-18
	smlatb r8,r4,r10,r8 @+=g0*-94
	mov r11,#4096
	mov r8,r8,asr #8
	ldr r6,[r1],#4 @r6={a1,r1,g1,b1}
	strb r8,[r3],#1
	and r10,r5,r6,lsr #8
	and r6,r6,r5 @r6={0,r1,0,b1}
	smlabt r11,r6,r7,r11
	smlatb r11,r6,r7,r11
	smlabb r11,r9,r10,r11
  subs r12,r12,#8
	mov r11,r11,asr #8
	strb r11,[r0],#1
	bgt L2
	
	ldr r6,[r13,#12] @line
	ldr r8,[r13] @hght
	add r6,r6,#1
	str r6,[r13,#12]
	cmp r6,r8
	blt L1
	add r13,r13,#16
	ldmfd r13!,{r4-r11,r15}
L3:
  @pld [r0,#32]
  @pld [r1,#32]
	ldr r6,[r1],#4
	mov r8,#4096
	and r10,r5,r6,lsr #8 @r10={0,a0,0,g0}
	and r6,r6,r5 @r6={0,r0,0,b0}
	smlabt r8,r6,r7,r8
	smlatb r8,r6,r7,r8
	smlabb r8,r9,r10,r8
  ldr r6,[r1],#4
	mov r8,r8,asr #8
	strb r8,[r0],#1
  and r10,r5,r6,lsr #8
  mov r8,#4096
  and r6,r6,r5
  smlabt r8,r6,r7,r8
	smlatb r8,r6,r7,r8
	smlabb r8,r9,r10,r8
  ldr r6,[r1],#4
  mov r8,r8,asr #8
  strb r8,[r0],#1
  and r10,r5,r6,lsr #8
  mov r8,#4096
  and r6,r6,r5
  smlabt r8,r6,r7,r8
	smlatb r8,r6,r7,r8
	smlabb r8,r9,r10,r8
  ldr r6,[r1],#4
  mov r8,r8,asr #8
  strb r8,[r0],#1
  and r10,r5,r6,lsr #8
  mov r8,#4096
  and r6,r6,r5
  smlabt r8,r6,r7,r8
	smlatb r8,r6,r7,r8
	smlabb r8,r9,r10,r8
  ldr r6,[r1],#4
  mov r8,r8,asr #8
  strb r8,[r0],#1
  and r10,r5,r6,lsr #8
  mov r8,#4096
  and r6,r6,r5
  smlabt r8,r6,r7,r8
	smlatb r8,r6,r7,r8
	smlabb r8,r9,r10,r8
  ldr r6,[r1],#4
  mov r8,r8,asr #8
  strb r8,[r0],#1
  and r10,r5,r6,lsr #8
  mov r8,#4096
  and r6,r6,r5
  smlabt r8,r6,r7,r8
	smlatb r8,r6,r7,r8
	smlabb r8,r9,r10,r8
  ldr r6,[r1],#4
  mov r8,r8,asr #8
  strb r8,[r0],#1
  and r10,r5,r6,lsr #8
  mov r8,#4096
  and r6,r6,r5
  smlabt r8,r6,r7,r8
	smlatb r8,r6,r7,r8
	smlabb r8,r9,r10,r8
  ldr r6,[r1],#4
  mov r8,r8,asr #8
  strb r8,[r0],#1
  and r10,r5,r6,lsr #8
  mov r8,#4096
  and r6,r6,r5
  smlabt r8,r6,r7,r8
	smlatb r8,r6,r7,r8
	smlabb r8,r9,r10,r8
  subs r12,r12,#8
  mov r8,r8,asr #8
  strb r8,[r0],#1
	bgt L3

	ldr r6,[r13,#12] @line
	ldr r8,[r13] @hght
	add r6,r6,#1
	str r6,[r13,#12]
	cmp r6,r8
	blt L1
	add r13,r13,#16
	ldmfd r13!,{r4-r11,r15}
.end
