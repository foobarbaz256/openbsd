/*      $OpenBSD: cast.c,v 1.2 1998/07/21 22:42:03 provos Exp $       */
/*
 *	CAST-128 in C
 *	Written by Steve Reid <sreid@sea-to-sky.net>
 *	100% Public Domain - no warranty
 *	Released 1997.10.11
 */

#include <sys/types.h>

#include <cast.h>

/* CAST S-Boxes */

static const u_int32_t cast_sbox1[256] = {
	0x30FB40D4, 0x9FA0FF0B, 0x6BECCD2F, 0x3F258C7A,
	0x1E213F2F, 0x9C004DD3, 0x6003E540, 0xCF9FC949,
	0xBFD4AF27, 0x88BBBDB5, 0xE2034090, 0x98D09675,
	0x6E63A0E0, 0x15C361D2, 0xC2E7661D, 0x22D4FF8E,
	0x28683B6F, 0xC07FD059, 0xFF2379C8, 0x775F50E2,
	0x43C340D3, 0xDF2F8656, 0x887CA41A, 0xA2D2BD2D,
	0xA1C9E0D6, 0x346C4819, 0x61B76D87, 0x22540F2F,
	0x2ABE32E1, 0xAA54166B, 0x22568E3A, 0xA2D341D0,
	0x66DB40C8, 0xA784392F, 0x004DFF2F, 0x2DB9D2DE,
	0x97943FAC, 0x4A97C1D8, 0x527644B7, 0xB5F437A7,
	0xB82CBAEF, 0xD751D159, 0x6FF7F0ED, 0x5A097A1F,
	0x827B68D0, 0x90ECF52E, 0x22B0C054, 0xBC8E5935,
	0x4B6D2F7F, 0x50BB64A2, 0xD2664910, 0xBEE5812D,
	0xB7332290, 0xE93B159F, 0xB48EE411, 0x4BFF345D,
	0xFD45C240, 0xAD31973F, 0xC4F6D02E, 0x55FC8165,
	0xD5B1CAAD, 0xA1AC2DAE, 0xA2D4B76D, 0xC19B0C50,
	0x882240F2, 0x0C6E4F38, 0xA4E4BFD7, 0x4F5BA272,
	0x564C1D2F, 0xC59C5319, 0xB949E354, 0xB04669FE,
	0xB1B6AB8A, 0xC71358DD, 0x6385C545, 0x110F935D,
	0x57538AD5, 0x6A390493, 0xE63D37E0, 0x2A54F6B3,
	0x3A787D5F, 0x6276A0B5, 0x19A6FCDF, 0x7A42206A,
	0x29F9D4D5, 0xF61B1891, 0xBB72275E, 0xAA508167,
	0x38901091, 0xC6B505EB, 0x84C7CB8C, 0x2AD75A0F,
	0x874A1427, 0xA2D1936B, 0x2AD286AF, 0xAA56D291,
	0xD7894360, 0x425C750D, 0x93B39E26, 0x187184C9,
	0x6C00B32D, 0x73E2BB14, 0xA0BEBC3C, 0x54623779,
	0x64459EAB, 0x3F328B82, 0x7718CF82, 0x59A2CEA6,
	0x04EE002E, 0x89FE78E6, 0x3FAB0950, 0x325FF6C2,
	0x81383F05, 0x6963C5C8, 0x76CB5AD6, 0xD49974C9,
	0xCA180DCF, 0x380782D5, 0xC7FA5CF6, 0x8AC31511,
	0x35E79E13, 0x47DA91D0, 0xF40F9086, 0xA7E2419E,
	0x31366241, 0x051EF495, 0xAA573B04, 0x4A805D8D,
	0x548300D0, 0x00322A3C, 0xBF64CDDF, 0xBA57A68E,
	0x75C6372B, 0x50AFD341, 0xA7C13275, 0x915A0BF5,
	0x6B54BFAB, 0x2B0B1426, 0xAB4CC9D7, 0x449CCD82,
	0xF7FBF265, 0xAB85C5F3, 0x1B55DB94, 0xAAD4E324,
	0xCFA4BD3F, 0x2DEAA3E2, 0x9E204D02, 0xC8BD25AC,
	0xEADF55B3, 0xD5BD9E98, 0xE31231B2, 0x2AD5AD6C,
	0x954329DE, 0xADBE4528, 0xD8710F69, 0xAA51C90F,
	0xAA786BF6, 0x22513F1E, 0xAA51A79B, 0x2AD344CC,
	0x7B5A41F0, 0xD37CFBAD, 0x1B069505, 0x41ECE491,
	0xB4C332E6, 0x032268D4, 0xC9600ACC, 0xCE387E6D,
	0xBF6BB16C, 0x6A70FB78, 0x0D03D9C9, 0xD4DF39DE,
	0xE01063DA, 0x4736F464, 0x5AD328D8, 0xB347CC96,
	0x75BB0FC3, 0x98511BFB, 0x4FFBCC35, 0xB58BCF6A,
	0xE11F0ABC, 0xBFC5FE4A, 0xA70AEC10, 0xAC39570A,
	0x3F04442F, 0x6188B153, 0xE0397A2E, 0x5727CB79,
	0x9CEB418F, 0x1CACD68D, 0x2AD37C96, 0x0175CB9D,
	0xC69DFF09, 0xC75B65F0, 0xD9DB40D8, 0xEC0E7779,
	0x4744EAD4, 0xB11C3274, 0xDD24CB9E, 0x7E1C54BD,
	0xF01144F9, 0xD2240EB1, 0x9675B3FD, 0xA3AC3755,
	0xD47C27AF, 0x51C85F4D, 0x56907596, 0xA5BB15E6,
	0x580304F0, 0xCA042CF1, 0x011A37EA, 0x8DBFAADB,
	0x35BA3E4A, 0x3526FFA0, 0xC37B4D09, 0xBC306ED9,
	0x98A52666, 0x5648F725, 0xFF5E569D, 0x0CED63D0,
	0x7C63B2CF, 0x700B45E1, 0xD5EA50F1, 0x85A92872,
	0xAF1FBDA7, 0xD4234870, 0xA7870BF3, 0x2D3B4D79,
	0x42E04198, 0x0CD0EDE7, 0x26470DB8, 0xF881814C,
	0x474D6AD7, 0x7C0C5E5C, 0xD1231959, 0x381B7298,
	0xF5D2F4DB, 0xAB838653, 0x6E2F1E23, 0x83719C9E,
	0xBD91E046, 0x9A56456E, 0xDC39200C, 0x20C8C571,
	0x962BDA1C, 0xE1E696FF, 0xB141AB08, 0x7CCA89B9,
	0x1A69E783, 0x02CC4843, 0xA2F7C579, 0x429EF47D,
	0x427B169C, 0x5AC9F049, 0xDD8F0F00, 0x5C8165BF
};

