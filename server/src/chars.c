/* chaosircd - pi-networks irc server
 *
 * Copyright (C) 2003  Roman Senn <smoli@paranoya.ch>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: chars.c,v 1.2 2004/12/30 16:31:17 smoli Exp $
 */

#include <stdlib.h>

#include <tichu/chars.h>

/* -------------------------------------------------------------------------- *
 * char attribute table                                                       *
 *                                                                            *
 * NOTE: RFC 1459 sez: anything but a ^G, comma, or space is allowed          *
 * for channel names                                                          *
 * -------------------------------------------------------------------------- */
const uint32_t chars[] = {
  /* 0  */     CNTRL_C,
  /* 1  */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 2  */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 3  */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 4  */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 5  */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 6  */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 7 BEL */  CNTRL_C|NONEOS_C,
  /* 8  \b */  CNTRL_C|CHAN_C|NONEOS_C,
  /* 9  \t */  CNTRL_C|SPACE_C|CHAN_C|NONEOS_C,
  /* 10 \n */  CNTRL_C|SPACE_C|CHAN_C|NONEOS_C|EOL_C,
  /* 11 \v */  CNTRL_C|SPACE_C|CHAN_C|NONEOS_C,
  /* 12 \f */  CNTRL_C|SPACE_C|CHAN_C|NONEOS_C,
  /* 13 \r */  CNTRL_C|SPACE_C|CHAN_C|NONEOS_C|EOL_C,
  /* 14 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 15 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 16 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 17 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 18 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 19 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 20 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 21 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 22 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 23 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 24 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 25 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 26 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 27 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 28 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 29 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 30 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* 31 */     CNTRL_C|CHAN_C|NONEOS_C,
  /* SP */     PRINT_C|SPACE_C,
  /* ! */      PRINT_C|KWILD_C|CHAN_C|NONEOS_C,
  /* " */      PRINT_C|CHAN_C|NONEOS_C,
  /* # */      PRINT_C|CHANPFX_C|CHAN_C|NONEOS_C,
  /* $ */      PRINT_C|CHAN_C|NONEOS_C|USER_C,
  /* % */      PRINT_C|CHAN_C|NONEOS_C,
  /* & */      PRINT_C|CHANPFX_C|CHAN_C|NONEOS_C,
  /* ' */      PRINT_C|CHAN_C|NONEOS_C,
  /* ( */      PRINT_C|CHAN_C|NONEOS_C,
  /* ) */      PRINT_C|CHAN_C|NONEOS_C,
  /* * */      PRINT_C|KWILD_C|CHAN_C|NONEOS_C|SERV_C,
  /* + */      PRINT_C|CHAN_C|NONEOS_C,
  /* , */      PRINT_C|NONEOS_C,
  /* - */      PRINT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
  /* . */      PRINT_C|KWILD_C|CHAN_C|NONEOS_C|USER_C|HOST_C|SERV_C,
  /* / */      PRINT_C|CHAN_C|NONEOS_C,
  /* 0 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* 1 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* 2 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* 3 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* 4 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* 5 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* 6 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* 7 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* 8 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* 9 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* : */      PRINT_C|CHAN_C|NONEOS_C|HOST_C,
  /* ; */      PRINT_C|CHAN_C|NONEOS_C,
  /* < */      PRINT_C|CHAN_C|NONEOS_C,
  /* = */      PRINT_C|CHAN_C|NONEOS_C,
  /* > */      PRINT_C|CHAN_C|NONEOS_C,
  /* ? */      PRINT_C|KWILD_C|CHAN_C|NONEOS_C,
  /* @ */      PRINT_C|KWILD_C|CHAN_C|NONEOS_C,
  /* A */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* B */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* C */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* D */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* E */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* F */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* G */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* H */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* I */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* J */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* K */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* L */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* M */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* N */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* O */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* P */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* Q */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* R */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* S */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* T */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* U */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* V */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* W */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* X */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* Y */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* Z */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* [ */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
  /* \ */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
  /* ] */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
  /* ^ */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
  /* _ */      PRINT_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
  /* ` */      PRINT_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
  /* a */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* b */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* c */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* d */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* e */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* f */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* g */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* h */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* i */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* j */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* k */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* l */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* m */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* n */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* o */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* p */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* q */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* r */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* s */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* t */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* u */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* v */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* w */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* x */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* y */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* z */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C|UID_C,
  /* { */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
  /* | */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
  /* } */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
  /* ~ */      PRINT_C|ALPHA_C|CHAN_C|NONEOS_C|USER_C,
  /* del  */   CHAN_C|NONEOS_C,
  /* 0x80 */   CHAN_C|NONEOS_C,
  /* 0x81 */   CHAN_C|NONEOS_C,
  /* 0x82 */   CHAN_C|NONEOS_C,
  /* 0x83 */   CHAN_C|NONEOS_C,
  /* 0x84 */   CHAN_C|NONEOS_C,
  /* 0x85 */   CHAN_C|NONEOS_C,
  /* 0x86 */   CHAN_C|NONEOS_C,
  /* 0x87 */   CHAN_C|NONEOS_C,
  /* 0x88 */   CHAN_C|NONEOS_C,
  /* 0x89 */   CHAN_C|NONEOS_C,
  /* 0x8A */   CHAN_C|NONEOS_C,
  /* 0x8B */   CHAN_C|NONEOS_C,
  /* 0x8C */   CHAN_C|NONEOS_C,
  /* 0x8D */   CHAN_C|NONEOS_C,
  /* 0x8E */   CHAN_C|NONEOS_C,
  /* 0x8F */   CHAN_C|NONEOS_C,
  /* 0x90 */   CHAN_C|NONEOS_C,
  /* 0x91 */   CHAN_C|NONEOS_C,
  /* 0x92 */   CHAN_C|NONEOS_C,
  /* 0x93 */   CHAN_C|NONEOS_C,
  /* 0x94 */   CHAN_C|NONEOS_C,
  /* 0x95 */   CHAN_C|NONEOS_C,
  /* 0x96 */   CHAN_C|NONEOS_C,
  /* 0x97 */   CHAN_C|NONEOS_C,
  /* 0x98 */   CHAN_C|NONEOS_C,
  /* 0x99 */   CHAN_C|NONEOS_C,
  /* 0x9A */   CHAN_C|NONEOS_C,
  /* 0x9B */   CHAN_C|NONEOS_C,
  /* 0x9C */   CHAN_C|NONEOS_C,
  /* 0x9D */   CHAN_C|NONEOS_C,
  /* 0x9E */   CHAN_C|NONEOS_C,
  /* 0x9F */   CHAN_C|NONEOS_C,
  /* 0xA0 */   CHAN_C|NONEOS_C,
  /* 0xA1 */   CHAN_C|NONEOS_C,
  /* 0xA2 */   CHAN_C|NONEOS_C,
  /* 0xA3 */   CHAN_C|NONEOS_C,
  /* 0xA4 */   CHAN_C|NONEOS_C,
  /* 0xA5 */   CHAN_C|NONEOS_C,
  /* 0xA6 */   CHAN_C|NONEOS_C,
  /* 0xA7 */   CHAN_C|NONEOS_C,
  /* 0xA8 */   CHAN_C|NONEOS_C,
  /* 0xA9 */   CHAN_C|NONEOS_C,
  /* 0xAA */   CHAN_C|NONEOS_C,
  /* 0xAB */   CHAN_C|NONEOS_C,
  /* 0xAC */   CHAN_C|NONEOS_C,
  /* 0xAD */   CHAN_C|NONEOS_C,
  /* 0xAE */   CHAN_C|NONEOS_C,
  /* 0xAF */   CHAN_C|NONEOS_C,
  /* 0xB0 */   CHAN_C|NONEOS_C,
  /* 0xB1 */   CHAN_C|NONEOS_C,
  /* 0xB2 */   CHAN_C|NONEOS_C,
  /* 0xB3 */   CHAN_C|NONEOS_C,
  /* 0xB4 */   CHAN_C|NONEOS_C,
  /* 0xB5 */   CHAN_C|NONEOS_C,
  /* 0xB6 */   CHAN_C|NONEOS_C,
  /* 0xB7 */   CHAN_C|NONEOS_C,
  /* 0xB8 */   CHAN_C|NONEOS_C,
  /* 0xB9 */   CHAN_C|NONEOS_C,
  /* 0xBA */   CHAN_C|NONEOS_C,
  /* 0xBB */   CHAN_C|NONEOS_C,
  /* 0xBC */   CHAN_C|NONEOS_C,
  /* ½ */      CHAN_C|NONEOS_C|UID_C,
  /* ¾ */      CHAN_C|NONEOS_C|UID_C,
  /* ¿ */      CHAN_C|NONEOS_C|UID_C,
  /* À */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Á */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Â */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ã */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ä */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Å */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Æ */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ç */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* È */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* É */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ê */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ë */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ì */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Í */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Î */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ï */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ð */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ñ */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ò */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ó */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ô */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Õ */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ö */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* × */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ø */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ù */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ú */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Û */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ü */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Ý */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* Þ */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ß */      CHAN_C|NONEOS_C|UID_C,
  /* à */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* á */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* â */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ã */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ä */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* å */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* æ */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ç */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* è */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* é */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ê */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ë */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ì */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* í */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* î */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ï */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ð */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ñ */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ò */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ó */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ô */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* õ */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ö */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ÷ */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ø */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ù */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ú */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* û */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ü */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* ý */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* þ */      CHAN_C|NONEOS_C|ALPHA_C|NICK_C|UID_C,
  /* 0xFF */   CHAN_C|NONEOS_C
};

/* -------------------------------------------------------------------------- *
 * Check for a valid hostname                                                 *
 * -------------------------------------------------------------------------- */
int chars_valid_host(const char *s)
{
  uint32_t dot = 0;
  
  if(s == NULL)
    return 0;
  
  if(*s == '.')
    return 0;
  
  while(*s)
  {
    if(!ishostchar(*s))
      return 0;
    
    if(*s == '.')
      dot++;
    
    s++;
  }
  
  return (dot > 0);
}

/* -------------------------------------------------------------------------- *
 * Check for a valid username                                                 *
 * -------------------------------------------------------------------------- */
int chars_valid_user(const char *s)
{
  if(s == NULL)
    return 0;
  
  if(*s == '~')
    s++;
  
  while(*s)
  {
    if(!isuserchar(*s))
      return 0;
    
    s++;
  }
  
  return 1;
}

/* -------------------------------------------------------------------------- *
 * Check for a valid nickname                                                 *
 * -------------------------------------------------------------------------- */
int chars_valid_nick(const char *s)
{
  if(s == NULL)
    return 0;
  
  if(isdigit(*s) || *s == '-')
    return 0;
  
  while(*s)
  {
    if(!isnickchar(*s))
      return 0;
    
    s++;
  }
  
  return 1;
}

/* -------------------------------------------------------------------------- *
 * Check for a valid channelname                                              *
 * -------------------------------------------------------------------------- */
int chars_valid_chan(const char *s)
{
  if(s == NULL)
    return 0;
  
  while(*s)
  {
    if(!ischanchar(*s))
      return 0;
    
    s++;
  }
  
  return 1;
}

