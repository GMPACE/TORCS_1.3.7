/*
The contents of this file are subject to the Mozilla Public License
Version 1.0 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is expat.

The Initial Developer of the Original Code is James Clark.
Portions created by James Clark are Copyright (C) 1998
James Clark. All Rights Reserved.

Contributor(s):
$Id: iasciitab.h,v 1.1.1.1 2001/06/24 18:29:46 torcs Exp $
*/

/* Like asciitab.h, except that 0xD has code BT_S rather than BT_CR */
/* 0x00 */ BT_NONXML, BT_NONXML, BT_NONXML, BT_NONXML,
/* 0x04 */ BT_NONXML, BT_NONXML, BT_NONXML, BT_NONXML,
/* 0x08 */ BT_NONXML, BT_S, BT_LF, BT_NONXML,
/* 0x0C */ BT_NONXML, BT_S, BT_NONXML, BT_NONXML,
/* 0x10 */ BT_NONXML, BT_NONXML, BT_NONXML, BT_NONXML,
/* 0x14 */ BT_NONXML, BT_NONXML, BT_NONXML, BT_NONXML,
/* 0x18 */ BT_NONXML, BT_NONXML, BT_NONXML, BT_NONXML,
/* 0x1C */ BT_NONXML, BT_NONXML, BT_NONXML, BT_NONXML,
/* 0x20 */ BT_S, BT_EXCL, BT_QUOT, BT_NUM,
/* 0x24 */ BT_OTHER, BT_PERCNT, BT_AMP, BT_APOS,
/* 0x28 */ BT_LPAR, BT_RPAR, BT_AST, BT_PLUS,
/* 0x2C */ BT_COMMA, BT_MINUS, BT_NAME, BT_SOL,
/* 0x30 */ BT_DIGIT, BT_DIGIT, BT_DIGIT, BT_DIGIT,
/* 0x34 */ BT_DIGIT, BT_DIGIT, BT_DIGIT, BT_DIGIT,
/* 0x38 */ BT_DIGIT, BT_DIGIT, BT_NMSTRT, BT_SEMI,
/* 0x3C */ BT_LT, BT_EQUALS, BT_GT, BT_QUEST,
/* 0x40 */ BT_OTHER, BT_HEX, BT_HEX, BT_HEX,
/* 0x44 */ BT_HEX, BT_HEX, BT_HEX, BT_NMSTRT,
/* 0x48 */ BT_NMSTRT, BT_NMSTRT, BT_NMSTRT, BT_NMSTRT,
/* 0x4C */ BT_NMSTRT, BT_NMSTRT, BT_NMSTRT, BT_NMSTRT,
/* 0x50 */ BT_NMSTRT, BT_NMSTRT, BT_NMSTRT, BT_NMSTRT,
/* 0x54 */ BT_NMSTRT, BT_NMSTRT, BT_NMSTRT, BT_NMSTRT,
/* 0x58 */ BT_NMSTRT, BT_NMSTRT, BT_NMSTRT, BT_LSQB,
/* 0x5C */ BT_OTHER, BT_RSQB, BT_OTHER, BT_NMSTRT,
/* 0x60 */ BT_OTHER, BT_HEX, BT_HEX, BT_HEX,
/* 0x64 */ BT_HEX, BT_HEX, BT_HEX, BT_NMSTRT,
/* 0x68 */ BT_NMSTRT, BT_NMSTRT, BT_NMSTRT, BT_NMSTRT,
/* 0x6C */ BT_NMSTRT, BT_NMSTRT, BT_NMSTRT, BT_NMSTRT,
/* 0x70 */ BT_NMSTRT, BT_NMSTRT, BT_NMSTRT, BT_NMSTRT,
/* 0x74 */ BT_NMSTRT, BT_NMSTRT, BT_NMSTRT, BT_NMSTRT,
/* 0x78 */ BT_NMSTRT, BT_NMSTRT, BT_NMSTRT, BT_OTHER,
/* 0x7C */ BT_VERBAR, BT_OTHER, BT_OTHER, BT_OTHER,