static const u_int32_t cast_sbox2[256] = {
	0x1F201094, 0xEF0BA75B, 0x69E3CF7E, 0x393F4380,
	0xFE61CF7A, 0xEEC5207A, 0x55889C94, 0x72FC0651,
	0xADA7EF79, 0x4E1D7235, 0xD55A63CE, 0xDE0436BA,
	0x99C430EF, 0x5F0C0794, 0x18DCDB7D, 0xA1D6EFF3,
	0xA0B52F7B, 0x59E83605, 0xEE15B094, 0xE9FFD909,
	0xDC440086, 0xEF944459, 0xBA83CCB3, 0xE0C3CDFB,
	0xD1DA4181, 0x3B092AB1, 0xF997F1C1, 0xA5E6CF7B,
	0x01420DDB, 0xE4E7EF5B, 0x25A1FF41, 0xE180F806,
	0x1FC41080, 0x179BEE7A, 0xD37AC6A9, 0xFE5830A4,
	0x98DE8B7F, 0x77E83F4E, 0x79929269, 0x24FA9F7B,
	0xE113C85B, 0xACC40083, 0xD7503525, 0xF7EA615F,
	0x62143154, 0x0D554B63, 0x5D681121, 0xC866C359,
	0x3D63CF73, 0xCEE234C0, 0xD4D87E87, 0x5C672B21,
	0x071F6181, 0x39F7627F, 0x361E3084, 0xE4EB573B,
	0x602F64A4, 0xD63ACD9C, 0x1BBC4635, 0x9E81032D,
	0x2701F50C, 0x99847AB4, 0xA0E3DF79, 0xBA6CF38C,
	0x10843094, 0x2537A95E, 0xF46F6FFE, 0xA1FF3B1F,
	0x208CFB6A, 0x8F458C74, 0xD9E0A227, 0x4EC73A34,
	0xFC884F69, 0x3E4DE8DF, 0xEF0E0088, 0x3559648D,
	0x8A45388C, 0x1D804366, 0x721D9BFD, 0xA58684BB,
	0xE8256333, 0x844E8212, 0x128D8098, 0xFED33FB4,
	0xCE280AE1, 0x27E19BA5, 0xD5A6C252, 0xE49754BD,
	0xC5D655DD, 0xEB667064, 0x77840B4D, 0xA1B6A801,
	0x84DB26A9, 0xE0B56714, 0x21F043B7, 0xE5D05860,
	0x54F03084, 0x066FF472, 0xA31AA153, 0xDADC4755,
	0xB5625DBF, 0x68561BE6, 0x83CA6B94, 0x2D6ED23B,
	0xECCF01DB, 0xA6D3D0BA, 0xB6803D5C, 0xAF77A709,
	0x33B4A34C, 0x397BC8D6, 0x5EE22B95, 0x5F0E5304,
	0x81ED6F61, 0x20E74364, 0xB45E1378, 0xDE18639B,
	0x881CA122, 0xB96726D1, 0x8049A7E8, 0x22B7DA7B,
	0x5E552D25, 0x5272D237, 0x79D2951C, 0xC60D894C,
	0x488CB402, 0x1BA4FE5B, 0xA4B09F6B, 0x1CA815CF,
	0xA20C3005, 0x8871DF63, 0xB9DE2FCB, 0x0CC6C9E9,
	0x0BEEFF53, 0xE3214517, 0xB4542835, 0x9F63293C,
	0xEE41E729, 0x6E1D2D7C, 0x50045286, 0x1E6685F3,
	0xF33401C6, 0x30A22C95, 0x31A70850, 0x60930F13,
	0x73F98417, 0xA1269859, 0xEC645C44, 0x52C877A9,
	0xCDFF33A6, 0xA02B1741, 0x7CBAD9A2, 0x2180036F,
	0x50D99C08, 0xCB3F4861, 0xC26BD765, 0x64A3F6AB,
	0x80342676, 0x25A75E7B, 0xE4E6D1FC, 0x20C710E6,
	0xCDF0B680, 0x17844D3B, 0x31EEF84D, 0x7E0824E4,
	0x2CCB49EB, 0x846A3BAE, 0x8FF77888, 0xEE5D60F6,
	0x7AF75673, 0x2FDD5CDB, 0xA11631C1, 0x30F66F43,
	0xB3FAEC54, 0x157FD7FA, 0xEF8579CC, 0xD152DE58,
	0xDB2FFD5E, 0x8F32CE19, 0x306AF97A, 0x02F03EF8,
	0x99319AD5, 0xC242FA0F, 0xA7E3EBB0, 0xC68E4906,
	0xB8DA230C, 0x80823028, 0xDCDEF3C8, 0xD35FB171,
	0x088A1BC8, 0xBEC0C560, 0x61A3C9E8, 0xBCA8F54D,
	0xC72FEFFA, 0x22822E99, 0x82C570B4, 0xD8D94E89,
	0x8B1C34BC, 0x301E16E6, 0x273BE979, 0xB0FFEAA6,
	0x61D9B8C6, 0x00B24869, 0xB7FFCE3F, 0x08DC283B,
	0x43DAF65A, 0xF7E19798, 0x7619B72F, 0x8F1C9BA4,
	0xDC8637A0, 0x16A7D3B1, 0x9FC393B7, 0xA7136EEB,
	0xC6BCC63E, 0x1A513742, 0xEF6828BC, 0x520365D6,
	0x2D6A77AB, 0x3527ED4B, 0x821FD216, 0x095C6E2E,
	0xDB92F2FB, 0x5EEA29CB, 0x145892F5, 0x91584F7F,
	0x5483697B, 0x2667A8CC, 0x85196048, 0x8C4BACEA,
	0x833860D4, 0x0D23E0F9, 0x6C387E8A, 0x0AE6D249,
	0xB284600C, 0xD835731D, 0xDCB1C647, 0xAC4C56EA,
	0x3EBD81B3, 0x230EABB0, 0x6438BC87, 0xF0B5B1FA,
	0x8F5EA2B3, 0xFC184642, 0x0A036B7A, 0x4FB089BD,
	0x649DA589, 0xA345415E, 0x5C038323, 0x3E5D3BB9,
	0x43D79572, 0x7E6DD07C, 0x06DFDF1E, 0x6C6CC4EF,
	0x7160A539, 0x73BFBE70, 0x83877605, 0x4523ECF1
};

