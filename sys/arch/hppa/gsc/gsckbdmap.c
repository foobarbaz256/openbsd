/*	$OpenBSD: gsckbdmap.c,v 1.18 2006/07/20 16:45:51 mickey Exp $	*/

/*
 * THIS FILE IS AUTOMAGICALLY GENERATED.  DO NOT EDIT.
 *
 * generated by:
 *	OpenBSD: makemap.awk,v 1.6 2005/05/12 16:21:23 miod Exp 
 * generated from:
 */
/*	OpenBSD: wskbdmap_mfii.c,v 1.33 2006/07/20 16:45:05 mickey Exp  */
/*	$NetBSD: wskbdmap_mfii.c,v 1.15 2000/05/19 16:40:04 drochner Exp $	*/

/*
 * PLEASE DO NOT FORGET TO REGEN
 *	sys/arch/hppa/gsc/gsckbdmap.c
 *	sys/dev/usb/ukbdmap.c
 * AFTER ANY CHANGES TO THIS FILE!
 */

/*-
 * Copyright (c) 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Juergen Hannken-Illjes.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <dev/wscons/wsksymdef.h>
#include <dev/wscons/wsksymvar.h>
#include <hppa/gsc/gsckbdmap.h>

#define KC(n) KS_KEYCODE(n)

static const keysym_t gsckbd_keydesc_us[] = {
/*  pos      command		normal		shifted */
/* initially KC(160),	KS_AudioMute,	*/
/* initially KC(174),	KS_AudioLower,	*/
/* initially KC(176),	KS_AudioRaise,	*/
/* initially KC(219),	KS_Meta_L,	*/
/* initially KC(220),	KS_Meta_R,	*/
/* initially KC(221),	KS_Menu,	*/
    KC(1),	KS_Cmd_Screen8,	KS_f9,
    KC(3),	KS_Cmd_Screen4,	KS_f5,
    KC(4),	KS_Cmd_Screen2,	KS_f3,
    KC(5),	KS_Cmd_Screen0,	KS_f1,
    KC(6),	KS_Cmd_Screen1,	KS_f2,
    KC(7),	KS_Cmd_Screen11,KS_f12,
    KC(9),	KS_Cmd_Screen9,	KS_f10,
    KC(10),	KS_Cmd_Screen7,	KS_f8,
    KC(11),	KS_Cmd_Screen5,	KS_f6,
    KC(12),	KS_Cmd_Screen3,	KS_f4,
    KC(13),	KS_Tab,
    KC(14),	KS_grave,	KS_asciitilde,
    KC(17),	KS_Cmd2,	KS_Alt_L,
    KC(18),	KS_Shift_L,
    KC(20),	KS_Cmd1,	KS_Control_L,
    KC(21),	KS_q,
    KC(22),	KS_1,		KS_exclam,
    KC(26),	KS_z,
    KC(27),	KS_s,
    KC(28),	KS_a,
    KC(29),	KS_w,
    KC(30),	KS_2,		KS_at,
    KC(33),	KS_c,
    KC(34),	KS_x,
    KC(35),	KS_d,
    KC(36),	KS_e,
    KC(37),	KS_4,		KS_dollar,
    KC(38),	KS_3,		KS_numbersign,
    KC(41),	KS_space,
    KC(42),	KS_v,
    KC(43),	KS_f,
    KC(44),	KS_t,
    KC(45),	KS_r,
    KC(46),	KS_5,		KS_percent,
    KC(49),	KS_n,
    KC(50),	KS_b,
    KC(51),	KS_h,
    KC(52),	KS_g,
    KC(53),	KS_y,
    KC(54),	KS_6,		KS_asciicircum,
    KC(58),	KS_m,
    KC(59),	KS_j,
    KC(60),	KS_u,
    KC(61),	KS_7,		KS_ampersand,
    KC(62),	KS_8,		KS_asterisk,
    KC(65),	KS_comma,	KS_less,
    KC(66),	KS_k,
    KC(67),	KS_i,
    KC(68),	KS_o,
    KC(69),	KS_0,		KS_parenright,
    KC(70),	KS_9,		KS_parenleft,
    KC(73),	KS_period,	KS_greater,
    KC(74),	KS_slash,	KS_question,
    KC(75),	KS_l,
    KC(76),	KS_semicolon,	KS_colon,
    KC(77),	KS_p,
    KC(78),	KS_minus,	KS_underscore,
    KC(82),	KS_apostrophe,	KS_quotedbl,
    KC(84),	KS_bracketleft,	KS_braceleft,
    KC(85),	KS_equal,	KS_plus,
    KC(88),	KS_Caps_Lock,
    KC(89),	KS_Shift_R,
    KC(90),	KS_Return,
    KC(91),	KS_bracketright,KS_braceright,
    KC(93),	KS_backslash,	KS_bar,
    KC(102),	KS_Cmd_ResetEmul,KS_Delete,
    KC(105),	KS_KP_End,	KS_KP_1,
    KC(107),	KS_KP_Left,	KS_KP_4,
    KC(108),	KS_KP_Home,	KS_KP_7,
    KC(112),	KS_KP_Insert,	KS_KP_0,
    KC(113),	KS_Cmd_KbdReset,KS_KP_Delete,
    KC(114),	KS_KP_Down,	KS_KP_2,
    KC(115),	KS_KP_Begin,	KS_KP_5,
    KC(116),	KS_KP_Right,	KS_KP_6,
    KC(117),	KS_KP_Up,	KS_KP_8,
    KC(118),	KS_Cmd_Debugger,KS_Escape,
    KC(119),	KS_Num_Lock,
    KC(120),	KS_Cmd_Screen10,KS_f11,
    KC(121),	KS_KP_Add,
    KC(122),	KS_KP_Next,	KS_KP_3,
    KC(123),	KS_KP_Subtract,
    KC(124),	KS_KP_Multiply,
    KC(125),	KS_KP_Prior,	KS_KP_9,
    KC(126),	KS_Hold_Screen,
    KC(127),	KS_Pause,	/*Break*/
    KC(131),	KS_Cmd_Screen6,	KS_f7,
    KC(145),	KS_Cmd2,	KS_Alt_R,	KS_Multi_key,
    KC(148),	KS_Cmd1,	KS_Control_R,
    KC(202),	KS_KP_Divide,
    KC(218),	KS_KP_Enter,
    KC(233),	KS_End,
    KC(235),	KS_Left,
    KC(236),	KS_Home,
    KC(240),	KS_Insert,
    KC(242),	KS_Down,
    KC(244),	KS_Right,
    KC(245),	KS_Up,
    KC(250),	KS_Cmd_ScrollFwd,KS_Next,
    KC(252),	KS_Print_Screen,
    KC(253),	KS_Cmd_ScrollBack,KS_Prior,
    KC(241),	KS_Delete
};

