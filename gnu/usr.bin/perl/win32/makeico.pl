binmode STDOUT;
while (<DATA>) {
  chomp;
  print pack "H*", $_;
}

# Create new hex data with
# perl -wle "binmode STDIN; $/ = \32; while (<>) {print unpack 'H*', $_}" <perl.ico.orig
# then place after __DATA__
__DATA__
0000010003001010100001000400280100003600000010100000010008006805
00005e010000101000000100200068040000c606000028000000100000002000
00000100040000000000c000000000000000000000000000000000000000ffff
ff007b000000007b00007b7b000000007b007b007b00007b7b00bdbdbd007b7b
7b00ff00000000ff0000ffff00000000ff00ff00ff0000ffff0000000000ffff
fffffffffffffffffffff7ff8fffffffffffff8fffffffffffffff7fffffffff
ffffff8fffffffffffffff8fffffffffffffff8f7ffffffffffffffff87fffff
fffffffffffffffff8fffffff8ffffffff8ffffffffffffffff8ffff7fffff8f
f8ff8ffffffffffffffff8ffffffffffffffffffffffffffffffffffffffffff
9e5cfbb77420fd9b7865fd9b2074fd5b7320fd5b6e20fd137573f0017072e003
6c65e0032077e4076e20e6076577c30f6720ff9f6520ffff6f6effff6e202800
0000100000002000000001000800000000004001000000000000000000000000
000000000000ffffff00fefefe00d6d6d600aaaaaa00fdfdfd00797979000000
00007a7a7a00fcfcfc004141410075757500848484001e1e1e00cbcbcb00b7b7
b70003030300888888000202020081818100f6f6f60020202000b6b6b6008a8a
8a00040404001a1a1a00e7e7e7000e0e0e00383838006b6b6b0018181800b2b2
b200c1c1c10015151600171717000b0b0b00010101001f1f1f000a0a0a007070
70009b9b9b00e9e9e80078787800111111002323230044444400e2e2e200a6a6
a600656565006a6a6a00b4b4b400afafaf00cfcfcf00080808006c6c6c008f8f
8f00b1b1b100bfbfbf00f1f1f100585858009a9a9a00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffffff00ffff
ff00ffffff00060606060606060606060606060606060606060606060606060d
06063a060606060606060606060606061c060606060606060606060606060606
030606060606060606060606060606061c060606060606060606060606060606
1c060606060606060606060606060c14090605060606060606060c0c06060606
061406052706060606060606060606060606060c060606060606061b06060606
0606061c06060606060614060b06060606060c060606060606060606060b0606
06060b06060606060906060506060b06060c0606060606060606060606060605
0606060606060606060606060606060606060606060606060606060606060606
060606060606ffff9e5cfbb77420fd9b7865fd9b2074fd5b7320fd5b6e20fd13
7573f0017072e0036c65e0032077e4076e20e6076577c30f6720ff9f6520ffff
6f6effff6e202800000010000000200000000100200000000000400400000000
0000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000
0000000000ff00000070000000010000004ebfbfbf400000000e000000005858
58a7000000650000000000000000000000000000000000000000000000000000
000000000030000000ff0000000100000001080808f76c6c6c93000000000000
0001000000ff0000000000000000000000000000000000000000000000000000
000000000003000000ff000000010000004b000000ffafafaf50000000000000
0001000000ff0000000100000000000000000000000000000000000000000000
000000000001000000ff00000001000000ff000000006a6a6a95000000010000
0001000000ff0000000100000000000000000000000000000000000000000000
000000000001000000ff00000001000000ff000000596565659a000000010000
0001000000ff0000000000000000000000000000000000000000000000000000
000000000017000000ff00000087111111ee232323dc444444bb000000017a7a
7a85000000ff0000001d000000000000000000000000000000010000003e1515
16ea171717e8020202fd0b0b0bf4010101fe000000ff020202fd1f1f1fe00a0a
0af57070708f9b9b9b6400000000000000000000000000000001000000ff0000
00ff000000ff000000ff000000ff000000ff000000ff000000ff000000ff0000
00ff181818e70000004d00000000000000000000000000000001000000ff0e0e
0ef1383838c7000000ff000000ff000000ff000000ff000000ff000000ff0000
00ff6b6b6b940000000000000000000000000000000000000000000000ff2020
20df000000498a8a8a75040404fb000000ff000000ff000000ff000000ff1a1a
1ae5000000180000000000000000000000000000000000000048000000ff0303
03fc0000008a0000000188888877020202fd000000ff000000ff000000ff8181
817e0000000900000000000000000000000000000000414141be000000ff0000
00ff7575758a00000000000000008484847b000000ff000000ff1e1e1ee10000
0034000000000000000000000000000000000000000000000001000000290000
00550000000100000000000000000000000279797986000000ff000000850000
0003000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000ffff9e5cfbb77420fd9b7865fd9b2074fd5b
7320fd5b6e20fd137573f0017072e0036c65e0032077e4076e20e6076577c30f
6720ff9f6520ffff6f6effff6e20