static const u_int32_t cast_sbox3[256] = {
	0x8DEFC240, 0x25FA5D9F, 0xEB903DBF, 0xE810C907,
	0x47607FFF, 0x369FE44B, 0x8C1FC644, 0xAECECA90,
	0xBEB1F9BF, 0xEEFBCAEA, 0xE8CF1950, 0x51DF07AE,
	0x920E8806, 0xF0AD0548, 0xE13C8D83, 0x927010D5,
	0x11107D9F, 0x07647DB9, 0xB2E3E4D4, 0x3D4F285E,
	0xB9AFA820, 0xFADE82E0, 0xA067268B, 0x8272792E,
	0x553FB2C0, 0x489AE22B, 0xD4EF9794, 0x125E3FBC,
	0x21FFFCEE, 0x825B1BFD, 0x9255C5ED, 0x1257A240,
	0x4E1A8302, 0xBAE07FFF, 0x528246E7, 0x8E57140E,
	0x3373F7BF, 0x8C9F8188, 0xA6FC4EE8, 0xC982B5A5,
	0xA8C01DB7, 0x579FC264, 0x67094F31, 0xF2BD3F5F,
	0x40FFF7C1, 0x1FB78DFC, 0x8E6BD2C1, 0x437BE59B,
	0x99B03DBF, 0xB5DBC64B, 0x638DC0E6, 0x55819D99,
	0xA197C81C, 0x4A012D6E, 0xC5884A28, 0xCCC36F71,
	0xB843C213, 0x6C0743F1, 0x8309893C, 0x0FEDDD5F,
	0x2F7FE850, 0xD7C07F7E, 0x02507FBF, 0x5AFB9A04,
	0xA747D2D0, 0x1651192E, 0xAF70BF3E, 0x58C31380,
	0x5F98302E, 0x727CC3C4, 0x0A0FB402, 0x0F7FEF82,
	0x8C96FDAD, 0x5D2C2AAE, 0x8EE99A49, 0x50DA88B8,
	0x8427F4A0, 0x1EAC5790, 0x796FB449, 0x8252DC15,
	0xEFBD7D9B, 0xA672597D, 0xADA840D8, 0x45F54504,
	0xFA5D7403, 0xE83EC305, 0x4F91751A, 0x925669C2,
	0x23EFE941, 0xA903F12E, 0x60270DF2, 0x0276E4B6,
	0x94FD6574, 0x927985B2, 0x8276DBCB, 0x02778176,
	0xF8AF918D, 0x4E48F79E, 0x8F616DDF, 0xE29D840E,
	0x842F7D83, 0x340CE5C8, 0x96BBB682, 0x93B4B148,
	0xEF303CAB, 0x984FAF28, 0x779FAF9B, 0x92DC560D,
	0x224D1E20, 0x8437AA88, 0x7D29DC96, 0x2756D3DC,
	0x8B907CEE, 0xB51FD240, 0xE7C07CE3, 0xE566B4A1,
	0xC3E9615E, 0x3CF8209D, 0x6094D1E3, 0xCD9CA341,
	0x5C76460E, 0x00EA983B, 0xD4D67881, 0xFD47572C,
	0xF76CEDD9, 0xBDA8229C, 0x127DADAA, 0x438A074E,
	0x1F97C090, 0x081BDB8A, 0x93A07EBE, 0xB938CA15,
	0x97B03CFF, 0x3DC2C0F8, 0x8D1AB2EC, 0x64380E51,
	0x68CC7BFB, 0xD90F2788, 0x12490181, 0x5DE5FFD4,
	0xDD7EF86A, 0x76A2E214, 0xB9A40368, 0x925D958F,
	0x4B39FFFA, 0xBA39AEE9, 0xA4FFD30B, 0xFAF7933B,
	0x6D498623, 0x193CBCFA, 0x27627545, 0x825CF47A,
	0x61BD8BA0, 0xD11E42D1, 0xCEAD04F4, 0x127EA392,
	0x10428DB7, 0x8272A972, 0x9270C4A8, 0x127DE50B,
	0x285BA1C8, 0x3C62F44F, 0x35C0EAA5, 0xE805D231,
	0x428929FB, 0xB4FCDF82, 0x4FB66A53, 0x0E7DC15B,
	0x1F081FAB, 0x108618AE, 0xFCFD086D, 0xF9FF2889,
	0x694BCC11, 0x236A5CAE, 0x12DECA4D, 0x2C3F8CC5,
	0xD2D02DFE, 0xF8EF5896, 0xE4CF52DA, 0x95155B67,
	0x494A488C, 0xB9B6A80C, 0x5C8F82BC, 0x89D36B45,
	0x3A609437, 0xEC00C9A9, 0x44715253, 0x0A874B49,
	0xD773BC40, 0x7C34671C, 0x02717EF6, 0x4FEB5536,
	0xA2D02FFF, 0xD2BF60C4, 0xD43F03C0, 0x50B4EF6D,
	0x07478CD1, 0x006E1888, 0xA2E53F55, 0xB9E6D4BC,
	0xA2048016, 0x97573833, 0xD7207D67, 0xDE0F8F3D,
	0x72F87B33, 0xABCC4F33, 0x7688C55D, 0x7B00A6B0,
	0x947B0001, 0x570075D2, 0xF9BB88F8, 0x8942019E,
	0x4264A5FF, 0x856302E0, 0x72DBD92B, 0xEE971B69,
	0x6EA22FDE, 0x5F08AE2B, 0xAF7A616D, 0xE5C98767,
	0xCF1FEBD2, 0x61EFC8C2, 0xF1AC2571, 0xCC8239C2,
	0x67214CB8, 0xB1E583D1, 0xB7DC3E62, 0x7F10BDCE,
	0xF90A5C38, 0x0FF0443D, 0x606E6DC6, 0x60543A49,
	0x5727C148, 0x2BE98A1D, 0x8AB41738, 0x20E1BE24,
	0xAF96DA0F, 0x68458425, 0x99833BE5, 0x600D457D,
	0x282F9350, 0x8334B362, 0xD91D1120, 0x2B6D8DA0,
	0x642B1E31, 0x9C305A00, 0x52BCE688, 0x1B03588A,
	0xF7BAEFD5, 0x4142ED9C, 0xA4315C11, 0x83323EC5,
	0xDFEF4636, 0xA133C501, 0xE9D3531C, 0xEE353783
};

static const u_int32_t cast_sbox4[256] = {
	0x9DB30420, 0x1FB6E9DE, 0xA7BE7BEF, 0xD273A298,
	0x4A4F7BDB, 0x64AD8C57, 0x85510443, 0xFA020ED1,
	0x7E287AFF, 0xE60FB663, 0x095F35A1, 0x79EBF120,
	0xFD059D43, 0x6497B7B1, 0xF3641F63, 0x241E4ADF,
	0x28147F5F, 0x4FA2B8CD, 0xC9430040, 0x0CC32220,
	0xFDD30B30, 0xC0A5374F, 0x1D2D00D9, 0x24147B15,
	0xEE4D111A, 0x0FCA5167, 0x71FF904C, 0x2D195FFE,
	0x1A05645F, 0x0C13FEFE, 0x081B08CA, 0x05170121,
	0x80530100, 0xE83E5EFE, 0xAC9AF4F8, 0x7FE72701,
	0xD2B8EE5F, 0x06DF4261, 0xBB9E9B8A, 0x7293EA25,
	0xCE84FFDF, 0xF5718801, 0x3DD64B04, 0xA26F263B,
	0x7ED48400, 0x547EEBE6, 0x446D4CA0, 0x6CF3D6F5,
	0x2649ABDF, 0xAEA0C7F5, 0x36338CC1, 0x503F7E93,
	0xD3772061, 0x11B638E1, 0x72500E03, 0xF80EB2BB,
	0xABE0502E, 0xEC8D77DE, 0x57971E81, 0xE14F6746,
	0xC9335400, 0x6920318F, 0x081DBB99, 0xFFC304A5,
	0x4D351805, 0x7F3D5CE3, 0xA6C866C6, 0x5D5BCCA9,
	0xDAEC6FEA, 0x9F926F91, 0x9F46222F, 0x3991467D,
	0xA5BF6D8E, 0x1143C44F, 0x43958302, 0xD0214EEB,
	0x022083B8, 0x3FB6180C, 0x18F8931E, 0x281658E6,
	0x26486E3E, 0x8BD78A70, 0x7477E4C1, 0xB506E07C,
	0xF32D0A25, 0x79098B02, 0xE4EABB81, 0x28123B23,
	0x69DEAD38, 0x1574CA16, 0xDF871B62, 0x211C40B7,
	0xA51A9EF9, 0x0014377B, 0x041E8AC8, 0x09114003,
	0xBD59E4D2, 0xE3D156D5, 0x4FE876D5, 0x2F91A340,
	0x557BE8DE, 0x00EAE4A7, 0x0CE5C2EC, 0x4DB4BBA6,
	0xE756BDFF, 0xDD3369AC, 0xEC17B035, 0x06572327,
	0x99AFC8B0, 0x56C8C391, 0x6B65811C, 0x5E146119,
	0x6E85CB75, 0xBE07C002, 0xC2325577, 0x893FF4EC,
	0x5BBFC92D, 0xD0EC3B25, 0xB7801AB7, 0x8D6D3B24,
	0x20C763EF, 0xC366A5FC, 0x9C382880, 0x0ACE3205,
	0xAAC9548A, 0xECA1D7C7, 0x041AFA32, 0x1D16625A,
	0x6701902C, 0x9B757A54, 0x31D477F7, 0x9126B031,
	0x36CC6FDB, 0xC70B8B46, 0xD9E66A48, 0x56E55A79,
	0x026A4CEB, 0x52437EFF, 0x2F8F76B4, 0x0DF980A5,
	0x8674CDE3, 0xEDDA04EB, 0x17A9BE04, 0x2C18F4DF,
	0xB7747F9D, 0xAB2AF7B4, 0xEFC34D20, 0x2E096B7C,
	0x1741A254, 0xE5B6A035, 0x213D42F6, 0x2C1C7C26,
	0x61C2F50F, 0x6552DAF9, 0xD2C231F8, 0x25130F69,
	0xD8167FA2, 0x0418F2C8, 0x001A96A6, 0x0D1526AB,
	0x63315C21, 0x5E0A72EC, 0x49BAFEFD, 0x187908D9,
	0x8D0DBD86, 0x311170A7, 0x3E9B640C, 0xCC3E10D7,
	0xD5CAD3B6, 0x0CAEC388, 0xF73001E1, 0x6C728AFF,
	0x71EAE2A1, 0x1F9AF36E, 0xCFCBD12F, 0xC1DE8417,
	0xAC07BE6B, 0xCB44A1D8, 0x8B9B0F56, 0x013988C3,
	0xB1C52FCA, 0xB4BE31CD, 0xD8782806, 0x12A3A4E2,
	0x6F7DE532, 0x58FD7EB6, 0xD01EE900, 0x24ADFFC2,
	0xF4990FC5, 0x9711AAC5, 0x001D7B95, 0x82E5E7D2,
	0x109873F6, 0x00613096, 0xC32D9521, 0xADA121FF,
	0x29908415, 0x7FBB977F, 0xAF9EB3DB, 0x29C9ED2A,
	0x5CE2A465, 0xA730F32C, 0xD0AA3FE8, 0x8A5CC091,
	0xD49E2CE7, 0x0CE454A9, 0xD60ACD86, 0x015F1919,
	0x77079103, 0xDEA03AF6, 0x78A8565E, 0xDEE356DF,
	0x21F05CBE, 0x8B75E387, 0xB3C50651, 0xB8A5C3EF,
	0xD8EEB6D2, 0xE523BE77, 0xC2154529, 0x2F69EFDF,
	0xAFE67AFB, 0xF470C4B2, 0xF3E0EB5B, 0xD6CC9876,
	0x39E4460C, 0x1FDA8538, 0x1987832F, 0xCA007367,
	0xA99144F8, 0x296B299E, 0x492FC295, 0x9266BEAB,
	0xB5676E69, 0x9BD3DDDA, 0xDF7E052F, 0xDB25701C,
	0x1B5E51EE, 0xF65324E6, 0x6AFCE36C, 0x0316CC04,
	0x8644213E, 0xB7DC59D0, 0x7965291F, 0xCCD6FD43,
	0x41823979, 0x932BCDF6, 0xB657C34D, 0x4EDFD282,
	0x7AE5290C, 0x3CB9536B, 0x851E20FE, 0x9833557E,
	0x13ECF0B0, 0xD3FFB372, 0x3F85C5C1, 0x0AEF7ED2
};