static const keysym_t gsckbd_keydesc_precisionbook[] = {
/*  pos      command		normal		shifted */
    KC(7),   KS_Cmd_Screen0,	KS_f1,
    KC(15),  KS_Cmd_Screen1,	KS_f2,
    KC(23),  KS_Cmd_Screen2,	KS_f3,
    KC(31),  KS_Cmd_Screen3,	KS_f4,
    KC(39),  KS_Cmd_Screen4,	KS_f5,
    KC(47),  KS_Cmd_Screen5,	KS_f6,
    KC(55),  KS_Cmd_Screen6,	KS_f7,
    KC(63),  KS_Cmd_Screen7,	KS_f8,
    KC(71),  KS_Cmd_Screen8,	KS_f9,
    KC(79),  KS_Cmd_Screen9,	KS_f10,
    KC(86),  KS_Cmd_Screen10,	KS_f11,
    KC(94),  KS_Cmd_Screen11,	KS_f12,
    KC(8),   KS_Cmd_Debugger,	KS_Escape,
    KC(87),			KS_Print_Screen,
    KC(92),			KS_backslash,	KS_bar,
    KC(96),			KS_KP_Down,	KS_KP_2,
    KC(95),			KS_Hold_Screen,
    KC(97),			KS_KP_Left,	KS_KP_4,
    KC(98),			KS_Pause, /* Break */
    KC(99),			KS_KP_Up,	KS_KP_8,
    KC(100),			KS_KP_Delete,	KS_KP_Decimal,
    KC(101),			KS_KP_End,	KS_KP_1,
    KC(103),			KS_KP_Insert,	KS_KP_0,
    KC(106),			KS_KP_Right,	KS_KP_6,
    KC(109),			KS_KP_Next,	KS_KP_3,
    KC(110),			KS_KP_Home,	KS_KP_7,
    KC(111),			KS_KP_Prior,	KS_KP_9,
    KC(20),			KS_Caps_Lock,
    KC(17),  KS_Cmd1,		KS_Control_L,
    KC(88),  KS_Cmd1,		KS_Control_R,
    KC(25),  KS_Cmd2,		KS_Alt_L,
    KC(57),  KS_Cmd2,		KS_Alt_R,	KS_Multi_key,
    KC(139),			KS_Meta_L,
    KC(140),			KS_Meta_R,
};

#if !defined(SMALL_KERNEL) || !defined(__alpha__)

