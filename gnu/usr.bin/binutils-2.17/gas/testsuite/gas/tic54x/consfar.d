#as: -mcpu=548 -mfar-mode
#objdump: -d -r
#name: c54x cons tests, w/extended addressing
#source: cons.s

.*: +file format .*c54x.*

Disassembly of section .text:

0+000 <binary>:
   0:	0003.*
   1:	0004.*

0+002 <octal>:
   2:	0009.*
   3:	000a.*
   4:	000b.*

0+005 <hex>:
   5:	000f.*
   6:	0010.*

0+007 <field>:
   7:	6440.*
   8:	0123.*
   9:	4000.*
   a:	0000.*
   b:	1234.*

0+00c <byte>:
   c:	00aa.*
   d:	00bb.*

0+00e <word>:
   e:	0ccc.*

0+00f <xlong>:
   f:	0eee.*
  10:	efff.*
	...

0+012 <long>:
  12:	eeee.*
  13:	ffff.*

0+014 <int>:
  14:	dddd.*

0+015 <xfloat>:
  15:	3fff.*
  16:	ffac.*
	...

0+018 <float>:
  18:	3fff.*
  19:	ffac.*

0+01a <string>:
  1a:	0061.*
  1b:	0062.*
  1c:	0063.*
  1d:	0064.*
  1e:	0061.*
  1f:	0062.*
  20:	0063.*
  21:	0064.*
  22:	0065.*
  23:	0066.*
  24:	0067.*
  25:	0030.*

0+026 <pstring>:
  26:	6162.*
  27:	6364.*
  28:	6162.*
  29:	6364.*
  2a:	6566.*
  2b:	6700.*

0+02c <DAT1>:
  2c:	0000.*
  2d:	abcd.*
  2e:	0000.*
  2f:	0141.*
  30:	0000.*
  31:	0067.*
  32:	0000.*
  33:	006f.*

0+034 <xlong.0>:
  34:	0000.*
.*34: ARELEXT.*
  35:	002c.*
  36:	aabb.*
  37:	ccdd.*

0+038 <DAT2>:
  38:	0000.*
	...

0+03a <DAT3>:
  3a:	1234.*
  3b:	5678.*
  3c:	0000.*
  3d:	aabb.*
  3e:	ccdd.*