static const u_int32_t cast_sbox5[256] = {
	0x7EC90C04, 0x2C6E74B9, 0x9B0E66DF, 0xA6337911,
	0xB86A7FFF, 0x1DD358F5, 0x44DD9D44, 0x1731167F,
	0x08FBF1FA, 0xE7F511CC, 0xD2051B00, 0x735ABA00,
	0x2AB722D8, 0x386381CB, 0xACF6243A, 0x69BEFD7A,
	0xE6A2E77F, 0xF0C720CD, 0xC4494816, 0xCCF5C180,
	0x38851640, 0x15B0A848, 0xE68B18CB, 0x4CAADEFF,
	0x5F480A01, 0x0412B2AA, 0x259814FC, 0x41D0EFE2,
	0x4E40B48D, 0x248EB6FB, 0x8DBA1CFE, 0x41A99B02,
	0x1A550A04, 0xBA8F65CB, 0x7251F4E7, 0x95A51725,
	0xC106ECD7, 0x97A5980A, 0xC539B9AA, 0x4D79FE6A,
	0xF2F3F763, 0x68AF8040, 0xED0C9E56, 0x11B4958B,
	0xE1EB5A88, 0x8709E6B0, 0xD7E07156, 0x4E29FEA7,
	0x6366E52D, 0x02D1C000, 0xC4AC8E05, 0x9377F571,
	0x0C05372A, 0x578535F2, 0x2261BE02, 0xD642A0C9,
	0xDF13A280, 0x74B55BD2, 0x682199C0, 0xD421E5EC,
	0x53FB3CE8, 0xC8ADEDB3, 0x28A87FC9, 0x3D959981,
	0x5C1FF900, 0xFE38D399, 0x0C4EFF0B, 0x062407EA,
	0xAA2F4FB1, 0x4FB96976, 0x90C79505, 0xB0A8A774,
	0xEF55A1FF, 0xE59CA2C2, 0xA6B62D27, 0xE66A4263,
	0xDF65001F, 0x0EC50966, 0xDFDD55BC, 0x29DE0655,
	0x911E739A, 0x17AF8975, 0x32C7911C, 0x89F89468,
	0x0D01E980, 0x524755F4, 0x03B63CC9, 0x0CC844B2,
	0xBCF3F0AA, 0x87AC36E9, 0xE53A7426, 0x01B3D82B,
	0x1A9E7449, 0x64EE2D7E, 0xCDDBB1DA, 0x01C94910,
	0xB868BF80, 0x0D26F3FD, 0x9342EDE7, 0x04A5C284,
	0x636737B6, 0x50F5B616, 0xF24766E3, 0x8ECA36C1,
	0x136E05DB, 0xFEF18391, 0xFB887A37, 0xD6E7F7D4,
	0xC7FB7DC9, 0x3063FCDF, 0xB6F589DE, 0xEC2941DA,
	0x26E46695, 0xB7566419, 0xF654EFC5, 0xD08D58B7,
	0x48925401, 0xC1BACB7F, 0xE5FF550F, 0xB6083049,
	0x5BB5D0E8, 0x87D72E5A, 0xAB6A6EE1, 0x223A66CE,
	0xC62BF3CD, 0x9E0885F9, 0x68CB3E47, 0x086C010F,
	0xA21DE820, 0xD18B69DE, 0xF3F65777, 0xFA02C3F6,
	0x407EDAC3, 0xCBB3D550, 0x1793084D, 0xB0D70EBA,
	0x0AB378D5, 0xD951FB0C, 0xDED7DA56, 0x4124BBE4,
	0x94CA0B56, 0x0F5755D1, 0xE0E1E56E, 0x6184B5BE,
	0x580A249F, 0x94F74BC0, 0xE327888E, 0x9F7B5561,
	0xC3DC0280, 0x05687715, 0x646C6BD7, 0x44904DB3,
	0x66B4F0A3, 0xC0F1648A, 0x697ED5AF, 0x49E92FF6,
	0x309E374F, 0x2CB6356A, 0x85808573, 0x4991F840,
	0x76F0AE02, 0x083BE84D, 0x28421C9A, 0x44489406,
	0x736E4CB8, 0xC1092910, 0x8BC95FC6, 0x7D869CF4,
	0x134F616F, 0x2E77118D, 0xB31B2BE1, 0xAA90B472,
	0x3CA5D717, 0x7D161BBA, 0x9CAD9010, 0xAF462BA2,
	0x9FE459D2, 0x45D34559, 0xD9F2DA13, 0xDBC65487,
	0xF3E4F94E, 0x176D486F, 0x097C13EA, 0x631DA5C7,
	0x445F7382, 0x175683F4, 0xCDC66A97, 0x70BE0288,
	0xB3CDCF72, 0x6E5DD2F3, 0x20936079, 0x459B80A5,
	0xBE60E2DB, 0xA9C23101, 0xEBA5315C, 0x224E42F2,
	0x1C5C1572, 0xF6721B2C, 0x1AD2FFF3, 0x8C25404E,
	0x324ED72F, 0x4067B7FD, 0x0523138E, 0x5CA3BC78,
	0xDC0FD66E, 0x75922283, 0x784D6B17, 0x58EBB16E,
	0x44094F85, 0x3F481D87, 0xFCFEAE7B, 0x77B5FF76,
	0x8C2302BF, 0xAAF47556, 0x5F46B02A, 0x2B092801,
	0x3D38F5F7, 0x0CA81F36, 0x52AF4A8A, 0x66D5E7C0,
	0xDF3B0874, 0x95055110, 0x1B5AD7A8, 0xF61ED5AD,
	0x6CF6E479, 0x20758184, 0xD0CEFA65, 0x88F7BE58,
	0x4A046826, 0x0FF6F8F3, 0xA09C7F70, 0x5346ABA0,
	0x5CE96C28, 0xE176EDA3, 0x6BAC307F, 0x376829D2,
	0x85360FA9, 0x17E3FE2A, 0x24B79767, 0xF5A96B20,
	0xD6CD2595, 0x68FF1EBF, 0x7555442C, 0xF19F06BE,
	0xF9E0659A, 0xEEB9491D, 0x34010718, 0xBB30CAB8,
	0xE822FE15, 0x88570983, 0x750E6249, 0xDA627E55,
	0x5E76FFA8, 0xB1534546, 0x6D47DE08, 0xEFE9E7D4
};