static const keysym_t gsckbd_keydesc_de[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_dead_circumflex,KS_dead_abovering,
    KC(21),	KS_q,		KS_Q,		KS_at,
    KC(26),	KS_y,
    KC(30),	KS_2,		KS_quotedbl,	KS_twosuperior,
    KC(38),	KS_3,		KS_section,	KS_threesuperior,
    KC(53),	KS_z,
    KC(54),	KS_6,		KS_ampersand,
    KC(58),	KS_m,		KS_M,		KS_mu,
    KC(61),	KS_7,		KS_slash,	KS_braceleft,
    KC(62),	KS_8,		KS_parenleft,	KS_bracketleft,
    KC(65),	KS_comma,	KS_semicolon,
    KC(69),	KS_0,		KS_equal,	KS_braceright,
    KC(70),	KS_9,		KS_parenright,	KS_bracketright,
    KC(73),	KS_period,	KS_colon,
    KC(74),	KS_minus,	KS_underscore,
    KC(76),	KS_odiaeresis,
    KC(78),	KS_ssharp,	KS_question,	KS_backslash,
    KC(82),	KS_adiaeresis,
    KC(84),	KS_udiaeresis,
    KC(85),	KS_dead_acute,	KS_dead_grave,
    KC(91),	KS_plus,	KS_asterisk,	KS_dead_tilde,
    KC(93),	KS_numbersign,	KS_apostrophe,
    KC(97),	KS_less,	KS_greater,	KS_bar,		KS_brokenbar,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_de_nodead[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_asciicircum,	KS_degree,
    KC(85),	KS_apostrophe,	KS_grave,
    KC(91),	KS_plus,	KS_asterisk,	KS_asciitilde,
};

static const keysym_t gsckbd_keydesc_dk[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_onehalf,	KS_paragraph,
    KC(30),	KS_2,		KS_quotedbl,	KS_at,
    KC(37),	KS_4,		KS_currency,	KS_dollar,
    KC(38),	KS_3,		KS_numbersign,	KS_sterling,
    KC(54),	KS_6,		KS_ampersand,
    KC(61),	KS_7,		KS_slash,	KS_braceleft,
    KC(62),	KS_8,		KS_parenleft,	KS_bracketleft,
    KC(65),	KS_comma,	KS_semicolon,
    KC(69),	KS_0,		KS_equal,	KS_braceright,
    KC(70),	KS_9,		KS_parenright,	KS_bracketright,
    KC(73),	KS_period,	KS_colon,
    KC(74),	KS_minus,	KS_underscore,
    KC(76),	KS_ae,
    KC(78),	KS_plus,	KS_question,
    KC(82),	KS_oslash,
    KC(84),	KS_aring,
    KC(85),	KS_dead_acute,	KS_dead_grave,	KS_bar,
    KC(91),	KS_dead_diaeresis,KS_dead_circumflex,KS_dead_tilde,
    KC(93),	KS_apostrophe,	KS_asterisk,
    KC(97),	KS_less,	KS_greater,	KS_backslash,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_dk_nodead[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(85),	KS_apostrophe,	KS_grave,	KS_bar,
    KC(91),	KS_diaeresis,	KS_asciicircum,	KS_asciitilde,
};

static const keysym_t gsckbd_keydesc_sv[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_section,	KS_onehalf,
    KC(76),	KS_odiaeresis,
    KC(78),	KS_plus,	KS_question,	KS_backslash,
    KC(82),	KS_adiaeresis,
    KC(91),	KS_dead_diaeresis,KS_dead_circumflex,KS_dead_tilde,
    KC(97),	KS_less,	KS_greater,	KS_bar,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_sv_nodead[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(85),	KS_apostrophe,	KS_grave,	KS_bar,
    KC(91),	KS_diaeresis,	KS_asciicircum,	KS_asciitilde,
};

static const keysym_t gsckbd_keydesc_no[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_bar,		KS_paragraph,
    KC(76),	KS_oslash,
    KC(82),	KS_ae,
    KC(85),	KS_backslash,	KS_dead_grave,	KS_dead_acute,
    KC(91),	KS_dead_diaeresis,KS_dead_circumflex,KS_dead_tilde,
    KC(97),	KS_less,	KS_greater,
};

static const keysym_t gsckbd_keydesc_no_nodead[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(85),	KS_backslash,	KS_grave,	KS_acute,
    KC(91),	KS_diaeresis,	KS_asciicircum,	KS_asciitilde,
};

static const keysym_t gsckbd_keydesc_fr[] = {
/*  pos	     normal		shifted		altgr		shift-altgr */
    KC(14),	KS_twosuperior,
    KC(21),	KS_a,
    KC(22),	KS_ampersand,	KS_1,
    KC(26),	KS_w,
    KC(28),	KS_q,
    KC(29),	KS_z,
    KC(30),	KS_eacute,	KS_2,		KS_asciitilde,
    KC(37),	KS_apostrophe,	KS_4,		KS_braceleft,
    KC(38),	KS_quotedbl,	KS_3,		KS_numbersign,
    KC(46),	KS_parenleft,	KS_5,		KS_bracketleft,
    KC(54),	KS_minus,	KS_6,		KS_bar,
    KC(58),	KS_comma,	KS_question,
    KC(61),	KS_egrave,	KS_7,		KS_grave,
    KC(62),	KS_underscore,	KS_8,		KS_backslash,
    KC(65),	KS_semicolon,	KS_period,
    KC(69),	KS_agrave,	KS_0,		KS_at,
    KC(70),	KS_ccedilla,	KS_9,		KS_asciicircum,
    KC(73),	KS_colon,	KS_slash,
    KC(74),	KS_exclam,	KS_section,
    KC(76),	KS_m,
    KC(78),	KS_parenright,	KS_degree,	KS_bracketright,
    KC(82),	KS_ugrave,	KS_percent,
    KC(84),	KS_dead_circumflex,KS_dead_diaeresis,
    KC(85),	KS_equal,	KS_plus,	KS_braceright,
    KC(91),	KS_dollar,	KS_sterling,	KS_currency,
    KC(93),	KS_asterisk,	KS_mu,
    KC(97),	KS_less,	KS_greater,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_it[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_backslash,	KS_bar,
    KC(30),	KS_2,		KS_quotedbl,	KS_twosuperior,
    KC(38),	KS_3,		KS_sterling,	KS_threesuperior,
    KC(46),	KS_5,		KS_percent,
    KC(54),	KS_6,		KS_ampersand,
    KC(61),	KS_7,		KS_slash,
    KC(62),	KS_8,		KS_parenleft,
    KC(65),	KS_comma,	KS_semicolon,
    KC(69),	KS_0,		KS_equal,
    KC(70),	KS_9,		KS_parenright,
    KC(73),	KS_period,	KS_colon,
    KC(74),	KS_minus,	KS_underscore,
    KC(76),	KS_ograve,	KS_Ccedilla,	KS_at,
    KC(78),	KS_apostrophe,	KS_question,
    KC(82),	KS_agrave,	KS_degree,	KS_numbersign,
    KC(84),	KS_egrave,	KS_eacute,	KS_braceleft,	KS_bracketleft,
    KC(85),	KS_igrave,	KS_asciicircum,
    KC(91),	KS_plus,	KS_asterisk,	KS_braceright,	KS_bracketright,
    KC(93),	KS_ugrave,	KS_section,
    KC(97),	KS_less,	KS_greater,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_uk[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_grave,	KS_grave,	KS_agrave,	KS_agrave,
    KC(22),	KS_1,		KS_exclam,	KS_plusminus,	KS_exclamdown,
    KC(30),	KS_2,		KS_quotedbl,	KS_twosuperior,	KS_cent,
    KC(37),	KS_4,		KS_dollar,	KS_acute,	KS_currency,
    KC(38),	KS_3,		KS_sterling,	KS_threesuperior,
    KC(46),	KS_5,		KS_percent,	KS_mu,		KS_yen,
    KC(54),	KS_6,		KS_asciicircum,	KS_paragraph,
    KC(61),	KS_7,		KS_ampersand,	KS_periodcentered,KS_brokenbar,
    KC(62),	KS_8,		KS_asterisk,	KS_cedilla,	KS_ordfeminine,
    KC(69),	KS_0,		KS_parenright,	KS_masculine,	KS_copyright,
    KC(70),	KS_9,		KS_parenleft,	KS_onesuperior,	KS_diaeresis,
    KC(78),	KS_minus,	KS_underscore,	KS_hyphen,	KS_ssharp,
    KC(82),	KS_apostrophe,	KS_at,		KS_section,	KS_Agrave,
    KC(85),	KS_equal,	KS_plus,	KS_onehalf,	KS_guillemotleft,
    KC(93),	KS_numbersign,	KS_asciitilde,	KS_sterling,	KS_thorn,
    KC(97),	KS_backslash,	KS_bar,		KS_Udiaeresis,
};

static const keysym_t gsckbd_keydesc_jp[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
/* initially KC(112),	KS_Hiragana_Katakana,	*/
/* initially KC(115),	KS_backslash,	KS_underscore,	*/
/* initially KC(121),	KS_Henkan,	*/
/* initially KC(123),	KS_Muhenkan,	*/
/* initially KC(125),	KS_backslash,	KS_bar,	*/
    KC(14),	KS_Zenkaku_Hankaku,/*replacegrave/tilde*/
    KC(30),	KS_2,		KS_quotedbl,
    KC(54),	KS_6,		KS_ampersand,
    KC(61),	KS_7,		KS_apostrophe,
    KC(62),	KS_8,		KS_parenleft,
    KC(69),	KS_0,
    KC(70),	KS_9,		KS_parenright,
    KC(76),	KS_semicolon,	KS_plus,
    KC(78),	KS_minus,	KS_equal,
    KC(82),	KS_colon,	KS_asterisk,
    KC(84),	KS_at,		KS_grave,
    KC(85),	KS_asciicircum,	KS_asciitilde,
    KC(91),	KS_bracketleft,	KS_braceleft,
    KC(93),	KS_bracketright,KS_braceright,
};

static const keysym_t gsckbd_keydesc_es[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_degree,	KS_ordfeminine,	KS_backslash,
    KC(22),	KS_1,		KS_exclam,	KS_bar,
    KC(30),	KS_2,		KS_quotedbl,	KS_at,
    KC(37),	KS_4,		KS_dollar,	KS_asciitilde,
    KC(38),	KS_3,		KS_periodcentered,KS_numbersign,
    KC(54),	KS_6,		KS_ampersand,
    KC(61),	KS_7,		KS_slash,
    KC(62),	KS_8,		KS_parenleft,
    KC(65),	KS_comma,	KS_semicolon,
    KC(69),	KS_0,		KS_equal,
    KC(70),	KS_9,		KS_parenright,
    KC(73),	KS_period,	KS_colon,
    KC(74),	KS_minus,	KS_underscore,
    KC(76),	KS_ntilde,
    KC(78),	KS_apostrophe,	KS_question,
    KC(82),	KS_dead_acute,	KS_dead_diaeresis,KS_braceleft,
    KC(84),	KS_dead_grave,	KS_dead_circumflex,KS_bracketleft,
    KC(85),	KS_exclamdown,	KS_questiondown,
    KC(91),	KS_plus,	KS_asterisk,	KS_bracketright,
    KC(93),	KS_ccedilla,	KS_Ccedilla,	KS_braceright,
    KC(97),	KS_less,	KS_greater,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_lt[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_grave,	KS_asciitilde,
    KC(21),	KS_L7_aogonek,	KS_L7_Aogonek,
    KC(22),	KS_exclam,	KS_1,		KS_at,
    KC(29),	KS_L7_zcaron,	KS_L7_Zcaron,
    KC(30),	KS_minus,	KS_2,		KS_underscore,
    KC(34),	KS_L7_umacron,	KS_L7_Umacron,
    KC(36),	KS_e,		KS_E,		KS_currency,
    KC(37),	KS_semicolon,	KS_4,		KS_dollar,
    KC(38),	KS_slash,	KS_3,		KS_numbersign,
    KC(41),	KS_space,	KS_space,	KS_nobreakspace,
    KC(43),	KS_L7_scaron,	KS_L7_Scaron,
    KC(46),	KS_colon,	KS_5,		KS_paragraph,
    KC(54),	KS_comma,	KS_6,		KS_asciicircum,
    KC(61),	KS_period,	KS_7,		KS_ampersand,
    KC(62),	KS_equal,	KS_8,		KS_asterisk,
    KC(65),	KS_L7_ccaron,	KS_L7_Ccaron,	KS_L7_dbllow9quot,
    KC(69),	KS_bracketright,KS_0,		KS_parenright,
    KC(70),	KS_bracketleft,	KS_9,		KS_parenleft,
    KC(73),	KS_f,		KS_F,		KS_L7_leftdblquot,
    KC(74),	KS_L7_eogonek,	KS_L7_Eogonek,	KS_backslash,
    KC(76),	KS_L7_uogonek,	KS_L7_Uogonek,
    KC(78),	KS_question,	KS_plus,	KS_apostrophe,
    KC(82),	KS_L7_edot,	KS_L7_Edot,	KS_quotedbl,
    KC(84),	KS_L7_iogonek,	KS_L7_Iogonek,	KS_braceleft,
    KC(85),	KS_x,		KS_X,		KS_percent,
    KC(91),	KS_w,		KS_W,		KS_braceright,
    KC(93),	KS_q,		KS_Q,		KS_bar,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_be[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_twosuperior,	KS_threesuperior,
    KC(21),	KS_a,
    KC(22),	KS_ampersand,	KS_1,		KS_bar,
    KC(26),	KS_w,
    KC(28),	KS_q,
    KC(29),	KS_z,
    KC(30),	KS_eacute,	KS_2,		KS_at,
    KC(37),	KS_apostrophe,	KS_4,
    KC(38),	KS_quotedbl,	KS_3,		KS_numbersign,
    KC(46),	KS_parenleft,	KS_5,
    KC(54),	KS_section,	KS_6,		KS_asciicircum,
    KC(58),	KS_comma,	KS_question,
    KC(61),	KS_egrave,	KS_7,
    KC(62),	KS_exclam,	KS_8,
    KC(65),	KS_semicolon,	KS_period,
    KC(69),	KS_agrave,	KS_0,		KS_braceright,
    KC(70),	KS_ccedilla,	KS_9,		KS_braceleft,
    KC(73),	KS_colon,	KS_slash,
    KC(74),	KS_equal,	KS_plus,	KS_asciitilde,
    KC(76),	KS_m,
    KC(78),	KS_parenright,	KS_degree,
    KC(82),	KS_ugrave,	KS_percent,	KS_acute,
    KC(84),	KS_dead_circumflex,KS_dead_diaeresis,KS_bracketleft,
    KC(85),	KS_minus,	KS_underscore,
    KC(91),	KS_dollar,	KS_asterisk,	KS_bracketright,
    KC(93),	KS_mu,		KS_sterling,	KS_grave,
    KC(97),	KS_less,	KS_greater,	KS_backslash,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};


static const keysym_t gsckbd_keydesc_us_dvorak[] = {
/*  pos      command		normal		shifted */
    KC(21),	KS_apostrophe,	KS_quotedbl,
    KC(26),	KS_semicolon,	KS_colon,
    KC(27),	KS_o,
    KC(29),	KS_comma,	KS_less,
    KC(33),	KS_j,
    KC(34),	KS_q,
    KC(35),	KS_e,
    KC(36),	KS_period,	KS_greater,
    KC(42),	KS_k,
    KC(43),	KS_u,
    KC(44),	KS_y,
    KC(45),	KS_p,
    KC(49),	KS_b,
    KC(50),	KS_x,
    KC(51),	KS_d,
    KC(52),	KS_i,
    KC(53),	KS_f,
    KC(59),	KS_h,
    KC(60),	KS_g,
    KC(65),	KS_w,
    KC(66),	KS_t,
    KC(67),	KS_c,
    KC(68),	KS_r,
    KC(73),	KS_v,
    KC(74),	KS_z,
    KC(75),	KS_n,
    KC(76),	KS_s,
    KC(77),	KS_l,
    KC(78),	KS_bracketleft,	KS_braceleft,
    KC(82),	KS_minus,	KS_underscore,
    KC(84),	KS_slash,	KS_question,
    KC(85),	KS_bracketright,KS_braceright,
    KC(91),	KS_equal,	KS_plus,
};

static const keysym_t gsckbd_keydesc_swapctrlcaps[] = {
/*  pos      command		normal		shifted */
    KC(20),	KS_Caps_Lock,
    KC(88),	KS_Cmd1,	KS_Control_L,
};

static const keysym_t gsckbd_keydesc_iopener[] = {
/*  pos      command		normal		shifted */
    KC(1),	KS_Cmd_Screen7,	KS_f8,
    KC(3),	KS_Cmd_Screen3,	KS_f4,
    KC(4),	KS_Cmd_Screen1,	KS_f2,
    KC(5),	KS_Cmd_Debugger,KS_Escape,
    KC(6),	KS_Cmd_Screen0,	KS_f1,
    KC(7),	KS_f11,
    KC(9),	KS_Cmd_Screen8,	KS_f9,
    KC(10),	KS_Cmd_Screen6,	KS_f7,
    KC(11),	KS_Cmd_Screen4,	KS_f5,
    KC(12),	KS_Cmd_Screen2,	KS_f3,
    KC(120),	KS_Cmd_Screen9,	KS_f10,
    KC(131),	KS_Cmd_Screen5,	KS_f6,
};

static const keysym_t gsckbd_keydesc_ru[] = {
/*  pos      normal		shifted		altgr			shift-altgr */
    KC(21),	KS_q,		KS_Q,		KS_Cyrillic_ishort,KS_Cyrillic_ISHORT,
    KC(26),	KS_z,		KS_Z,		KS_Cyrillic_ya,	KS_Cyrillic_YA,
    KC(27),	KS_s,		KS_S,		KS_Cyrillic_yeru,KS_Cyrillic_YERU,
    KC(28),	KS_a,		KS_A,		KS_Cyrillic_ef,	KS_Cyrillic_EF,
    KC(29),	KS_w,		KS_W,		KS_Cyrillic_tse,KS_Cyrillic_TSE,
    KC(33),	KS_c,		KS_C,		KS_Cyrillic_es,	KS_Cyrillic_ES,
    KC(34),	KS_x,		KS_X,		KS_Cyrillic_che,KS_Cyrillic_CHE,
    KC(35),	KS_d,		KS_D,		KS_Cyrillic_ve,	KS_Cyrillic_VE,
    KC(36),	KS_e,		KS_E,		KS_Cyrillic_u,	KS_Cyrillic_U,
    KC(42),	KS_v,		KS_V,		KS_Cyrillic_em,	KS_Cyrillic_EM,
    KC(43),	KS_f,		KS_F,		KS_Cyrillic_a,	KS_Cyrillic_A,
    KC(44),	KS_t,		KS_T,		KS_Cyrillic_ie,	KS_Cyrillic_IE,
    KC(45),	KS_r,		KS_R,		KS_Cyrillic_ka,	KS_Cyrillic_KA,
    KC(49),	KS_n,		KS_N,		KS_Cyrillic_te,	KS_Cyrillic_TE,
    KC(50),	KS_b,		KS_B,		KS_Cyrillic_i,	KS_Cyrillic_I,
    KC(51),	KS_h,		KS_H,		KS_Cyrillic_er,	KS_Cyrillic_ER,
    KC(52),	KS_g,		KS_G,		KS_Cyrillic_pe,	KS_Cyrillic_PE,
    KC(53),	KS_y,		KS_Y,		KS_Cyrillic_en,	KS_Cyrillic_EN,
    KC(54),	KS_6,		KS_asciicircum,	KS_6,		KS_comma,
    KC(58),	KS_m,		KS_M,		KS_Cyrillic_ssighn,KS_Cyrillic_SSIGHN,
    KC(59),	KS_j,		KS_J,		KS_Cyrillic_o,	KS_Cyrillic_O,
    KC(60),	KS_u,		KS_U,		KS_Cyrillic_ge,	KS_Cyrillic_GE,
    KC(61),	KS_7,		KS_ampersand,	KS_7,		KS_period,
    KC(65),	KS_comma,	KS_less,	KS_Cyrillic_be,	KS_Cyrillic_BE,
    KC(66),	KS_k,		KS_K,		KS_Cyrillic_el,	KS_Cyrillic_EL,
    KC(67),	KS_i,		KS_I,		KS_Cyrillic_sha,KS_Cyrillic_SHA,
    KC(68),	KS_o,		KS_O,		KS_Cyrillic_scha,KS_Cyrillic_SCHA,
    KC(73),	KS_period,	KS_greater,	KS_Cyrillic_yu,	KS_Cyrillic_YU,
    KC(74),	KS_slash,	KS_question,	KS_Cyrillic_yo,	KS_Cyrillic_YO,
    KC(75),	KS_l,		KS_L,		KS_Cyrillic_de,	KS_Cyrillic_DE,
    KC(76),	KS_semicolon,	KS_colon,	KS_Cyrillic_zhe,KS_Cyrillic_ZHE,
    KC(77),	KS_p,		KS_P,		KS_Cyrillic_ze,	KS_Cyrillic_ZE,
    KC(82),	KS_apostrophe,	KS_quotedbl,	KS_Cyrillic_e,	KS_Cyrillic_E,
    KC(84),	KS_bracketleft,	KS_braceleft,	KS_Cyrillic_ha,	KS_Cyrillic_HA,
    KC(91),	KS_bracketright,KS_braceright,	KS_Cyrillic_hsighn,KS_Cyrillic_HSIGHN,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_ua[] = {
/*  pos      normal		shifted		altgr			shift-altgr */
    KC(14),	KS_grave,	KS_asciitilde,	KS_Cyrillic_gheukr,KS_Cyrillic_GHEUKR,
    KC(21),	KS_q,		KS_Q,		KS_Cyrillic_ishort,KS_Cyrillic_ISHORT,
    KC(26),	KS_z,		KS_Z,		KS_Cyrillic_ya,	KS_Cyrillic_YA,
    KC(27),	KS_s,		KS_S,		KS_Cyrillic_yeru,KS_Cyrillic_YERU,
    KC(28),	KS_a,		KS_A,		KS_Cyrillic_ef,	KS_Cyrillic_EF,
    KC(29),	KS_w,		KS_W,		KS_Cyrillic_tse,KS_Cyrillic_TSE,
    KC(33),	KS_c,		KS_C,		KS_Cyrillic_es,	KS_Cyrillic_ES,
    KC(34),	KS_x,		KS_X,		KS_Cyrillic_che,KS_Cyrillic_CHE,
    KC(35),	KS_d,		KS_D,		KS_Cyrillic_ve,	KS_Cyrillic_VE,
    KC(36),	KS_e,		KS_E,		KS_Cyrillic_u,	KS_Cyrillic_U,
    KC(42),	KS_v,		KS_V,		KS_Cyrillic_em,	KS_Cyrillic_EM,
    KC(43),	KS_f,		KS_F,		KS_Cyrillic_a,	KS_Cyrillic_A,
    KC(44),	KS_t,		KS_T,		KS_Cyrillic_ie,	KS_Cyrillic_IE,
    KC(45),	KS_r,		KS_R,		KS_Cyrillic_ka,	KS_Cyrillic_KA,
    KC(49),	KS_n,		KS_N,		KS_Cyrillic_te,	KS_Cyrillic_TE,
    KC(50),	KS_b,		KS_B,		KS_Cyrillic_i,	KS_Cyrillic_I,
    KC(51),	KS_h,		KS_H,		KS_Cyrillic_er,	KS_Cyrillic_ER,
    KC(52),	KS_g,		KS_G,		KS_Cyrillic_pe,	KS_Cyrillic_PE,
    KC(53),	KS_y,		KS_Y,		KS_Cyrillic_en,	KS_Cyrillic_EN,
    KC(54),	KS_6,		KS_asciicircum,	KS_6,		KS_comma,
    KC(58),	KS_m,		KS_M,		KS_Cyrillic_ssighn,KS_Cyrillic_SSIGHN,
    KC(59),	KS_j,		KS_J,		KS_Cyrillic_o,	KS_Cyrillic_O,
    KC(60),	KS_u,		KS_U,		KS_Cyrillic_ge,	KS_Cyrillic_GE,
    KC(61),	KS_7,		KS_ampersand,	KS_7,		KS_period,
    KC(65),	KS_comma,	KS_less,	KS_Cyrillic_be,	KS_Cyrillic_BE,
    KC(66),	KS_k,		KS_K,		KS_Cyrillic_el,	KS_Cyrillic_EL,
    KC(67),	KS_i,		KS_I,		KS_Cyrillic_sha,KS_Cyrillic_SHA,
    KC(68),	KS_o,		KS_O,		KS_Cyrillic_scha,KS_Cyrillic_SCHA,
    KC(73),	KS_period,	KS_greater,	KS_Cyrillic_yu,	KS_Cyrillic_YU,
    KC(74),	KS_slash,	KS_question,	KS_Cyrillic_yo,	KS_Cyrillic_YO,
    KC(75),	KS_l,		KS_L,		KS_Cyrillic_de,	KS_Cyrillic_DE,
    KC(76),	KS_semicolon,	KS_colon,	KS_Cyrillic_zhe,KS_Cyrillic_ZHE,
    KC(77),	KS_p,		KS_P,		KS_Cyrillic_ze,	KS_Cyrillic_ZE,
    KC(78),	KS_minus,	KS_underscore,	KS_Cyrillic_iukr,KS_Cyrillic_IUKR,
    KC(82),	KS_apostrophe,	KS_quotedbl,	KS_Cyrillic_e,	KS_Cyrillic_E,
    KC(84),	KS_bracketleft,	KS_braceleft,	KS_Cyrillic_ha,	KS_Cyrillic_HA,
    KC(85),	KS_equal,	KS_plus,	KS_Cyrillic_yeukr,KS_Cyrillic_YEUKR,
    KC(91),	KS_bracketright,KS_braceright,	KS_Cyrillic_hsighn,KS_Cyrillic_HSIGHN,
    KC(93),	KS_backslash,	KS_bar,		KS_Cyrillic_yi,	KS_Cyrillic_YI,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_sg[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_section,	KS_degree,	KS_dead_abovering,
    KC(22),	KS_1,		KS_plus,	KS_bar,
    KC(26),	KS_y,
    KC(30),	KS_2,		KS_quotedbl,	KS_at,
    KC(36),	KS_e,		KS_E,		KS_currency,
    KC(37),	KS_4,		KS_ccedilla,
    KC(38),	KS_3,		KS_asterisk,	KS_numbersign,
    KC(53),	KS_z,
    KC(54),	KS_6,		KS_ampersand,	KS_notsign,
    KC(61),	KS_7,		KS_slash,	KS_brokenbar,
    KC(62),	KS_8,		KS_parenleft,	KS_cent,
    KC(65),	KS_comma,	KS_semicolon,
    KC(69),	KS_0,		KS_equal,
    KC(70),	KS_9,		KS_parenright,
    KC(73),	KS_period,	KS_colon,
    KC(74),	KS_minus,	KS_underscore,
    KC(76),	KS_odiaeresis,	KS_eacute,
    KC(78),	KS_apostrophe,	KS_question,	KS_dead_acute,
    KC(82),	KS_adiaeresis,	KS_agrave,	KS_braceleft,
    KC(84),	KS_udiaeresis,	KS_egrave,	KS_bracketleft,
    KC(85),	KS_dead_circumflex,KS_dead_grave,KS_dead_tilde,
    KC(91),	KS_dead_diaeresis,KS_exclam,	KS_bracketright,
    KC(93),	KS_dollar,	KS_sterling,	KS_braceright,
    KC(97),	KS_less,	KS_greater,	KS_backslash,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_sg_nodead[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(78),	KS_apostrophe,	KS_question,	KS_acute,
    KC(85),	KS_asciicircum,	KS_grave,	KS_asciitilde,
    KC(91),	KS_diaeresis,	KS_exclam,	KS_bracketright,
};

static const keysym_t gsckbd_keydesc_sf[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(76),	KS_eacute,	KS_odiaeresis,
    KC(82),	KS_agrave,	KS_adiaeresis,	KS_braceleft,
    KC(84),	KS_egrave,	KS_udiaeresis,	KS_bracketleft,
};

static const keysym_t gsckbd_keydesc_pt[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_backslash,	KS_bar,
    KC(30),	KS_2,		KS_quotedbl,	KS_at,
    KC(38),	KS_3,		KS_numbersign,	KS_sterling,
    KC(54),	KS_6,		KS_ampersand,
    KC(61),	KS_7,		KS_slash,	KS_braceleft,
    KC(62),	KS_8,		KS_parenleft,	KS_bracketleft,
    KC(65),	KS_comma,	KS_semicolon,
    KC(69),	KS_0,		KS_equal,	KS_braceright,
    KC(70),	KS_9,		KS_parenright,	KS_bracketright,
    KC(73),	KS_period,	KS_colon,
    KC(74),	KS_minus,	KS_underscore,
    KC(76),	KS_ccedilla,	KS_Ccedilla,
    KC(78),	KS_apostrophe,	KS_question,
    KC(82),	KS_masculine,	KS_ordfeminine,
    KC(84),	KS_plus,	KS_asterisk,
    KC(85),	KS_less,	KS_greater,
    KC(91),	KS_dead_acute,	KS_dead_grave,
    KC(93),	KS_dead_tilde,	KS_dead_circumflex,
    KC(97),	KS_less,	KS_greater,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_la[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_bar,		KS_degree,	KS_notsign,
    KC(21),	KS_q,		KS_Q,		KS_at,
    KC(22),	KS_1,		KS_exclam,
    KC(30),	KS_2,		KS_quotedbl,
    KC(38),	KS_3,		KS_numbersign,
    KC(54),	KS_6,		KS_ampersand,
    KC(61),	KS_7,		KS_slash,
    KC(62),	KS_8,		KS_parenleft,
    KC(65),	KS_comma,	KS_semicolon,
    KC(69),	KS_0,		KS_equal,
    KC(70),	KS_9,		KS_parenright,
    KC(73),	KS_period,	KS_colon,
    KC(74),	KS_minus,	KS_underscore,
    KC(76),	KS_ntilde,
    KC(78),	KS_apostrophe,	KS_question,	KS_backslash,
    KC(82),	KS_braceleft,	KS_bracketleft,	KS_dead_circumflex,
    KC(84),	KS_dead_acute,	KS_dead_diaeresis,
    KC(85),	KS_questiondown,KS_exclamdown,
    KC(91),	KS_plus,	KS_asterisk,	KS_asciitilde,
    KC(93),	KS_braceright,	KS_bracketright,KS_dead_grave,
    KC(97),	KS_less,	KS_greater,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_br[] = {
/*  pos      normal		shifted         altgr           shift-altgr */
/* initially KC(115),	KS_slash,	KS_question,	KS_degree,	*/
    KC(14),	KS_apostrophe,	KS_quotedbl,
    KC(22),	KS_1,		KS_exclam,	KS_onesuperior,
    KC(30),	KS_2,		KS_at,		KS_twosuperior,
    KC(37),	KS_4,		KS_dollar,	KS_sterling,
    KC(38),	KS_3,		KS_numbersign,	KS_threesuperior,
    KC(46),	KS_5,		KS_percent,	KS_cent,
    KC(54),	KS_6,		KS_dead_diaeresis,KS_notsign,
    KC(74),	KS_semicolon,	KS_colon,
    KC(76),	KS_ccedilla,	KS_Ccedilla,
    KC(82),	KS_dead_tilde,	KS_dead_circumflex,
    KC(84),	KS_dead_acute,	KS_dead_grave,
    KC(85),	KS_equal,	KS_plus,	KS_section,
    KC(91),	KS_bracketleft,	KS_braceleft,	KS_ordfeminine,
    KC(93),	KS_bracketright,KS_braceright,	KS_masculine,
    KC(97),	KS_backslash,	KS_bar,
    KC(113),	KS_KP_Delete,	KS_KP_Decimal,
};

static const keysym_t gsckbd_keydesc_tr[] = {
/*  pos      normal		shifted         altgr           shift-altgr */
    KC(14),	KS_quotedbl,	KS_eacute,
    KC(21),	KS_q,		KS_Q,		KS_at,
    KC(30),	KS_2,		KS_apostrophe,	KS_sterling,
    KC(37),	KS_4,		KS_plus,	KS_dollar,
    KC(38),	KS_3,		KS_asciicircum,	KS_numbersign,
    KC(46),	KS_5,		KS_percent,	KS_onehalf,
    KC(54),	KS_6,		KS_ampersand,
    KC(61),	KS_7,		KS_slash,	KS_braceleft,
    KC(62),	KS_8,		KS_parenleft,	KS_bracketleft,
    KC(65),	KS_odiaeresis,	KS_Odiaeresis,
    KC(67),	KS_L5_idotless,	KS_I,
    KC(69),	KS_0,		KS_equal,	KS_braceright,
    KC(70),	KS_9,		KS_parenright,	KS_bracketright,
    KC(73),	KS_ccedilla,	KS_Ccedilla,
    KC(74),	KS_period,	KS_colon,
    KC(76),	KS_L5_scedilla,	KS_L5_Scedilla,	KS_dead_acute,
    KC(78),	KS_asterisk,	KS_question,	KS_backslash,
    KC(82),	KS_i,		KS_L5_Idotabove,
    KC(84),	KS_L5_gbreve,	KS_L5_Gbreve,	KS_dead_diaeresis,
    KC(85),	KS_minus,	KS_underscore,
    KC(91),	KS_udiaeresis,	KS_Udiaeresis,	KS_asciitilde,
    KC(93),	KS_comma,	KS_semicolon,	KS_dead_grave,
    KC(97),	KS_less,	KS_greater,	KS_bar,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_tr_nodead[] = {
/*  pos      normal		shifted         altgr           shift-altgr */
    KC(76),	KS_L5_scedilla,	KS_L5_Scedilla,	KS_apostrophe,
    KC(84),	KS_L5_gbreve,	KS_L5_Gbreve,
    KC(93),	KS_comma,	KS_semicolon,	KS_grave,
};

static const keysym_t gsckbd_keydesc_pl[] = {
/*  pos      normal		shifted         altgr           shift-altgr */
    KC(26),	KS_z,		KS_Z,		KS_L2_zdotabove,KS_L2_Zdotabove,
    KC(27),	KS_s,		KS_S,		KS_L2_sacute,	KS_L2_Sacute,
    KC(28),	KS_a,		KS_A,		KS_L2_aogonek,	KS_L2_Aogonek,
    KC(33),	KS_c,		KS_C,		KS_L2_cacute,	KS_L2_Cacute,
    KC(34),	KS_x,		KS_X,		KS_L2_zacute,	KS_L2_Zacute,
    KC(36),	KS_e,		KS_E,		KS_L2_eogonek,	KS_L2_Eogonek,
    KC(49),	KS_n,		KS_N,		KS_L2_nacute,	KS_L2_Nacute,
    KC(68),	KS_o,		KS_O,		KS_oacute,	KS_Oacute,
    KC(75),	KS_l,		KS_L,		KS_L2_lstroke,	KS_L2_Lstroke,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_hu[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_0,		KS_section,
    KC(21),	KS_q,		KS_Q,		KS_backslash,
    KC(22),	KS_1,		KS_apostrophe,	KS_asciitilde,
    KC(26),	KS_y,		KS_Y,		KS_greater,
    KC(29),	KS_w,		KS_W,KS_bar,
    KC(30),	KS_2,		KS_quotedbl,
    KC(33),	KS_c,		KS_C,		KS_ampersand,
    KC(34),	KS_x,		KS_X,		KS_numbersign,
    KC(37),	KS_4,		KS_exclam,
    KC(38),	KS_3,		KS_plus,	KS_asciicircum,
    KC(42),	KS_v,		KS_V,		KS_at,
    KC(43),	KS_f,		KS_F,		KS_bracketleft,
    KC(46),	KS_5,		KS_percent,
    KC(49),	KS_n,		KS_N,		KS_braceright,
    KC(50),	KS_b,		KS_B,		KS_braceleft,
    KC(52),	KS_g,		KS_G,		KS_bracketright,
    KC(53),	KS_z,		KS_Z,
    KC(54),	KS_6,		KS_slash,
    KC(59),	KS_j,		KS_J,		KS_iacute,
    KC(61),	KS_7,		KS_equal,KS_grave,
    KC(62),	KS_8,		KS_parenleft,
    KC(65),	KS_comma,	KS_question,	KS_semicolon,
    KC(67),	KS_i,		KS_I,		KS_iacute,
    KC(69),	KS_odiaeresis,	KS_Odiaeresis,
    KC(70),	KS_9,		KS_parenright,	KS_acute,
    KC(73),	KS_period,	KS_colon,
    KC(74),	KS_minus,	KS_underscore,	KS_asterisk,
    KC(76),	KS_eacute,	KS_Eacute,	KS_dollar,
    KC(78),	KS_udiaeresis,	KS_Udiaeresis,
    KC(82),	KS_aacute,	KS_Aacute,	KS_ssharp,
    KC(84),	KS_odoubleacute,KS_Odoubleacute,KS_division,
    KC(85),	KS_oacute,	KS_Oacute,
    KC(91),	KS_uacute,	KS_Uacute,	KS_multiply,
    KC(93),	KS_udoubleacute,KS_Udoubleacute,KS_currency,
    KC(97),	KS_iacute,	KS_Iacute,	KS_less,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_si[]=
{
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_cedilla,	KS_diaeresis,
    KC(21),	KS_q,		KS_Q,		KS_backslash,
    KC(22),	KS_1,		KS_exclam,	KS_asciitilde,
    KC(26),	KS_y,		KS_Y,
    KC(29),	KS_w,		KS_W,		KS_bar,
    KC(30),	KS_2,		KS_quotedbl,	KS_L2_caron,
    KC(37),	KS_4,		KS_dollar,	KS_L2_breve,
    KC(38),	KS_3,		KS_numbersign,	KS_asciicircum,
    KC(42),	KS_v,		KS_V,		KS_at,
    KC(43),	KS_f,		KS_F,		KS_bracketleft,
    KC(46),	KS_5,		KS_percent,	KS_degree,
    KC(49),	KS_n,		KS_N,		KS_braceright,
    KC(50),	KS_b,		KS_B,		KS_braceleft,
    KC(52),	KS_g,		KS_G,		KS_bracketright,
    KC(53),	KS_z,		KS_Z,
    KC(54),	KS_6,		KS_ampersand,	KS_L2_ogonek,
    KC(58),	KS_m,		KS_M,		KS_section,
    KC(61),	KS_7,		KS_slash,	KS_grave,
    KC(62),	KS_8,		KS_parenleft,	KS_L2_dotabove,
    KC(65),	KS_comma,	KS_semicolon,
    KC(66),	KS_k,		KS_K,		KS_L2_lstroke,
    KC(69),	KS_0,		KS_equal,	KS_L2_dblacute,
    KC(70),	KS_9,		KS_parenright,	KS_acute,
    KC(73),	KS_period,	KS_colon,
    KC(74),	KS_minus,	KS_underscore,
    KC(75),	KS_l,		KS_L,		KS_L2_Lstroke,
    KC(76),	KS_L2_ccaron,	KS_L2_Ccaron,
    KC(78),	KS_apostrophe,	KS_question,	KS_diaeresis,
    KC(82),	KS_L2_cacute,	KS_L2_Cacute,	KS_ssharp,
    KC(84),	KS_L2_scaron,	KS_L2_Scaron,	KS_division,
    KC(85),	KS_plus,	KS_asterisk,	KS_cedilla,
    KC(91),	KS_L2_dstroke,	KS_L2_Dstroke,	KS_multiply,
    KC(93),	KS_L2_zcaron,	KS_L2_Zcaron,	KS_currency,
    KC(97),	KS_less,	KS_greater,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_cf[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(14),	KS_numbersign,	KS_bar,		KS_backslash,
    KC(22),	KS_1,		KS_exclam,	KS_plusminus,
    KC(30),	KS_2,		KS_quotedbl,	KS_at,
    KC(37),	KS_4,		KS_dollar,	KS_cent,
    KC(38),	KS_3,		KS_slash,	KS_sterling,
    KC(46),	KS_5,		KS_percent,	KS_diaeresis,
    KC(54),	KS_6,		KS_question,	KS_macron,
    KC(58),	KS_m,		KS_M,		KS_mu,
    KC(61),	KS_7,		KS_ampersand,	KS_brokenbar,
    KC(62),	KS_8,		KS_asterisk,	KS_twosuperior,
    KC(65),	KS_comma,	KS_apostrophe,	KS_hyphen,
    KC(68),	KS_o,		KS_O,		KS_section,
    KC(69),	KS_0,		KS_parenright,	KS_onequarter,
    KC(70),	KS_9,		KS_parenleft,	KS_threesuperior,
    KC(73),	KS_period,	KS_period,
    KC(74),	KS_eacute,	KS_Eacute,	KS_dead_acute,
    KC(76),	KS_semicolon,	KS_colon,	KS_asciitilde,
    KC(77),	KS_p,		KS_P,		KS_paragraph,
    KC(78),	KS_minus,	KS_underscore,	KS_onehalf,
    KC(82),	KS_dead_grave,	KS_dead_grave,	KS_braceleft,
    KC(84),	KS_dead_circumflex,KS_dead_circumflex,KS_bracketleft,
    KC(85),	KS_equal,	KS_plus,	KS_threequarters,
    KC(91),	KS_dead_cedilla,KS_dead_diaeresis,KS_bracketright,
    KC(93),	KS_less,	KS_greater,	KS_braceright,
    KC(97),	KS_guillemotleft,KS_guillemotright,KS_degree,
    KC(145),	KS_Mode_switch,KS_Multi_key,
};

static const keysym_t gsckbd_keydesc_cf_nodead[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(74),	KS_eacute,	KS_Eacute,	KS_acute,
    KC(82),	KS_grave,	KS_grave,	KS_braceleft,
    KC(84),	KS_asciicircum,KS_asciicircum,KS_bracketleft,
    KC(91),	KS_cedilla,	KS_diaeresis,	KS_bracketright,
};

static const keysym_t gsckbd_keydesc_lv[] = {
/*  pos      normal		shifted		altgr		shift-altgr */
    KC(26),	KS_z,		KS_Z,		KS_L7_zcaron,	KS_L7_Zcaron,
    KC(27),	KS_s,		KS_S,		KS_L7_scaron,	KS_L7_Scaron,
    KC(28),	KS_a,		KS_A,		KS_L7_amacron,	KS_L7_Amacron,
    KC(33),	KS_c,		KS_C,		KS_L7_ccaron,	KS_L7_Ccaron,
    KC(36),	KS_e,		KS_E,		KS_L7_emacron,	KS_L7_Emacron,
    KC(49),	KS_n,		KS_N,		KS_L7_ncedilla,	KS_L7_Ncedilla,
    KC(52),	KS_g,		KS_G,		KS_L7_gcedilla,	KS_L7_Gcedilla,
    KC(60),	KS_u,		KS_U,		KS_L7_umacron,	KS_L7_Umacron,
    KC(66),	KS_k,		KS_K,		KS_L7_kcedilla,	KS_L7_Kcedilla,
    KC(67),	KS_i,		KS_I,		KS_L7_imacron,	KS_L7_Imacron,
    KC(68),	KS_o,		KS_O,		KS_L7_omacron,	KS_L7_Omacron,
    KC(75),	KS_l,		KS_L,		KS_L7_lcedilla,	KS_L7_Lcedilla,
    KC(145),	KS_Mode_switch,	KS_Multi_key,
};

#endif	/* SMALL_KERNEL */

#define KBD_MAP(name, base, map) \
			{ name, base, sizeof(map)/sizeof(keysym_t), map }

const struct wscons_keydesc gsckbd_keydesctab[] = {
	KBD_MAP(KB_US,			0,	gsckbd_keydesc_us),
#if !defined(SMALL_KERNEL) || !defined(__alpha__)
	KBD_MAP(KB_DE,			KB_US,	gsckbd_keydesc_de),
	KBD_MAP(KB_DE | KB_NODEAD,	KB_DE,	gsckbd_keydesc_de_nodead),
	KBD_MAP(KB_FR,			KB_US,	gsckbd_keydesc_fr),
	KBD_MAP(KB_DK,			KB_US,	gsckbd_keydesc_dk),
	KBD_MAP(KB_DK | KB_NODEAD,	KB_DK,	gsckbd_keydesc_dk_nodead),
	KBD_MAP(KB_IT,			KB_US,	gsckbd_keydesc_it),
	KBD_MAP(KB_UK,			KB_US,	gsckbd_keydesc_uk),
	KBD_MAP(KB_JP,			KB_US,	gsckbd_keydesc_jp),
	KBD_MAP(KB_SV,			KB_DK,	gsckbd_keydesc_sv),
	KBD_MAP(KB_SV | KB_NODEAD,	KB_SV,	gsckbd_keydesc_sv_nodead),
	KBD_MAP(KB_NO,			KB_DK,	gsckbd_keydesc_no),
	KBD_MAP(KB_NO | KB_NODEAD,	KB_NO,	gsckbd_keydesc_no_nodead),
	KBD_MAP(KB_US | KB_DVORAK,	KB_US,	gsckbd_keydesc_us_dvorak),
	KBD_MAP(KB_US | KB_SWAPCTRLCAPS, KB_US,	gsckbd_keydesc_swapctrlcaps),
	KBD_MAP(KB_US | KB_IOPENER,	KB_US,	gsckbd_keydesc_iopener),
	KBD_MAP(KB_JP | KB_SWAPCTRLCAPS, KB_JP,	gsckbd_keydesc_swapctrlcaps),
	KBD_MAP(KB_FR | KB_SWAPCTRLCAPS, KB_FR,	gsckbd_keydesc_swapctrlcaps),
	KBD_MAP(KB_BE | KB_SWAPCTRLCAPS, KB_BE,	gsckbd_keydesc_swapctrlcaps),
	KBD_MAP(KB_US | KB_DVORAK | KB_SWAPCTRLCAPS,	KB_US | KB_DVORAK,
		gsckbd_keydesc_swapctrlcaps),
	KBD_MAP(KB_US | KB_IOPENER | KB_SWAPCTRLCAPS,	KB_US | KB_IOPENER,
		gsckbd_keydesc_swapctrlcaps),
	KBD_MAP(KB_ES,			KB_US,	gsckbd_keydesc_es),
	KBD_MAP(KB_BE,			KB_US,	gsckbd_keydesc_be),
	KBD_MAP(KB_RU,			KB_US,	gsckbd_keydesc_ru),
	KBD_MAP(KB_UA,			KB_US,	gsckbd_keydesc_ua),
	KBD_MAP(KB_SG,			KB_US,	gsckbd_keydesc_sg),
	KBD_MAP(KB_SG | KB_NODEAD,	KB_SG,	gsckbd_keydesc_sg_nodead),
	KBD_MAP(KB_SF,			KB_SG,	gsckbd_keydesc_sf),
	KBD_MAP(KB_SF | KB_NODEAD,	KB_SF,	gsckbd_keydesc_sg_nodead),
	KBD_MAP(KB_PT,			KB_US,	gsckbd_keydesc_pt),
	KBD_MAP(KB_LT,			KB_US,	gsckbd_keydesc_lt),
	KBD_MAP(KB_LA,			KB_US,	gsckbd_keydesc_la),
	KBD_MAP(KB_BR,			KB_US,	gsckbd_keydesc_br),
	KBD_MAP(KB_TR,			KB_US,	gsckbd_keydesc_tr),
	KBD_MAP(KB_TR | KB_NODEAD,	KB_TR,	gsckbd_keydesc_tr_nodead),
	KBD_MAP(KB_PL,			KB_US,	gsckbd_keydesc_pl),
	KBD_MAP(KB_HU,			KB_US,	gsckbd_keydesc_hu),
	KBD_MAP(KB_SI,			KB_US,	gsckbd_keydesc_si),
	KBD_MAP(KB_CF,			KB_US, 	gsckbd_keydesc_cf),
	KBD_MAP(KB_CF | KB_NODEAD,	KB_CF,	gsckbd_keydesc_cf_nodead),
	KBD_MAP(KB_LV,			KB_US,	gsckbd_keydesc_lv),
#endif	/* SMALL_KERNEL */
	KBD_MAP(KB_US | KB_MACHDEP,	KB_US,	gsckbd_keydesc_precisionbook),
	{0, 0, 0, 0}
};

#undef KBD_MAP
#undef KC
