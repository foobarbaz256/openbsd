#as:
#objdump: -dr
#name: i860 pfsm

.*: +file format .*

Disassembly of section \.text:

00000000 <\.text>:
   0:	10 04 22 48 	r2s1.ss	%f0,%f1,%f2
   4:	90 1c 85 48 	r2s1.sd	%f3,%f4,%f5
   8:	90 05 44 48 	r2s1.dd	%f0,%f2,%f4
   c:	11 0c 43 48 	r2st.ss	%f1,%f2,%f3
  10:	91 24 a6 48 	r2st.sd	%f4,%f5,%f6
  14:	91 15 86 48 	r2st.dd	%f2,%f4,%f6
  18:	12 14 64 48 	r2as1.ss	%f2,%f3,%f4
  1c:	92 34 e8 48 	r2as1.sd	%f6,%f7,%f8
  20:	92 25 c8 48 	r2as1.dd	%f4,%f6,%f8
  24:	13 1c 85 48 	r2ast.ss	%f3,%f4,%f5
  28:	93 3c 09 49 	r2ast.sd	%f7,%f8,%f9
  2c:	93 35 0a 49 	r2ast.dd	%f6,%f8,%f10
  30:	14 24 a6 48 	i2s1.ss	%f4,%f5,%f6
  34:	94 44 2a 49 	i2s1.sd	%f8,%f9,%f10
  38:	94 65 d0 49 	i2s1.dd	%f12,%f14,%f16
  3c:	15 3c 09 49 	i2st.ss	%f7,%f8,%f9
  40:	95 5c 8d 49 	i2st.sd	%f11,%f12,%f13
  44:	95 75 12 4a 	i2st.dd	%f14,%f16,%f18
  48:	16 54 6c 49 	i2as1.ss	%f10,%f11,%f12
  4c:	96 74 f0 49 	i2as1.sd	%f14,%f15,%f16
  50:	96 85 54 4a 	i2as1.dd	%f16,%f18,%f20
  54:	17 6c cf 49 	i2ast.ss	%f13,%f14,%f15
  58:	97 8c 53 4a 	i2ast.sd	%f17,%f18,%f19
  5c:	97 95 96 4a 	i2ast.dd	%f18,%f20,%f22
  60:	18 74 f0 49 	rat1s2.ss	%f14,%f15,%f16
  64:	98 a4 b6 4a 	rat1s2.sd	%f20,%f21,%f22
  68:	98 a5 d8 4a 	rat1s2.dd	%f20,%f22,%f24
  6c:	19 7c 11 4a 	m12asm.ss	%f15,%f16,%f17
  70:	99 bc 19 4b 	m12asm.sd	%f23,%f24,%f25
  74:	99 b5 1a 4b 	m12asm.dd	%f22,%f24,%f26
  78:	1a 94 74 4a 	ra1s2.ss	%f18,%f19,%f20
  7c:	9a d4 7c 4b 	ra1s2.sd	%f26,%f27,%f28
  80:	9a a5 d8 4a 	ra1s2.dd	%f20,%f22,%f24
  84:	1b 9c 95 4a 	m12ttsa.ss	%f19,%f20,%f21
  88:	9b ec df 4b 	m12ttsa.sd	%f29,%f30,%f31
  8c:	9b b5 1a 4b 	m12ttsa.dd	%f22,%f24,%f26
  90:	1c a4 b6 4a 	iat1s2.ss	%f20,%f21,%f22
  94:	9c 04 22 48 	iat1s2.sd	%f0,%f1,%f2
  98:	9c c5 5c 4b 	iat1s2.dd	%f24,%f26,%f28
  9c:	1d ac d7 4a 	m12tsm.ss	%f21,%f22,%f23
  a0:	9d 1c 85 48 	m12tsm.sd	%f3,%f4,%f5
  a4:	9d f5 02 48 	m12tsm.dd	%f30,%f0,%f2
  a8:	1e b4 f8 4a 	ia1s2.ss	%f22,%f23,%f24
  ac:	9e 34 e8 48 	ia1s2.sd	%f6,%f7,%f8
  b0:	9e 25 c8 48 	ia1s2.dd	%f4,%f6,%f8
  b4:	1f bc 19 4b 	m12tsa.ss	%f23,%f24,%f25
  b8:	9f 4c 4b 49 	m12tsa.sd	%f9,%f10,%f11
  bc:	9f 35 0a 49 	m12tsa.dd	%f6,%f8,%f10
  c0:	10 06 22 48 	d.r2s1.ss	%f0,%f1,%f2
  c4:	00 00 00 a0 	shl	%r0,%r0,%r0
  c8:	90 1e 85 48 	d.r2s1.sd	%f3,%f4,%f5
  cc:	00 00 00 a0 	shl	%r0,%r0,%r0
  d0:	90 07 44 48 	d.r2s1.dd	%f0,%f2,%f4
  d4:	00 00 00 a0 	shl	%r0,%r0,%r0
  d8:	11 0e 43 48 	d.r2st.ss	%f1,%f2,%f3
  dc:	00 00 00 a0 	shl	%r0,%r0,%r0
  e0:	91 26 a6 48 	d.r2st.sd	%f4,%f5,%f6
  e4:	00 00 00 a0 	shl	%r0,%r0,%r0
  e8:	91 17 86 48 	d.r2st.dd	%f2,%f4,%f6
  ec:	00 00 00 a0 	shl	%r0,%r0,%r0
  f0:	12 16 64 48 	d.r2as1.ss	%f2,%f3,%f4
  f4:	00 00 00 a0 	shl	%r0,%r0,%r0
  f8:	92 36 e8 48 	d.r2as1.sd	%f6,%f7,%f8
  fc:	00 00 00 a0 	shl	%r0,%r0,%r0
 100:	92 27 c8 48 	d.r2as1.dd	%f4,%f6,%f8
 104:	00 00 00 a0 	shl	%r0,%r0,%r0
 108:	13 1e 85 48 	d.r2ast.ss	%f3,%f4,%f5
 10c:	00 00 00 a0 	shl	%r0,%r0,%r0
 110:	93 3e 09 49 	d.r2ast.sd	%f7,%f8,%f9
 114:	00 00 00 a0 	shl	%r0,%r0,%r0
 118:	93 37 0a 49 	d.r2ast.dd	%f6,%f8,%f10
 11c:	00 00 00 a0 	shl	%r0,%r0,%r0
 120:	14 26 a6 48 	d.i2s1.ss	%f4,%f5,%f6
 124:	00 00 00 a0 	shl	%r0,%r0,%r0
 128:	94 46 2a 49 	d.i2s1.sd	%f8,%f9,%f10
 12c:	00 00 00 a0 	shl	%r0,%r0,%r0
 130:	94 67 d0 49 	d.i2s1.dd	%f12,%f14,%f16
 134:	00 00 00 a0 	shl	%r0,%r0,%r0
 138:	15 3e 09 49 	d.i2st.ss	%f7,%f8,%f9
 13c:	00 00 00 a0 	shl	%r0,%r0,%r0
 140:	95 5e 8d 49 	d.i2st.sd	%f11,%f12,%f13
 144:	00 00 00 a0 	shl	%r0,%r0,%r0
 148:	95 77 12 4a 	d.i2st.dd	%f14,%f16,%f18
 14c:	00 00 00 a0 	shl	%r0,%r0,%r0
 150:	16 56 6c 49 	d.i2as1.ss	%f10,%f11,%f12
 154:	00 00 00 a0 	shl	%r0,%r0,%r0
 158:	96 76 f0 49 	d.i2as1.sd	%f14,%f15,%f16
 15c:	00 00 00 a0 	shl	%r0,%r0,%r0
 160:	96 87 54 4a 	d.i2as1.dd	%f16,%f18,%f20
 164:	00 00 00 a0 	shl	%r0,%r0,%r0
 168:	17 6e cf 49 	d.i2ast.ss	%f13,%f14,%f15
 16c:	00 00 00 a0 	shl	%r0,%r0,%r0
 170:	97 8e 53 4a 	d.i2ast.sd	%f17,%f18,%f19
 174:	00 00 00 a0 	shl	%r0,%r0,%r0
 178:	97 97 96 4a 	d.i2ast.dd	%f18,%f20,%f22
 17c:	00 00 00 a0 	shl	%r0,%r0,%r0
 180:	18 76 f0 49 	d.rat1s2.ss	%f14,%f15,%f16
 184:	00 00 00 a0 	shl	%r0,%r0,%r0
 188:	98 a6 b6 4a 	d.rat1s2.sd	%f20,%f21,%f22
 18c:	00 00 00 a0 	shl	%r0,%r0,%r0
 190:	98 a7 d8 4a 	d.rat1s2.dd	%f20,%f22,%f24
 194:	00 00 00 a0 	shl	%r0,%r0,%r0
 198:	19 7e 11 4a 	d.m12asm.ss	%f15,%f16,%f17
 19c:	00 00 00 a0 	shl	%r0,%r0,%r0
 1a0:	99 be 19 4b 	d.m12asm.sd	%f23,%f24,%f25
 1a4:	00 00 00 a0 	shl	%r0,%r0,%r0
 1a8:	99 b7 1a 4b 	d.m12asm.dd	%f22,%f24,%f26
 1ac:	00 00 00 a0 	shl	%r0,%r0,%r0
 1b0:	1a 96 74 4a 	d.ra1s2.ss	%f18,%f19,%f20
 1b4:	00 00 00 a0 	shl	%r0,%r0,%r0
 1b8:	9a d6 7c 4b 	d.ra1s2.sd	%f26,%f27,%f28
 1bc:	00 00 00 a0 	shl	%r0,%r0,%r0
 1c0:	9a a7 d8 4a 	d.ra1s2.dd	%f20,%f22,%f24
 1c4:	00 00 00 a0 	shl	%r0,%r0,%r0
 1c8:	1b 9e 95 4a 	d.m12ttsa.ss	%f19,%f20,%f21
 1cc:	00 00 00 a0 	shl	%r0,%r0,%r0
 1d0:	9b ee df 4b 	d.m12ttsa.sd	%f29,%f30,%f31
 1d4:	00 00 00 a0 	shl	%r0,%r0,%r0
 1d8:	9b b7 1a 4b 	d.m12ttsa.dd	%f22,%f24,%f26
 1dc:	00 00 00 a0 	shl	%r0,%r0,%r0
 1e0:	1c a6 b6 4a 	d.iat1s2.ss	%f20,%f21,%f22
 1e4:	00 00 00 a0 	shl	%r0,%r0,%r0
 1e8:	9c 06 22 48 	d.iat1s2.sd	%f0,%f1,%f2
 1ec:	00 00 00 a0 	shl	%r0,%r0,%r0
 1f0:	9c c7 5c 4b 	d.iat1s2.dd	%f24,%f26,%f28
 1f4:	00 00 00 a0 	shl	%r0,%r0,%r0
 1f8:	1d ae d7 4a 	d.m12tsm.ss	%f21,%f22,%f23
 1fc:	00 00 00 a0 	shl	%r0,%r0,%r0
 200:	9d 1e 85 48 	d.m12tsm.sd	%f3,%f4,%f5
 204:	00 00 00 a0 	shl	%r0,%r0,%r0
 208:	9d f7 02 48 	d.m12tsm.dd	%f30,%f0,%f2
 20c:	00 00 00 a0 	shl	%r0,%r0,%r0
 210:	1e b6 f8 4a 	d.ia1s2.ss	%f22,%f23,%f24
 214:	00 00 00 a0 	shl	%r0,%r0,%r0
 218:	9e 36 e8 48 	d.ia1s2.sd	%f6,%f7,%f8
 21c:	00 00 00 a0 	shl	%r0,%r0,%r0
 220:	9e 27 c8 48 	d.ia1s2.dd	%f4,%f6,%f8
 224:	00 00 00 a0 	shl	%r0,%r0,%r0
 228:	1f be 19 4b 	d.m12tsa.ss	%f23,%f24,%f25
 22c:	00 00 00 a0 	shl	%r0,%r0,%r0
 230:	9f 4e 4b 49 	d.m12tsa.sd	%f9,%f10,%f11
 234:	00 00 00 a0 	shl	%r0,%r0,%r0
 238:	9f 37 0a 49 	d.m12tsa.dd	%f6,%f8,%f10
 23c:	00 00 00 a0 	shl	%r0,%r0,%r0