static const u_int32_t cast_sbox6[256] = {
	0xF6FA8F9D, 0x2CAC6CE1, 0x4CA34867, 0xE2337F7C,
	0x95DB08E7, 0x016843B4, 0xECED5CBC, 0x325553AC,
	0xBF9F0960, 0xDFA1E2ED, 0x83F0579D, 0x63ED86B9,
	0x1AB6A6B8, 0xDE5EBE39, 0xF38FF732, 0x8989B138,
	0x33F14961, 0xC01937BD, 0xF506C6DA, 0xE4625E7E,
	0xA308EA99, 0x4E23E33C, 0x79CBD7CC, 0x48A14367,
	0xA3149619, 0xFEC94BD5, 0xA114174A, 0xEAA01866,
	0xA084DB2D, 0x09A8486F, 0xA888614A, 0x2900AF98,
	0x01665991, 0xE1992863, 0xC8F30C60, 0x2E78EF3C,
	0xD0D51932, 0xCF0FEC14, 0xF7CA07D2, 0xD0A82072,
	0xFD41197E, 0x9305A6B0, 0xE86BE3DA, 0x74BED3CD,
	0x372DA53C, 0x4C7F4448, 0xDAB5D440, 0x6DBA0EC3,
	0x083919A7, 0x9FBAEED9, 0x49DBCFB0, 0x4E670C53,
	0x5C3D9C01, 0x64BDB941, 0x2C0E636A, 0xBA7DD9CD,
	0xEA6F7388, 0xE70BC762, 0x35F29ADB, 0x5C4CDD8D,
	0xF0D48D8C, 0xB88153E2, 0x08A19866, 0x1AE2EAC8,
	0x284CAF89, 0xAA928223, 0x9334BE53, 0x3B3A21BF,
	0x16434BE3, 0x9AEA3906, 0xEFE8C36E, 0xF890CDD9,
	0x80226DAE, 0xC340A4A3, 0xDF7E9C09, 0xA694A807,
	0x5B7C5ECC, 0x221DB3A6, 0x9A69A02F, 0x68818A54,
	0xCEB2296F, 0x53C0843A, 0xFE893655, 0x25BFE68A,
	0xB4628ABC, 0xCF222EBF, 0x25AC6F48, 0xA9A99387,
	0x53BDDB65, 0xE76FFBE7, 0xE967FD78, 0x0BA93563,
	0x8E342BC1, 0xE8A11BE9, 0x4980740D, 0xC8087DFC,
	0x8DE4BF99, 0xA11101A0, 0x7FD37975, 0xDA5A26C0,
	0xE81F994F, 0x9528CD89, 0xFD339FED, 0xB87834BF,
	0x5F04456D, 0x22258698, 0xC9C4C83B, 0x2DC156BE,
	0x4F628DAA, 0x57F55EC5, 0xE2220ABE, 0xD2916EBF,
	0x4EC75B95, 0x24F2C3C0, 0x42D15D99, 0xCD0D7FA0,
	0x7B6E27FF, 0xA8DC8AF0, 0x7345C106, 0xF41E232F,
	0x35162386, 0xE6EA8926, 0x3333B094, 0x157EC6F2,
	0x372B74AF, 0x692573E4, 0xE9A9D848, 0xF3160289,
	0x3A62EF1D, 0xA787E238, 0xF3A5F676, 0x74364853,
	0x20951063, 0x4576698D, 0xB6FAD407, 0x592AF950,
	0x36F73523, 0x4CFB6E87, 0x7DA4CEC0, 0x6C152DAA,
	0xCB0396A8, 0xC50DFE5D, 0xFCD707AB, 0x0921C42F,
	0x89DFF0BB, 0x5FE2BE78, 0x448F4F33, 0x754613C9,
	0x2B05D08D, 0x48B9D585, 0xDC049441, 0xC8098F9B,
	0x7DEDE786, 0xC39A3373, 0x42410005, 0x6A091751,
	0x0EF3C8A6, 0x890072D6, 0x28207682, 0xA9A9F7BE,
	0xBF32679D, 0xD45B5B75, 0xB353FD00, 0xCBB0E358,
	0x830F220A, 0x1F8FB214, 0xD372CF08, 0xCC3C4A13,
	0x8CF63166, 0x061C87BE, 0x88C98F88, 0x6062E397,
	0x47CF8E7A, 0xB6C85283, 0x3CC2ACFB, 0x3FC06976,
	0x4E8F0252, 0x64D8314D, 0xDA3870E3, 0x1E665459,
	0xC10908F0, 0x513021A5, 0x6C5B68B7, 0x822F8AA0,
	0x3007CD3E, 0x74719EEF, 0xDC872681, 0x073340D4,
	0x7E432FD9, 0x0C5EC241, 0x8809286C, 0xF592D891,
	0x08A930F6, 0x957EF305, 0xB7FBFFBD, 0xC266E96F,
	0x6FE4AC98, 0xB173ECC0, 0xBC60B42A, 0x953498DA,
	0xFBA1AE12, 0x2D4BD736, 0x0F25FAAB, 0xA4F3FCEB,
	0xE2969123, 0x257F0C3D, 0x9348AF49, 0x361400BC,
	0xE8816F4A, 0x3814F200, 0xA3F94043, 0x9C7A54C2,
	0xBC704F57, 0xDA41E7F9, 0xC25AD33A, 0x54F4A084,
	0xB17F5505, 0x59357CBE, 0xEDBD15C8, 0x7F97C5AB,
	0xBA5AC7B5, 0xB6F6DEAF, 0x3A479C3A, 0x5302DA25,
	0x653D7E6A, 0x54268D49, 0x51A477EA, 0x5017D55B,
	0xD7D25D88, 0x44136C76, 0x0404A8C8, 0xB8E5A121,
	0xB81A928A, 0x60ED5869, 0x97C55B96, 0xEAEC991B,
	0x29935913, 0x01FDB7F1, 0x088E8DFA, 0x9AB6F6F5,
	0x3B4CBF9F, 0x4A5DE3AB, 0xE6051D35, 0xA0E1D855,
	0xD36B4CF1, 0xF544EDEB, 0xB0E93524, 0xBEBB8FBD,
	0xA2D762CF, 0x49C92F54, 0x38B5F331, 0x7128A454,
	0x48392905, 0xA65B1DB8, 0x851C97BD, 0xD675CF2F
};

