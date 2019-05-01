#include "diablo.h"

DEVILUTION_BEGIN_NAMESPACE

int qtexty; // weak
char *qtextptr;
int qtextSpd;     // weak
char qtextflag;   // weak
int scrolltexty;  // weak
int sgLastScroll; // weak
void *pMedTextCels;
void *pTextBoxCels;

const unsigned char mfontframe[127] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 37, 49, 38, 0, 39, 40, 47,
	42, 43, 41, 45, 52, 44, 53, 55, 36, 27,
	28, 29, 30, 31, 32, 33, 34, 35, 51, 50,
	48, 46, 49, 54, 0, 1, 2, 3, 4, 5,
	6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
	26, 42, 0, 43, 0, 0, 0, 1, 2, 3,
	4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 48, 0, 49, 0
};
const unsigned char mfontkern[56] = {
	5, 15, 10, 13, 14, 10, 9, 13, 11, 5,
	5, 11, 10, 16, 13, 16, 10, 15, 12, 10,
	14, 17, 17, 22, 17, 16, 11, 5, 11, 11,
	11, 10, 11, 11, 11, 11, 15, 5, 10, 18,
	15, 8, 6, 6, 7, 10, 9, 6, 10, 10,
	5, 5, 5, 5, 11, 12
};

/* data */

int qscroll_spd_tbl[9] = { 2, 4, 6, 8, 0, -1, -2, -3, -4 };

void FreeQuestText()
{
	MemFreeDbg(pMedTextCels);
	MemFreeDbg(pTextBoxCels);
}

void InitQuestText()
{
	pMedTextCels = LoadFileInMem("Data\\MedTextS.CEL", 0);
	pTextBoxCels = LoadFileInMem("Data\\TextBox.CEL", 0);
	qtextflag = FALSE;
}
// 646D00: using guessed type char qtextflag;

void InitQTextMsg(int m)
{
	if (alltext[m].scrlltxt) {
		questlog = FALSE;
		qtextptr = alltext[m].txtstr;
		qtextflag = TRUE;
		qtexty = 500;
		sgLastScroll = qscroll_spd_tbl[alltext[m].txtspd - 1]; /* double check offset */
		scrolltexty = sgLastScroll;
		qtextSpd = GetTickCount();
	}
	PlaySFX(alltext[m].sfxnr);
}
// 646CF4: using guessed type int qtexty;
// 646CFC: using guessed type int qtextSpd;
// 646D00: using guessed type char qtextflag;
// 646D04: using guessed type int scrolltexty;
// 646D08: using guessed type int sgLastScroll;

void DrawQTextBack()
{
	CelDecodeOnly(88, 487, (BYTE *)pTextBoxCels, 1, 591);

#define TRANS_RECT_X 27
#define TRANS_RECT_Y 28
#define TRANS_RECT_WIDTH 585
#define TRANS_RECT_HEIGHT 297
#include "asm_trans_rect.inc"
}

