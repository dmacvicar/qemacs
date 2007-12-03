/*
 * Arabic algorithms for QEmacs.  
 *
 * Copyright (c) 2000 Fabrice Bellard.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "qe.h"
#include "qfribidi.h"

/* define it if your font handle more ligatures than the minimum
   required by unicode */

typedef struct ArabicChar {
    unsigned short ch, isolated, final, medial, initial;
} ArabicChar;

static const ArabicChar arabic_table[] = {
{ 0x0621, 0xfe80, 0x0000, 0x0000, 0x0000 },
{ 0x0622, 0xfe81, 0xfe82, 0x0000, 0x0000 },
{ 0x0623, 0xfe83, 0xfe84, 0x0000, 0x0000 },
{ 0x0624, 0xfe85, 0xfe86, 0x0000, 0x0000 },
{ 0x0625, 0xfe87, 0xfe88, 0x0000, 0x0000 },
{ 0x0626, 0xfe89, 0xfe8a, 0xfe8c, 0xfe8b },
{ 0x0627, 0xfe8d, 0xfe8e, 0x0000, 0x0000 },
{ 0x0628, 0xfe8f, 0xfe90, 0xfe92, 0xfe91 },
{ 0x0629, 0xfe93, 0xfe94, 0x0000, 0x0000 },
{ 0x062a, 0xfe95, 0xfe96, 0xfe98, 0xfe97 },
{ 0x062b, 0xfe99, 0xfe9a, 0xfe9c, 0xfe9b },
{ 0x062c, 0xfe9d, 0xfe9e, 0xfea0, 0xfe9f },
{ 0x062d, 0xfea1, 0xfea2, 0xfea4, 0xfea3 },
{ 0x062e, 0xfea5, 0xfea6, 0xfea8, 0xfea7 },
{ 0x062f, 0xfea9, 0xfeaa, 0x0000, 0x0000 },
{ 0x0630, 0xfeab, 0xfeac, 0x0000, 0x0000 },
{ 0x0631, 0xfead, 0xfeae, 0x0000, 0x0000 },
{ 0x0632, 0xfeaf, 0xfeb0, 0x0000, 0x0000 },
{ 0x0633, 0xfeb1, 0xfeb2, 0xfeb4, 0xfeb3 },
{ 0x0634, 0xfeb5, 0xfeb6, 0xfeb8, 0xfeb7 },
{ 0x0635, 0xfeb9, 0xfeba, 0xfebc, 0xfebb },
{ 0x0636, 0xfebd, 0xfebe, 0xfec0, 0xfebf },
{ 0x0637, 0xfec1, 0xfec2, 0xfec4, 0xfec3 },
{ 0x0638, 0xfec5, 0xfec6, 0xfec8, 0xfec7 },
{ 0x0639, 0xfec9, 0xfeca, 0xfecc, 0xfecb },
{ 0x063a, 0xfecd, 0xfece, 0xfed0, 0xfecf },

{ 0x0640, 0x0640, 0x0640, 0x0640, 0x0640 },
{ 0x0641, 0xfed1, 0xfed2, 0xfed4, 0xfed3 },
{ 0x0642, 0xfed5, 0xfed6, 0xfed8, 0xfed7 },
{ 0x0643, 0xfed9, 0xfeda, 0xfedc, 0xfedb },
{ 0x0644, 0xfedd, 0xfede, 0xfee0, 0xfedf },
{ 0x0645, 0xfee1, 0xfee2, 0xfee4, 0xfee3 },
{ 0x0646, 0xfee5, 0xfee6, 0xfee8, 0xfee7 },
{ 0x0647, 0xfee9, 0xfeea, 0xfeec, 0xfeeb },
{ 0x0648, 0xfeed, 0xfeee, 0x0000, 0x0000 },
{ 0x0649, 0xfeef, 0xfef0, 0x0000, 0x0000 },
{ 0x064a, 0xfef1, 0xfef2, 0xfef4, 0xfef3 },

{ 0x0671, 0xfb50, 0x0000, 0x0000, 0x0000 },
{ 0x0672, 0x0672, 0x0672, 0x0000, 0x0000 },
{ 0x0673, 0x0673, 0x0673, 0x0000, 0x0000 },
{ 0x0674, 0x0674, 0x0000, 0x0000, 0x0000 },
{ 0x0675, 0x0675, 0x0675, 0x0000, 0x0000 },
{ 0x0676, 0x0676, 0x0676, 0x0000, 0x0000 },
{ 0x0677, 0xfbdd, 0x0677, 0x0000, 0x0000 },
{ 0x0678, 0x0678, 0x0678, 0x0678, 0x0678 },
{ 0x0679, 0xfb66, 0xfb67, 0xfb69, 0xfb68 },
{ 0x067a, 0xfb5e, 0xfb5f, 0xfb61, 0xfb60 },
{ 0x067b, 0xfb52, 0xfb53, 0xfb55, 0xfb54 },
{ 0x067c, 0x067c, 0x067c, 0x067c, 0x067c },
{ 0x067d, 0x067d, 0x067d, 0x067d, 0x067d },
{ 0x067e, 0xfb56, 0xfb57, 0xfb59, 0xfb58 },
{ 0x067f, 0xfb62, 0xfb63, 0xfb65, 0xfb64 },
{ 0x0680, 0xfb5a, 0xfb5b, 0xfb5d, 0xfb5c },
{ 0x0681, 0x0681, 0x0681, 0x0681, 0x0681 },
{ 0x0682, 0x0682, 0x0682, 0x0682, 0x0682 },
{ 0x0683, 0xfb76, 0xfb77, 0xfb79, 0xfb78 },
{ 0x0684, 0xfb72, 0xfb73, 0xfb75, 0xfb74 },
{ 0x0685, 0x0685, 0x0685, 0x0685, 0x0685 },
{ 0x0686, 0xfb7a, 0xfb7b, 0xfb7d, 0xfb7c },
{ 0x0687, 0xfb7e, 0xfb7f, 0xfb81, 0xfb80 },
{ 0x0688, 0xfb88, 0xfb89, 0x0000, 0x0000 },
{ 0x0689, 0x0689, 0x0689, 0x0000, 0x0000 },
{ 0x068a, 0x068a, 0x068a, 0x0000, 0x0000 },
{ 0x068b, 0x068b, 0x068b, 0x0000, 0x0000 },
{ 0x068c, 0xfb84, 0xfb85, 0x0000, 0x0000 },
{ 0x068d, 0xfb82, 0xfb83, 0x0000, 0x0000 },
{ 0x068e, 0xfb86, 0xfb87, 0x0000, 0x0000 },
{ 0x068f, 0x068f, 0x068f, 0x0000, 0x0000 },
{ 0x0690, 0x0690, 0x0690, 0x0000, 0x0000 },
{ 0x0691, 0xfb8c, 0xfb8d, 0x0000, 0x0000 },
{ 0x0692, 0x0692, 0x0692, 0x0000, 0x0000 },
{ 0x0693, 0x0693, 0x0693, 0x0000, 0x0000 },
{ 0x0694, 0x0694, 0x0694, 0x0000, 0x0000 },
{ 0x0695, 0x0695, 0x0695, 0x0000, 0x0000 },
{ 0x0696, 0x0695, 0x0696, 0x0000, 0x0000 },
{ 0x0697, 0x0697, 0x0697, 0x0000, 0x0000 },
{ 0x0698, 0xfb8a, 0xfb8b, 0x0000, 0x0000 },
{ 0x0699, 0x0699, 0x0699, 0x0000, 0x0000 },
{ 0x069a, 0x069a, 0x069a, 0x069a, 0x069a },
{ 0x069b, 0x069b, 0x069b, 0x069b, 0x069b },
{ 0x069c, 0x069c, 0x069c, 0x069c, 0x069c },
{ 0x069d, 0x069d, 0x069d, 0x069d, 0x069d },
{ 0x069e, 0x069e, 0x069e, 0x069e, 0x069e },
{ 0x069f, 0x069f, 0x069f, 0x069f, 0x069f },
{ 0x06a0, 0x06a0, 0x06a0, 0x06a0, 0x06a0 },
{ 0x06a1, 0x06a1, 0x06a1, 0x06a1, 0x06a1 },
{ 0x06a2, 0x06a2, 0x06a2, 0x06a2, 0x06a2 },
{ 0x06a3, 0x06a3, 0x06a3, 0x06a3, 0x06a3 },
{ 0x06a4, 0xfb6a, 0xfb6b, 0xfb6d, 0xfb6c },
{ 0x06a5, 0x06a5, 0x06a5, 0x06a5, 0x06a5 },
{ 0x06a6, 0xfb6e, 0xfb6f, 0xfb71, 0xfb70 },
{ 0x06a7, 0x06a7, 0x06a7, 0x06a7, 0x06a7 },
{ 0x06a8, 0x06a8, 0x06a8, 0x06a8, 0x06a8 },
{ 0x06a9, 0xfb8e, 0xfb8f, 0xfb91, 0xfb90 },
{ 0x06aa, 0x06aa, 0x06aa, 0x06aa, 0x06aa },
{ 0x06ab, 0x06ab, 0x06ab, 0x06ab, 0x06ab },
{ 0x06ac, 0x06ac, 0x06ac, 0x06ac, 0x06ac },
{ 0x06ad, 0xfbd3, 0xfbd4, 0xfbd6, 0xfbd5 },
{ 0x06ae, 0x06ae, 0x06ae, 0x06ae, 0x06ae },
{ 0x06af, 0xfb92, 0xfb93, 0xfb95, 0xfb94 },
{ 0x06b0, 0x06b0, 0x06b0, 0x06b0, 0x06b0 },
{ 0x06b1, 0xfb9a, 0xfb9b, 0xfb9d, 0xfb9c },
{ 0x06b2, 0x06b2, 0x06b2, 0x06b2, 0x06b2 },
{ 0x06b3, 0xfb96, 0xfb97, 0xfb99, 0xfb98 },
{ 0x06b4, 0x06b4, 0x06b4, 0x06b4, 0x06b4 },
{ 0x06b5, 0x06b5, 0x06b5, 0x06b5, 0x06b5 },
{ 0x06b6, 0x06b6, 0x06b6, 0x06b6, 0x06b6 },
{ 0x06b7, 0x06b7, 0x06b7, 0x06b7, 0x06b7 },
{ 0x06ba, 0xfb9e, 0xfb9f, 0x06ba, 0x06ba },
{ 0x06bb, 0xfba0, 0xfba1, 0xfba3, 0xfba2 },
{ 0x06bc, 0x06bc, 0x06bc, 0x06bc, 0x06bc },
{ 0x06bd, 0x06bd, 0x06bd, 0x06bd, 0x06bd },
{ 0x06be, 0xfbaa, 0xfbab, 0xfbad, 0xfbac },
{ 0x06c0, 0xfba4, 0xfba5, 0x0000, 0x0000 },
{ 0x06c1, 0xfba6, 0xfba7, 0xfba9, 0xfba8 },
{ 0x06c2, 0x06c2, 0x06c2, 0x0000, 0x0000 },
{ 0x06c3, 0x06c3, 0x06c3, 0x0000, 0x0000 },
{ 0x06c4, 0x06c4, 0x06c4, 0x0000, 0x0000 },
{ 0x06c5, 0xfbe0, 0xfbe1, 0x0000, 0x0000 },
{ 0x06c6, 0xfbd9, 0xfbda, 0x0000, 0x0000 },
{ 0x06c7, 0xfbd7, 0xfbd8, 0x0000, 0x0000 },
{ 0x06c8, 0xfbdb, 0xfbdc, 0x0000, 0x0000 },
{ 0x06c9, 0xfbe2, 0xfbe3, 0x0000, 0x0000 },
{ 0x06ca, 0x06ca, 0x06ca, 0x0000, 0x0000 },
{ 0x06cb, 0xfbde, 0xfbdf, 0x0000, 0x0000 },
{ 0x06cc, 0xfbfc, 0xfbfd, 0xfbff, 0xfbfe },
{ 0x06cd, 0x06cd, 0x06cd, 0x0000, 0x0000 },
{ 0x06ce, 0x06ce, 0x06ce, 0x06ce, 0x06ce },

    /* XXX: incorrect ? */
{ 0x06d0, 0xfbe4, 0xfbe5, 0xfbe7, 0xfbe6 },
{ 0x06c1, 0x06c1, 0x06c1, 0x06c1, 0x06c1 },
{ 0x06c2, 0x06c2, 0x06c2, 0x0000, 0x0000 },
{ 0x06c3, 0x06c3, 0x06c3, 0x0000, 0x0000 },
{ 0x06c4, 0x06c4, 0x06c4, 0x0000, 0x0000 },
{ 0x06c5, 0x06c5, 0x06c5, 0x0000, 0x0000 },
{ 0x06c6, 0x06c6, 0x06c6, 0x0000, 0x0000 },
{ 0x06c7, 0x06c7, 0x06c7, 0x0000, 0x0000 },
{ 0x06c8, 0x06c8, 0x06c8, 0x0000, 0x0000 },
{ 0x06c9, 0x06c9, 0x06c9, 0x0000, 0x0000 },
{ 0x06ca, 0x06ca, 0x06ca, 0x0000, 0x0000 },
{ 0x06cb, 0x06cb, 0x06cb, 0x0000, 0x0000 },
{ 0x06cc, 0x06cc, 0x06cc, 0x06cc, 0x06cc },
{ 0x06cd, 0x06cd, 0x06cd, 0x0000, 0x0000 },
{ 0x06ce, 0x06ce, 0x06ce, 0x06ce, 0x06ce },
{ 0x06d0, 0x06d0, 0x06d0, 0x06d0, 0x06d0 },
{ 0x06d1, 0x06d1, 0x06d1, 0x06d1, 0x06d1 },
{ 0x06d2, 0xfbae, 0xfbaf, 0x0000, 0x0000 },
{ 0x06d3, 0xfbb0, 0xfbb1, 0x0000, 0x0000 },
{ 0x06d5, 0x06d5, 0x0000, 0x0000, 0x0000 },
{ 0x200d, 0x200d, 0x200d, 0x200d, 0x200d },
{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
};

static const unsigned short transparent[] = {
0x064b, 0x064c, 0x064d, 0x064e, 0x064f, 0x0650, 
0x0670, 
0x06d7, 0x06d8, 0x06d9, 0x06da, 0x06db, 0x06dc, 0x06df, 
0x06e0, 0x06e1, 0x06e2, 0x06e3, 0x06e4, 0x06e7, 0x06e8, 0x06ea, 0x06eb, 0x06ec, 
0x06ed,
};

/* XXX: optimize tables, use binary search */
static int is_transparent(int ch)
{
    int i;

    for (i = 0; i < (int)(sizeof(transparent)/sizeof(transparent[0])); i++) {
        if (transparent[i] == ch)
            return 1;
    }
    return 0;
}

static const ArabicChar *find_char(int ch)
{
    const ArabicChar *c;

    c = arabic_table;
    while (c->ch != 0 && c->ch != ch)
        c++;
    return c;
}

/* ctog is NOT filled because it is not needed. We put it for homogoneity */
int arab_join(unsigned int *line, __unused__ unsigned int *ctog, int len)
{
    int a, b, c, i, j, res;
    const ArabicChar *aa, *bb, *cc;

    a = 0;
    for (i = 0; i < len;) {
        j = i;
        b = line[i++];
        /* find the next non transparent char */
        c = 0;
        while (i < len) {
            c = line[i];
            if (!is_transparent(c))
                break;
            i++;
        }
        /* apply Unicode Arabic substitution rules */
        aa = find_char(a); /* previous */
        bb = find_char(b); /* current */
        cc = find_char(c); /* next */

        if (a && cc->final && bb->medial) {
            res = bb->medial;
        } else if (cc->final && bb->initial) {
            res = bb->initial;
        } else if (a && bb->final) {
            res = bb->final;
        } else if (bb->isolated) {
            res = bb->isolated;
        } else {
            res = b;
        }
        line[j] = res;

        a = 0;
        if (bb->initial && cc->final)
            a = cc->final;
    }
    return len;
}