static const u_int32_t cast_sbox7[256] = {
	0x85E04019, 0x332BF567, 0x662DBFFF, 0xCFC65693,
	0x2A8D7F6F, 0xAB9BC912, 0xDE6008A1, 0x2028DA1F,
	0x0227BCE7, 0x4D642916, 0x18FAC300, 0x50F18B82,
	0x2CB2CB11, 0xB232E75C, 0x4B3695F2, 0xB28707DE,
	0xA05FBCF6, 0xCD4181E9, 0xE150210C, 0xE24EF1BD,
	0xB168C381, 0xFDE4E789, 0x5C79B0D8, 0x1E8BFD43,
	0x4D495001, 0x38BE4341, 0x913CEE1D, 0x92A79C3F,
	0x089766BE, 0xBAEEADF4, 0x1286BECF, 0xB6EACB19,
	0x2660C200, 0x7565BDE4, 0x64241F7A, 0x8248DCA9,
	0xC3B3AD66, 0x28136086, 0x0BD8DFA8, 0x356D1CF2,
	0x107789BE, 0xB3B2E9CE, 0x0502AA8F, 0x0BC0351E,
	0x166BF52A, 0xEB12FF82, 0xE3486911, 0xD34D7516,
	0x4E7B3AFF, 0x5F43671B, 0x9CF6E037, 0x4981AC83,
	0x334266CE, 0x8C9341B7, 0xD0D854C0, 0xCB3A6C88,
	0x47BC2829, 0x4725BA37, 0xA66AD22B, 0x7AD61F1E,
	0x0C5CBAFA, 0x4437F107, 0xB6E79962, 0x42D2D816,
	0x0A961288, 0xE1A5C06E, 0x13749E67, 0x72FC081A,
	0xB1D139F7, 0xF9583745, 0xCF19DF58, 0xBEC3F756,
	0xC06EBA30, 0x07211B24, 0x45C28829, 0xC95E317F,
	0xBC8EC511, 0x38BC46E9, 0xC6E6FA14, 0xBAE8584A,
	0xAD4EBC46, 0x468F508B, 0x7829435F, 0xF124183B,
	0x821DBA9F, 0xAFF60FF4, 0xEA2C4E6D, 0x16E39264,
	0x92544A8B, 0x009B4FC3, 0xABA68CED, 0x9AC96F78,
	0x06A5B79A, 0xB2856E6E, 0x1AEC3CA9, 0xBE838688,
	0x0E0804E9, 0x55F1BE56, 0xE7E5363B, 0xB3A1F25D,
	0xF7DEBB85, 0x61FE033C, 0x16746233, 0x3C034C28,
	0xDA6D0C74, 0x79AAC56C, 0x3CE4E1AD, 0x51F0C802,
	0x98F8F35A, 0x1626A49F, 0xEED82B29, 0x1D382FE3,
	0x0C4FB99A, 0xBB325778, 0x3EC6D97B, 0x6E77A6A9,
	0xCB658B5C, 0xD45230C7, 0x2BD1408B, 0x60C03EB7,
	0xB9068D78, 0xA33754F4, 0xF430C87D, 0xC8A71302,
	0xB96D8C32, 0xEBD4E7BE, 0xBE8B9D2D, 0x7979FB06,
	0xE7225308, 0x8B75CF77, 0x11EF8DA4, 0xE083C858,
	0x8D6B786F, 0x5A6317A6, 0xFA5CF7A0, 0x5DDA0033,
	0xF28EBFB0, 0xF5B9C310, 0xA0EAC280, 0x08B9767A,
	0xA3D9D2B0, 0x79D34217, 0x021A718D, 0x9AC6336A,
	0x2711FD60, 0x438050E3, 0x069908A8, 0x3D7FEDC4,
	0x826D2BEF, 0x4EEB8476, 0x488DCF25, 0x36C9D566,
	0x28E74E41, 0xC2610ACA, 0x3D49A9CF, 0xBAE3B9DF,
	0xB65F8DE6, 0x92AEAF64, 0x3AC7D5E6, 0x9EA80509,
	0xF22B017D, 0xA4173F70, 0xDD1E16C3, 0x15E0D7F9,
	0x50B1B887, 0x2B9F4FD5, 0x625ABA82, 0x6A017962,
	0x2EC01B9C, 0x15488AA9, 0xD716E740, 0x40055A2C,
	0x93D29A22, 0xE32DBF9A, 0x058745B9, 0x3453DC1E,
	0xD699296E, 0x496CFF6F, 0x1C9F4986, 0xDFE2ED07,
	0xB87242D1, 0x19DE7EAE, 0x053E561A, 0x15AD6F8C,
	0x66626C1C, 0x7154C24C, 0xEA082B2A, 0x93EB2939,
	0x17DCB0F0, 0x58D4F2AE, 0x9EA294FB, 0x52CF564C,
	0x9883FE66, 0x2EC40581, 0x763953C3, 0x01D6692E,
	0xD3A0C108, 0xA1E7160E, 0xE4F2DFA6, 0x693ED285,
	0x74904698, 0x4C2B0EDD, 0x4F757656, 0x5D393378,
	0xA132234F, 0x3D321C5D, 0xC3F5E194, 0x4B269301,
	0xC79F022F, 0x3C997E7E, 0x5E4F9504, 0x3FFAFBBD,
	0x76F7AD0E, 0x296693F4, 0x3D1FCE6F, 0xC61E45BE,
	0xD3B5AB34, 0xF72BF9B7, 0x1B0434C0, 0x4E72B567,
	0x5592A33D, 0xB5229301, 0xCFD2A87F, 0x60AEB767,
	0x1814386B, 0x30BCC33D, 0x38A0C07D, 0xFD1606F2,
	0xC363519B, 0x589DD390, 0x5479F8E6, 0x1CB8D647,
	0x97FD61A9, 0xEA7759F4, 0x2D57539D, 0x569A58CF,
	0xE84E63AD, 0x462E1B78, 0x6580F87E, 0xF3817914,
	0x91DA55F4, 0x40A230F3, 0xD1988F35, 0xB6E318D2,
	0x3FFA50BC, 0x3D40F021, 0xC3C0BDAE, 0x4958C24C,
	0x518F36B2, 0x84B1D370, 0x0FEDCE83, 0x878DDADA,
	0xF2A279C7, 0x94E01BE8, 0x90716F4B, 0x954B8AA3
};