void PrintQTextChr(int sx, int sy, BYTE *pCelBuff, int nCel)
{
	BYTE *dst, *pStart, *pEnd, *end;

	/// ASSERT: assert(gpBuffer);

	dst = &gpBuffer[sx + PitchTbl[sy]];
	pStart = &gpBuffer[PitchTbl[209]];
	pEnd = &gpBuffer[PitchTbl[469]];

#ifdef USE_ASM
	__asm {
		mov		ebx, pCelBuff
		mov		eax, nCel
		shl		eax, 2
		add		ebx, eax
		mov		eax, [ebx+4]
		sub		eax, [ebx]
		mov		end, eax
		mov		esi, pCelBuff
		add		esi, [ebx]
		mov		edi, dst
		mov		ebx, end
		add		ebx, esi
	label1:
		mov		edx, 22
	label2:
		xor		eax, eax
		lodsb
		or		al, al
		js		label7
		sub		edx, eax
		cmp		edi, pStart
		jb		label5
		cmp		edi, pEnd
		ja		label5
		mov		ecx, eax
		shr		ecx, 1
		jnb		label3
		movsb
		jecxz	label6
	label3:
		shr		ecx, 1
		jnb		label4
		movsw
		jecxz	label6
	label4:
		rep movsd
		jmp		label6
	label5:
		add		esi, eax
		add		edi, eax
	label6:
		or		edx, edx
		jz		label8
		jmp		label2
	label7:
		neg		al
		add		edi, eax
		sub		edx, eax
		jnz		label2
	label8:
		sub		edi, 768 + 22
		cmp		ebx, esi
		jnz		label1
	}
#else
	int i;
	BYTE width;
	BYTE *src;
	DWORD *pFrameTable;

	pFrameTable = (DWORD *)&pCelBuff[4 * nCel];
	src = &pCelBuff[pFrameTable[0]];
	end = &src[pFrameTable[1] - pFrameTable[0]];

	for (; src != end; dst -= 768 + 22) {
		for (i = 22; i;) {
			width = *src++;
			if (!(width & 0x80)) {
				i -= width;
				if (dst >= pStart && dst <= pEnd) {
					if (width & 1) {
						dst[0] = src[0];
						src++;
						dst++;
					}
					width >>= 1;
					if (width & 1) {
						dst[0] = src[0];
						dst[1] = src[1];
						src += 2;
						dst += 2;
					}
					width >>= 1;
					for (; width; width--) {
						dst[0] = src[0];
						dst[1] = src[1];
						dst[2] = src[2];
						dst[3] = src[3];
						src += 4;
						dst += 4;
					}
				} else {
					src += width;
					dst += width;
				}
			} else {
				width = -(char)width;
				dst += width;
				i -= width;
			}
		}
	}
#endif
}

void DrawQText()
{
	int i, l, w, tx, ty;
	BYTE c;
	char *p, *pnl, *s;
	char tempstr[128];
	BOOL doneflag;
	DWORD currTime;

	DrawQTextBack();

	p = qtextptr;
	pnl = NULL;
	tx = 112;
	ty = qtexty;

	doneflag = FALSE;
	while(!doneflag) {
		w = 0;
		s = p;
		l = 0;
		while(*s != '\n' && *s != '|' && w < 543) {
			c = gbFontTransTbl[(BYTE)*s];
			s++;
			if(c != '\0') {
				tempstr[l] = c;
				w += mfontkern[mfontframe[c]] + 2;
			} else {
				l--;
			}
			l++;
		}
		tempstr[l] = '\0';
		if(*s == '|') {
			tempstr[l] = '\0';
			doneflag = TRUE;
		} else if(*s != '\n') {
			while(tempstr[l] != ' ' && l > 0) {
				tempstr[l] = '\0';
				l--;
			}
		}
		for(i = 0; tempstr[i]; i++) {
			p++;
			c = mfontframe[gbFontTransTbl[(BYTE)tempstr[i]]];
			if(*p == '\n') {
				p++;
			}
			if(c != 0) {
				PrintQTextChr(tx, ty, (BYTE *)pMedTextCels, c);
			}
			tx += mfontkern[c] + 2;
		}
		if(pnl == NULL) {
			pnl = p;
		}
		tx = 112;
		ty += 38;
		if(ty > 501) {
			doneflag = TRUE;
		}
	}

	currTime = GetTickCount();
	while(1) {
		if(sgLastScroll <= 0) {
			qtexty--;
			qtexty += sgLastScroll;
		} else {
			scrolltexty--;
			if(scrolltexty != 0) {
				qtexty--;
			}
		}
		if(scrolltexty == 0) {
			scrolltexty = sgLastScroll;
		}
		if(qtexty <= 209) {
			qtexty += 38;
			qtextptr = pnl;
			if(*pnl == '|') {
				qtextflag = 0;
			}
			break;
		}
		qtextSpd += 50;
		if(currTime - qtextSpd >= 0x7FFFFFFF) {
			break;
		}
	}
}
// 646CF4: using guessed type int qtexty;
// 646CFC: using guessed type int qtextSpd;
// 646D00: using guessed type char qtextflag;
// 646D04: using guessed type int scrolltexty;
// 646D08: using guessed type int sgLastScroll;
// 428202: using guessed type char qstr[128];

DEVILUTION_END_NAMESPACE