static const u_int32_t cast_sbox8[256] = {
	0xE216300D, 0xBBDDFFFC, 0xA7EBDABD, 0x35648095,
	0x7789F8B7, 0xE6C1121B, 0x0E241600, 0x052CE8B5,
	0x11A9CFB0, 0xE5952F11, 0xECE7990A, 0x9386D174,
	0x2A42931C, 0x76E38111, 0xB12DEF3A, 0x37DDDDFC,
	0xDE9ADEB1, 0x0A0CC32C, 0xBE197029, 0x84A00940,
	0xBB243A0F, 0xB4D137CF, 0xB44E79F0, 0x049EEDFD,
	0x0B15A15D, 0x480D3168, 0x8BBBDE5A, 0x669DED42,
	0xC7ECE831, 0x3F8F95E7, 0x72DF191B, 0x7580330D,
	0x94074251, 0x5C7DCDFA, 0xABBE6D63, 0xAA402164,
	0xB301D40A, 0x02E7D1CA, 0x53571DAE, 0x7A3182A2,
	0x12A8DDEC, 0xFDAA335D, 0x176F43E8, 0x71FB46D4,
	0x38129022, 0xCE949AD4, 0xB84769AD, 0x965BD862,
	0x82F3D055, 0x66FB9767, 0x15B80B4E, 0x1D5B47A0,
	0x4CFDE06F, 0xC28EC4B8, 0x57E8726E, 0x647A78FC,
	0x99865D44, 0x608BD593, 0x6C200E03, 0x39DC5FF6,
	0x5D0B00A3, 0xAE63AFF2, 0x7E8BD632, 0x70108C0C,
	0xBBD35049, 0x2998DF04, 0x980CF42A, 0x9B6DF491,
	0x9E7EDD53, 0x06918548, 0x58CB7E07, 0x3B74EF2E,
	0x522FFFB1, 0xD24708CC, 0x1C7E27CD, 0xA4EB215B,
	0x3CF1D2E2, 0x19B47A38, 0x424F7618, 0x35856039,
	0x9D17DEE7, 0x27EB35E6, 0xC9AFF67B, 0x36BAF5B8,
	0x09C467CD, 0xC18910B1, 0xE11DBF7B, 0x06CD1AF8,
	0x7170C608, 0x2D5E3354, 0xD4DE495A, 0x64C6D006,
	0xBCC0C62C, 0x3DD00DB3, 0x708F8F34, 0x77D51B42,
	0x264F620F, 0x24B8D2BF, 0x15C1B79E, 0x46A52564,
	0xF8D7E54E, 0x3E378160, 0x7895CDA5, 0x859C15A5,
	0xE6459788, 0xC37BC75F, 0xDB07BA0C, 0x0676A3AB,
	0x7F229B1E, 0x31842E7B, 0x24259FD7, 0xF8BEF472,
	0x835FFCB8, 0x6DF4C1F2, 0x96F5B195, 0xFD0AF0FC,
	0xB0FE134C, 0xE2506D3D, 0x4F9B12EA, 0xF215F225,
	0xA223736F, 0x9FB4C428, 0x25D04979, 0x34C713F8,
	0xC4618187, 0xEA7A6E98, 0x7CD16EFC, 0x1436876C,
	0xF1544107, 0xBEDEEE14, 0x56E9AF27, 0xA04AA441,
	0x3CF7C899, 0x92ECBAE6, 0xDD67016D, 0x151682EB,
	0xA842EEDF, 0xFDBA60B4, 0xF1907B75, 0x20E3030F,
	0x24D8C29E, 0xE139673B, 0xEFA63FB8, 0x71873054,
	0xB6F2CF3B, 0x9F326442, 0xCB15A4CC, 0xB01A4504,
	0xF1E47D8D, 0x844A1BE5, 0xBAE7DFDC, 0x42CBDA70,
	0xCD7DAE0A, 0x57E85B7A, 0xD53F5AF6, 0x20CF4D8C,
	0xCEA4D428, 0x79D130A4, 0x3486EBFB, 0x33D3CDDC,
	0x77853B53, 0x37EFFCB5, 0xC5068778, 0xE580B3E6,
	0x4E68B8F4, 0xC5C8B37E, 0x0D809EA2, 0x398FEB7C,
	0x132A4F94, 0x43B7950E, 0x2FEE7D1C, 0x223613BD,
	0xDD06CAA2, 0x37DF932B, 0xC4248289, 0xACF3EBC3,
	0x5715F6B7, 0xEF3478DD, 0xF267616F, 0xC148CBE4,
	0x9052815E, 0x5E410FAB, 0xB48A2465, 0x2EDA7FA4,
	0xE87B40E4, 0xE98EA084, 0x5889E9E1, 0xEFD390FC,
	0xDD07D35B, 0xDB485694, 0x38D7E5B2, 0x57720101,
	0x730EDEBC, 0x5B643113, 0x94917E4F, 0x503C2FBA,
	0x646F1282, 0x7523D24A, 0xE0779695, 0xF9C17A8F,
	0x7A5B2121, 0xD187B896, 0x29263A4D, 0xBA510CDF,
	0x81F47C9F, 0xAD1163ED, 0xEA7B5965, 0x1A00726E,
	0x11403092, 0x00DA6D77, 0x4A0CDD61, 0xAD1F4603,
	0x605BDFB0, 0x9EEDC364, 0x22EBE6A8, 0xCEE7D28A,
	0xA0E736A0, 0x5564A6B9, 0x10853209, 0xC7EB8F37,
	0x2DE705CA, 0x8951570F, 0xDF09822B, 0xBD691A6C,
	0xAA12E4F2, 0x87451C0F, 0xE0F6A27A, 0x3ADA4819,
	0x4CF1764F, 0x0D771C2B, 0x67CDB156, 0x350D8384,
	0x5938FA0F, 0x42399EF3, 0x36997B07, 0x0E84093D,
	0x4AA93E61, 0x8360D87B, 0x1FA98B0C, 0x1149382C,
	0xE97625A5, 0x0614D1B7, 0x0E25244B, 0x0C768347,
	0x589E8D82, 0x0D2059D1, 0xA466BB1E, 0xF8DA0A82,
	0x04F19130, 0xBA6E4EC0, 0x99265164, 0x1EE7230D,
	0x50B2AD80, 0xEAEE6801, 0x8DB2A283, 0xEA8BF59E
};

/* Macros to access 8-bit bytes out of a 32-bit word */
#define U8a(x) ( (u_int8_t) (x>>24) )
#define U8b(x) ( (u_int8_t) ((x>>16)&255) )
#define U8c(x) ( (u_int8_t) ((x>>8)&255) )
#define U8d(x) ( (u_int8_t) ((x)&255) )

/* Circular left shift */
#define ROL(x, n) ( ((x)<<(n)) | ((x)>>(32-(n))) )

/* CAST-128 uses three different round functions */
#define F1(l, r, i) \
	t = ROL(key->xkey[i] + r, key->xkey[i+16]); \
	l ^= ((cast_sbox1[U8a(t)] ^ cast_sbox2[U8b(t)]) - \
	 cast_sbox3[U8c(t)]) + cast_sbox4[U8d(t)];
#define F2(l, r, i) \
	t = ROL(key->xkey[i] ^ r, key->xkey[i+16]); \
	l ^= ((cast_sbox1[U8a(t)] - cast_sbox2[U8b(t)]) + \
	 cast_sbox3[U8c(t)]) ^ cast_sbox4[U8d(t)];
#define F3(l, r, i) \
	t = ROL(key->xkey[i] - r, key->xkey[i+16]); \
	l ^= ((cast_sbox1[U8a(t)] + cast_sbox2[U8b(t)]) ^ \
	 cast_sbox3[U8c(t)]) - cast_sbox4[U8d(t)];


/***** Encryption Function *****/

void cast_encrypt(cast_key* key, u_int8_t* inblock, u_int8_t* outblock)
{
u_int32_t t, l, r;

	/* Get inblock into l,r */
	l = ((u_int32_t)inblock[0] << 24) | ((u_int32_t)inblock[1] << 16) |
	 ((u_int32_t)inblock[2] << 8) | (u_int32_t)inblock[3];
	r = ((u_int32_t)inblock[4] << 24) | ((u_int32_t)inblock[5] << 16) |
	 ((u_int32_t)inblock[6] << 8) | (u_int32_t)inblock[7];
	/* Do the work */
	F1(l, r,  0);
	F2(r, l,  1);
	F3(l, r,  2);
	F1(r, l,  3);
	F2(l, r,  4);
	F3(r, l,  5);
	F1(l, r,  6);
	F2(r, l,  7);
	F3(l, r,  8);
	F1(r, l,  9);
	F2(l, r, 10);
	F3(r, l, 11);
	/* Only do full 16 rounds if key length > 80 bits */
	if (key->rounds > 12) {
		F1(l, r, 12);
		F2(r, l, 13);
		F3(l, r, 14);
		F1(r, l, 15);
	}
	/* Put l,r into outblock */
	outblock[0] = U8a(r);
	outblock[1] = U8b(r);
	outblock[2] = U8c(r);
	outblock[3] = U8d(r);
	outblock[4] = U8a(l);
	outblock[5] = U8b(l);
	outblock[6] = U8c(l);
	outblock[7] = U8d(l);
	/* Wipe clean */
	t = l = r = 0;
}


/***** Decryption Function *****/

void cast_decrypt(cast_key* key, u_int8_t* inblock, u_int8_t* outblock)
{
u_int32_t t, l, r;

	/* Get inblock into l,r */
	r = ((u_int32_t)inblock[0] << 24) | ((u_int32_t)inblock[1] << 16) |
	 ((u_int32_t)inblock[2] << 8) | (u_int32_t)inblock[3];
	l = ((u_int32_t)inblock[4] << 24) | ((u_int32_t)inblock[5] << 16) |
	 ((u_int32_t)inblock[6] << 8) | (u_int32_t)inblock[7];
	/* Do the work */
	/* Only do full 16 rounds if key length > 80 bits */
	if (key->rounds > 12) {
		F1(r, l, 15);
		F3(l, r, 14);
		F2(r, l, 13);
		F1(l, r, 12);
	}
	F3(r, l, 11);
	F2(l, r, 10);
	F1(r, l,  9);
	F3(l, r,  8);
	F2(r, l,  7);
	F1(l, r,  6);
	F3(r, l,  5);
	F2(l, r,  4);
	F1(r, l,  3);
	F3(l, r,  2);
	F2(r, l,  1);
	F1(l, r,  0);
	/* Put l,r into outblock */
	outblock[0] = U8a(l);
	outblock[1] = U8b(l);
	outblock[2] = U8c(l);
	outblock[3] = U8d(l);
	outblock[4] = U8a(r);
	outblock[5] = U8b(r);
	outblock[6] = U8c(r);
	outblock[7] = U8d(r);
	/* Wipe clean */
	t = l = r = 0;
}


/***** Key Schedual *****/

void cast_setkey(cast_key* key, u_int8_t* rawkey, int keybytes)
{
u_int32_t t[4], z[4], x[4];
int i;

	/* Set number of rounds to 12 or 16, depending on key length */
	key->rounds = (keybytes <= 10 ? 12 : 16);

	/* Copy key to workspace x */
	for (i = 0; i < 4; i++) {
		x[i] = 0;
		if ((i*4+0) < keybytes) x[i] = (u_int32_t)rawkey[i*4+0] << 24;
		if ((i*4+1) < keybytes) x[i] |= (u_int32_t)rawkey[i*4+1] << 16;
		if ((i*4+2) < keybytes) x[i] |= (u_int32_t)rawkey[i*4+2] << 8;
		if ((i*4+3) < keybytes) x[i] |= (u_int32_t)rawkey[i*4+3];
	}
	/* Generate 32 subkeys, four at a time */
	for (i = 0; i < 32; i+=4) {
		switch (i & 4) {
		 case 0:
			t[0] = z[0] = x[0] ^ cast_sbox5[U8b(x[3])] ^
			 cast_sbox6[U8d(x[3])] ^ cast_sbox7[U8a(x[3])] ^
			 cast_sbox8[U8c(x[3])] ^ cast_sbox7[U8a(x[2])];
			t[1] = z[1] = x[2] ^ cast_sbox5[U8a(z[0])] ^
			 cast_sbox6[U8c(z[0])] ^ cast_sbox7[U8b(z[0])] ^
			 cast_sbox8[U8d(z[0])] ^ cast_sbox8[U8c(x[2])];
			t[2] = z[2] = x[3] ^ cast_sbox5[U8d(z[1])] ^
			 cast_sbox6[U8c(z[1])] ^ cast_sbox7[U8b(z[1])] ^
			 cast_sbox8[U8a(z[1])] ^ cast_sbox5[U8b(x[2])];
			t[3] = z[3] = x[1] ^ cast_sbox5[U8c(z[2])] ^
			 cast_sbox6[U8b(z[2])] ^ cast_sbox7[U8d(z[2])] ^
			 cast_sbox8[U8a(z[2])] ^ cast_sbox6[U8d(x[2])];
			break;
		 case 4:
			t[0] = x[0] = z[2] ^ cast_sbox5[U8b(z[1])] ^
			 cast_sbox6[U8d(z[1])] ^ cast_sbox7[U8a(z[1])] ^
			 cast_sbox8[U8c(z[1])] ^ cast_sbox7[U8a(z[0])];
			t[1] = x[1] = z[0] ^ cast_sbox5[U8a(x[0])] ^
			 cast_sbox6[U8c(x[0])] ^ cast_sbox7[U8b(x[0])] ^
			 cast_sbox8[U8d(x[0])] ^ cast_sbox8[U8c(z[0])];
			t[2] = x[2] = z[1] ^ cast_sbox5[U8d(x[1])] ^
			 cast_sbox6[U8c(x[1])] ^ cast_sbox7[U8b(x[1])] ^
			 cast_sbox8[U8a(x[1])] ^ cast_sbox5[U8b(z[0])];
			t[3] = x[3] = z[3] ^ cast_sbox5[U8c(x[2])] ^
			 cast_sbox6[U8b(x[2])] ^ cast_sbox7[U8d(x[2])] ^
			 cast_sbox8[U8a(x[2])] ^ cast_sbox6[U8d(z[0])];
			break;
		}
		switch (i & 12) {
		 case 0:
		 case 12:
			key->xkey[i+0] = cast_sbox5[U8a(t[2])] ^ cast_sbox6[U8b(t[2])] ^
			 cast_sbox7[U8d(t[1])] ^ cast_sbox8[U8c(t[1])];
			key->xkey[i+1] = cast_sbox5[U8c(t[2])] ^ cast_sbox6[U8d(t[2])] ^
			 cast_sbox7[U8b(t[1])] ^ cast_sbox8[U8a(t[1])];
			key->xkey[i+2] = cast_sbox5[U8a(t[3])] ^ cast_sbox6[U8b(t[3])] ^
			 cast_sbox7[U8d(t[0])] ^ cast_sbox8[U8c(t[0])];
			key->xkey[i+3] = cast_sbox5[U8c(t[3])] ^ cast_sbox6[U8d(t[3])] ^
			 cast_sbox7[U8b(t[0])] ^ cast_sbox8[U8a(t[0])];
			break;
		 case 4:
		 case 8:
			key->xkey[i+0] = cast_sbox5[U8d(t[0])] ^ cast_sbox6[U8c(t[0])] ^
			 cast_sbox7[U8a(t[3])] ^ cast_sbox8[U8b(t[3])];
			key->xkey[i+1] = cast_sbox5[U8b(t[0])] ^ cast_sbox6[U8a(t[0])] ^
			 cast_sbox7[U8c(t[3])] ^ cast_sbox8[U8d(t[3])];
			key->xkey[i+2] = cast_sbox5[U8d(t[1])] ^ cast_sbox6[U8c(t[1])] ^
			 cast_sbox7[U8a(t[2])] ^ cast_sbox8[U8b(t[2])];
			key->xkey[i+3] = cast_sbox5[U8b(t[1])] ^ cast_sbox6[U8a(t[1])] ^
			 cast_sbox7[U8c(t[2])] ^ cast_sbox8[U8d(t[2])];
			break;
		}
		switch (i & 12) {
		 case 0:
			key->xkey[i+0] ^= cast_sbox5[U8c(z[0])];
			key->xkey[i+1] ^= cast_sbox6[U8c(z[1])];
			key->xkey[i+2] ^= cast_sbox7[U8b(z[2])];
			key->xkey[i+3] ^= cast_sbox8[U8a(z[3])];
			break;
		 case 4:
			key->xkey[i+0] ^= cast_sbox5[U8a(x[2])];
			key->xkey[i+1] ^= cast_sbox6[U8b(x[3])];
			key->xkey[i+2] ^= cast_sbox7[U8d(x[0])];
			key->xkey[i+3] ^= cast_sbox8[U8d(x[1])];
			break;
		 case 8:
			key->xkey[i+0] ^= cast_sbox5[U8b(z[2])];
			key->xkey[i+1] ^= cast_sbox6[U8a(z[3])];
			key->xkey[i+2] ^= cast_sbox7[U8c(z[0])];
			key->xkey[i+3] ^= cast_sbox8[U8c(z[1])];
			break;
		 case 12:
			key->xkey[i+0] ^= cast_sbox5[U8d(x[0])];
			key->xkey[i+1] ^= cast_sbox6[U8d(x[1])];
			key->xkey[i+2] ^= cast_sbox7[U8a(x[2])];
			key->xkey[i+3] ^= cast_sbox8[U8b(x[3])];
			break;
		}
		if (i >= 16) {
			key->xkey[i+0] &= 31;
			key->xkey[i+1] &= 31;
			key->xkey[i+2] &= 31;
			key->xkey[i+3] &= 31;
		}
	}
	/* Wipe clean */
	for (i = 0; i < 4; i++) {
		t[i] = x[i] = z[i] = 0;
	}
}

/* Made in Canada */

