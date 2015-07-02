/*
 * JAWS.c -- Code for Windows front end to XBoard to use it with JAWS
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Free Software Foundation, Inc.
 *
 * XBoard borrows its colors and the bitmaps.xchess bitmap set from XChess,
 * which was written and is copyrighted by Wayne Christopher.
 *
 * The following terms apply to Digital Equipment Corporation's copyright
 * interest in XBoard:
 * ------------------------------------------------------------------------
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * ------------------------------------------------------------------------
 *
 * The following terms apply to the enhanced version of XBoard
 * distributed by the Free Software Foundation:
 * ------------------------------------------------------------------------
 *
 * GNU XBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU XBoard is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.  *
 *
 *------------------------------------------------------------------------
 ** See the file ChangeLog for a revision history.  */

// This file collects all patches for the JAWS version, so they can all be included in winboard.c
// in one big swoop. At the bottom of this file you can read instructions for how to patch
// WinBoard to work with JAWS with the aid of this file. Note that the code in this file
// is for WinBoard 4.3 and later; for older WB versions you would have to throw out the
// piece names for all pieces from Guard to Unicorn, #define ONE as '1', AAA as 'a',
// BOARD_LEFT as 0, BOARD_RGHT and BOARD_HEIGHT as 8, and set holdingssizes to 0.
// You will need to build with jaws.rc in stead of winboard.rc.

// from resource.h

#define IDM_PossibleAttackMove          1800
#define IDM_PossibleAttacked            1801
#define IDM_SayMachineMove              1802
#define IDM_ReadRow                     1803
#define IDM_ReadColumn                  1804
#define IDM_SayCurrentPos               1805
#define IDM_SayAllBoard                 1806
#define IDM_SayUpperDiagnols            1807
#define IDM_SayLowerDiagnols            1808
#define IDM_SayClockTime                1810
#define IDM_SayWhosTurn                 1811
#define IDM_SayKnightMoves              1812
#define ID_SHITTY_HI                    1813
#define IDM_SayWhitePieces              1816
#define IDM_SayBlackPieces              1817


// from winboard.c: all new routines
#define JFWAPI __declspec(dllimport)
JFWAPI BOOL WINAPI JFWSayString (LPCTSTR lpszStrinToSpeak, BOOL bInterrupt);

typedef JFWAPI BOOL (WINAPI *PSAYSTRING)(LPCTSTR lpszStrinToSpeak, BOOL bInterrupt);

PSAYSTRING RealSayString;

VOID SayString(char *mess, BOOL flag)
{ // for debug file
	static char buf[8000], *p;
        int l = strlen(buf);
	if(appData.debugMode) fprintf(debugFP, "SAY '%s'\n", mess);
        if(l) buf[l++] = ' '; // separate by space from previous
	safeStrCpy(buf+l, _(mess), 8000-1-l); // buffer
        if(!flag) return; // wait for flush
	if(p = StrCaseStr(buf, "Xboard adjudication:")) {
		int i;
		for(i=19; i>1; i--) p[i] = p[i-1];
		p[1] = ' ';
	}
	RealSayString(buf, !strcmp(mess, " ")); // kludge to indicate flushing of interruptable speach
	if(appData.debugMode) fprintf(debugFP, "SPEAK '%s'\n", buf);
	buf[0] = NULLCHAR;
}

//static int fromX = 0, fromY = 0;
static int oldFromX, oldFromY;
static int timeflag;
static int suppressClocks = 0;
static int suppressOneKey = 0;
static HANDLE hAccelJAWS;

typedef struct { char *name; int code; } MenuItemDesc;

MenuItemDesc menuItemJAWS[] = {
{"Say Clock &Time\tAlt+T",      IDM_SayClockTime },
{"-", 0 },
{"Say Last &Move\tAlt+M",       IDM_SayMachineMove },
{"Say W&ho's Turn\tAlt+X",      IDM_SayWhosTurn },
{"-", 0 },
{"Say Complete &Position\tAlt+P",IDM_SayAllBoard },
{"Say &White Pieces\tAlt+W",    IDM_SayWhitePieces },
{"Say &Black Pieces\tAlt+B",    IDM_SayBlackPieces },
{"Say Board &Rank\tAlt+R",      IDM_ReadRow },
{"Say Board &File\tAlt+F",      IDM_ReadColumn },
{"-", 0 },
{"Say &Upper Diagonals\tAlt+U",  IDM_SayUpperDiagnols },
{"Say &Lower Diagonals\tAlt+L",  IDM_SayLowerDiagnols },
{"Say K&night Moves\tAlt+N",    IDM_SayKnightMoves },
{"Say Current &Square\tAlt+S",  IDM_SayCurrentPos },
{"Say &Attacks\tAlt+A",         IDM_PossibleAttackMove },
{"Say Attacke&d\tAlt+D",        IDM_PossibleAttacked },
{NULL, 0}
};

ACCEL acceleratorsJAWS[] = {
{FVIRTKEY|FALT, 'T', IDM_SayClockTime },
{FVIRTKEY|FALT, 'M', IDM_SayMachineMove },
{FVIRTKEY|FALT, 'X', IDM_SayWhosTurn },
{FVIRTKEY|FALT, 'P', IDM_SayAllBoard },
{FVIRTKEY|FALT, 'W', IDM_SayWhitePieces },
{FVIRTKEY|FALT, 'B', IDM_SayBlackPieces },
{FVIRTKEY|FALT, 'R', IDM_ReadRow },
{FVIRTKEY|FALT, 'F', IDM_ReadColumn },
{FVIRTKEY|FALT, 'U', IDM_SayUpperDiagnols },
{FVIRTKEY|FALT, 'L', IDM_SayLowerDiagnols },
{FVIRTKEY|FALT, 'N', IDM_SayKnightMoves },
{FVIRTKEY|FALT, 'S', IDM_SayCurrentPos },
{FVIRTKEY|FALT, 'A', IDM_PossibleAttackMove },
{FVIRTKEY|FALT, 'D', IDM_PossibleAttacked }
};

void
AdaptMenu()
{
	HMENU menuMain, menuJAWS;
	MENUBARINFO helpMenuInfo;
	int i;

	helpMenuInfo.cbSize = sizeof(helpMenuInfo);
	menuMain = GetMenu(hwndMain);
	menuJAWS = CreatePopupMenu();

	for(i=0; menuItemJAWS[i].name; i++) {
	    if(menuItemJAWS[i].name[0] == '-')
		 AppendMenu(menuJAWS, MF_SEPARATOR, (UINT_PTR) 0, NULL);
	    else AppendMenu(menuJAWS, MF_ENABLED|MF_STRING,
			(UINT_PTR) menuItemJAWS[i].code, (LPCTSTR) _(menuItemJAWS[i].name));
	}
	InsertMenu(menuMain, 7, MF_BYPOSITION|MF_POPUP|MF_ENABLED|MF_STRING,
		(UINT_PTR) menuJAWS, "&JAWS");
	oldMenuItemState[8] = oldMenuItemState[7];
	DrawMenuBar(hwndMain);
}

BOOL
InitJAWS()
{	// to be called at beginning of WinMain, after InitApplication and InitInstance
	HINSTANCE hApi = LoadLibrary("jfwapi32.dll");
	if(!hApi) {
		DisplayInformation("Missing jfwapi32.dll");
		return (FALSE);
	}

	RealSayString = (PSAYSTRING)GetProcAddress(hApi, "JFWSayString");
	if(!RealSayString) {
		DisplayInformation("SayString returned a null pointer");
		return (FALSE);
	}

	{
		// [HGM] kludge to reduce need for modification of winboard.c: make tinyLayout menu identical
		// to standard layout, so that code for switching between them does not have to be deleted
		int i;

		AdaptMenu();
		menuBarText[0][8] = menuBarText[0][7]; menuBarText[0][7] = "&JAWS";
		for(i=0; i<9; i++) menuBarText[1][i] = menuBarText[0][i];
	}

	hAccelJAWS = CreateAcceleratorTable(acceleratorsJAWS, 14);

	/* initialize cursor position */
	fromX = fromY = 0;
	SetHighlights(fromX, fromY, -1, -1);
	DrawPosition(FALSE, NULL);
	oldFromX = oldFromY = -1;

	if(hwndConsole) SetFocus(hwndConsole);
	return TRUE;
}

int beeps[] = { 1, 0, 0, 0, 0 };
int beepCodes[] = { 0, MB_OK, MB_ICONERROR, MB_ICONQUESTION, MB_ICONEXCLAMATION, MB_ICONASTERISK };
static int dropX = -1, dropY = -1;

VOID
KeyboardEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ChessSquare currentPiece;
	char *piece, *xchar, *ynum ;
	int n, beepType = 1; // empty beep

	if(fromX == -1 || fromY == -1) { // if we just dropped piece, stay at that square
		fromX = dropX; fromY = dropY;
		dropX = dropY = -1; // but only once
        }
	if(fromX == -1 || fromY == -1) {
		fromX = BOARD_LEFT; fromY = 0;
        }
	switch(wParam) {
	case VK_LEFT:
		if(fromX == BOARD_RGHT+1) fromX -= 2; else
		if(fromX == BOARD_LEFT) { if(fromY >= BOARD_HEIGHT - gameInfo.holdingsSize) fromX -= 2; else beepType = 0; } else
		if(fromX > BOARD_LEFT) fromX--; else beepType = 0; // off-board beep
		break;
	case VK_RIGHT:
		if(fromX == BOARD_LEFT-2) fromX += 2; else
		if(fromX == BOARD_RGHT-1) { if(fromY < gameInfo.holdingsSize) fromX += 2; else beepType = 0; } else
		if(fromX < BOARD_RGHT-1) fromX++; else beepType = 0;
		break;
	case VK_UP:
		if(fromX == BOARD_RGHT+1) { if(fromY < gameInfo.holdingsSize - 1) fromY++; else beepType = 0; } else
		if(fromY < BOARD_HEIGHT-1) fromY++; else beepType = 0;
		break;
	case VK_DOWN:
		if(fromX == BOARD_LEFT-2) { if(fromY > BOARD_HEIGHT - gameInfo.holdingsSize) fromY--; else beepType = 0; } else
		if(fromY > 0) fromY--; else beepType = 0;
		break;
	}
	SetHighlights(fromX, fromY, -1, -1);
	DrawPosition(FALSE, NULL);
	currentPiece = boards[currentMove][fromY][fromX];
	piece = PieceToName(currentPiece,1);
	if(beepType == 1 && currentPiece != EmptySquare) beepType = currentPiece < (int) BlackPawn ? 2 : 3; // white or black beep
	if(beeps[beepType] == beeps[1] && (fromX == BOARD_RGHT+1 || fromX == BOARD_LEFT-2)) beepType = 4; // holdings beep
	beepType = beeps[beepType]%6;
	if(beepType) MessageBeep(beepCodes[beepType]);
	if(fromX == BOARD_LEFT - 2) {
		SayString("black holdings", FALSE);
		if(currentPiece != EmptySquare) {
			char buf[MSG_SIZ];
			n = boards[currentMove][fromY][1];
			snprintf(buf, MSG_SIZ, "%d %s%s", n, PieceToName(currentPiece,0), n == 1 ? "" : "s");
			SayString(buf, FALSE);
		}
		SayString(" ", TRUE);
	} else
	if(fromX == BOARD_RGHT + 1) {
		SayString("white holdings", FALSE);
		if(currentPiece != EmptySquare) {
			char buf[MSG_SIZ];
			n = boards[currentMove][fromY][BOARD_WIDTH-2];
			snprintf(buf, MSG_SIZ,"%d %s%s", n, PieceToName(currentPiece,0), n == 1 ? "" : "s");
			SayString(buf, FALSE);
		}
		SayString(" ", TRUE);
	} else
	if(fromX >= BOARD_LEFT && fromX < BOARD_RGHT) {
		char buf[MSG_SIZ];
		xchar = SquareToChar(fromX);
		ynum = SquareToNum(fromY);
		if(currentPiece != EmptySquare) {
		  snprintf(buf, MSG_SIZ, "%s %s %s", xchar, ynum, piece);
		} else snprintf(buf, MSG_SIZ, "%s %s", xchar, ynum);
		SayString(buf, FALSE);
		SayString(" ", TRUE);
	}
	return;
}


VOID
Toggle(Boolean *b, char *mess)
{
	*b = !*b;
	SayString(mess, FALSE);
	SayString("is now", FALSE);
	SayString(*b ? "on" : "off", FALSE);
	SayString("", TRUE); // flush
}

/* handles keyboard moves in a click-click fashion */
VOID
KeyboardMove(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ChessSquare currentpiece;
	char *piece;

	static BOOLEAN sameAgain = FALSE;
	switch (message) {
	case WM_KEYDOWN:
		sameAgain = FALSE;
		if(oldFromX == fromX && oldFromY == fromY) {
			sameAgain = TRUE;
			/* click on same square */
			break;
		}
		else if(oldFromX != -1) {

			ChessSquare pdown, pup;
      pdown = boards[currentMove][oldFromY][oldFromX];
      pup = boards[currentMove][fromY][fromX];

		if (gameMode == EditPosition ||
			!((WhitePawn <= pdown && pdown <= WhiteKing &&
				 WhitePawn <= pup && pup <= WhiteKing) ||
				(BlackPawn <= pdown && pdown <= BlackKing &&
				 BlackPawn <= pup && pup <= BlackKing))) {
			/* EditPosition, empty square, or different color piece;
			click-click move is possible */
			char promoChoice = NULLCHAR;

			if (HasPromotionChoice(oldFromX, oldFromY, fromX, fromY, &promoChoice)) {
				if (appData.alwaysPromoteToQueen) {
					UserMoveEvent(oldFromX, oldFromY, fromX, fromY, 'q');
				}
				else {
					toX = fromX; toY = fromY; fromX = oldFromX; fromY = oldFromY;
					PromotionPopup(hwnd);
					fromX = toX; fromY = toY;
				}
			}
			else {
				UserMoveEvent(oldFromX, oldFromY, fromX, fromY, promoChoice);
			}
		oldFromX = oldFromY = -1;
		break;
		}

		}
		/* First downclick, or restart on a square with same color piece */
		if (OKToStartUserMove(fromX, fromY)) {
		oldFromX = fromX;
		oldFromY = fromY;
		currentpiece = boards[currentMove][fromY][fromX];
		piece = PieceToName(currentpiece,1);
		SayString(piece, FALSE);
		SayString("selected", TRUE);
		}
		else {
		oldFromX = oldFromY = -1;
		}
		break;

	case WM_KEYUP:
		if (oldFromX == fromX && oldFromY == fromY) {
      /* Upclick on same square */
      if (sameAgain) {
	/* Clicked same square twice: abort click-click move */
			oldFromX = oldFromY = -1;
			currentpiece = boards[currentMove][fromY][fromX];
			piece = PieceToName(currentpiece,0);
			SayString(piece, FALSE);
			SayString("unselected", TRUE);
			}
		}
	}
}

int
NiceTime(int x)
{	// return TRUE for times we want to announce
	if(x<0) return 0;
	x = (x+50)/100;   // tenth of seconds
	if(x <= 100) return (x%10 == 0);
	if(x <= 600) return (x%100 == 0);
	if(x <= 6000) return (x%600 == 0);
	return (x%3000 == 0);
}

#define JAWS_ARGS \
  { "beepOffBoard", ArgInt, (LPVOID) beeps, TRUE, (ArgIniType) 1 },\
  { "beepEmpty", ArgInt, (LPVOID) (beeps+1), TRUE, (ArgIniType) 0 },\
  { "beepWhite", ArgInt, (LPVOID) (beeps+2), TRUE, (ArgIniType) 0 },\
  { "beepBlack", ArgInt, (LPVOID) (beeps+3), TRUE, (ArgIniType) 0 },\
  { "beepHoldings", ArgInt, (LPVOID) (beeps+4), TRUE, (ArgIniType) 0 },\

#define JAWS_ALT_INTERCEPT \
	    if(suppressOneKey) {\
		suppressOneKey = 0;\
		if(GetKeyState(VK_MENU) < 0 && GetKeyState(VK_CONTROL) < 0) break;\
	    }\
	    if ((char)wParam == 022 && gameMode == EditPosition) { /* <Ctl R>. Pop up piece menu */\
		POINT pt; int x, y;\
		SquareToPos(fromY, fromX, &x, &y);\
		dropX = fromX; dropY = fromY;\
		pt.x = x; pt.y = y;\
        	if(gameInfo.variant != VariantShogi)\
		    MenuPopup(hwnd, pt, LoadMenu(hInst, "PieceMenu"), -1);\
	        else\
		    MenuPopup(hwnd, pt, LoadMenu(hInst, "ShogiPieceMenu"), -1);\
		break;\
	    }\

#define JAWS_REPLAY \
    case '\020': /* ctrl P */\
      { char buf[MSG_SIZ];\
	if(GetWindowText(hwnd, buf, MSG_SIZ-1))\
		SayString(buf, TRUE);\
      }\
      return 0;\

#define JAWS_KBDOWN_NAVIGATION \
\
\
		if(GetKeyState(VK_MENU) < 0 && GetKeyState(VK_CONTROL) < 0) {\
		    /* Control + Alt + letter used for speaking piece positions */\
		    static int lastTime; static char lastChar;\
		    int mine = 0, time = GetTickCount(); char c;\
\
		    if((char)wParam == lastChar && time-lastTime < 250) mine = 1;\
		    lastChar = wParam; lastTime = time;\
		    c = wParam;\
\
		    if(gameMode == IcsPlayingWhite || gameMode == MachinePlaysBlack) mine = !mine;\
\
		    if(ToLower(c) == 'x') {\
			SayPieces(mine ? WhitePlay : BlackPlay);\
			suppressOneKey = 1;\
			break;\
		    } else\
		    if(CharToPiece(c) != EmptySquare) {\
			SayPieces(CharToPiece(mine ? ToUpper(c) : ToLower(c)));\
			suppressOneKey = 1;\
			break;\
		    }\
		}\
\
		switch (wParam) {\
		case VK_LEFT:\
		case VK_RIGHT:\
		case VK_UP:\
		case VK_DOWN:\
			KeyboardEvent(hwnd, message, wParam, lParam);\
			break;\
		case VK_SPACE:\
                        shiftKey = GetKeyState(VK_SHIFT) < 0;\
			KeyboardMove(hwnd, message, wParam, lParam);\
			break;\
		}\

#define JAWS_KBUP_NAVIGATION \
		switch (wParam) {\
		case VK_SPACE:\
			KeyboardMove(hwnd, message, wParam, lParam);\
			break;\
		}\

#define JAWS_MENU_ITEMS \
		case IDM_PossibleAttackMove:  /*What can I possible attack from here */\
			PossibleAttackMove();\
			break;\
\
		case IDM_PossibleAttacked:  /*what can possible attack this square*/\
			PossibleAttacked();\
			break;\
\
		case IDM_ReadRow:   /* Read the current row of pieces */\
			ReadRow();\
			break;\
\
		case IDM_ReadColumn:   /* Read the current column of pieces */\
			ReadColumn();\
			break;\
\
		case IDM_SayCurrentPos: /* Say current position including color */\
			SayCurrentPos();\
			break;\
\
		case IDM_SayAllBoard:  /* Say the whole board from bottom to top */\
			SayAllBoard();\
			break;\
\
		case IDM_SayMachineMove:  /* Say the last move made */\
			timeflag = 1;\
			SayMachineMove(1);\
			//if(commentDialog && commentList[currentMove]) SetFocus(commentDialog);
			break;\
\
		case IDM_SayUpperDiagnols:  /* Says the diagnol positions above you */\
			SayUpperDiagnols();\
			break;\
\
		case IDM_SayLowerDiagnols:  /* Say the diagnol positions below you */\
			SayLowerDiagnols();\
			break;\
\
		case IDM_SayBlackPieces: /*Say the opponents pieces */\
			SayBlackPieces();\
			break;\
\
		case IDM_SayWhitePieces: /*Say the opponents pieces */\
			SayWhitePieces();\
			break;\
\
		case IDM_SayClockTime:  /*Say the clock time */\
			SayClockTime();\
			break;\
\
		case IDM_SayWhosTurn:   /* Say whos turn it its */\
			SayWhosTurn();\
			break;\
\
		case IDM_SayKnightMoves:  /* Say Knights (L-shaped) move */\
			SayKnightMoves();\
			break;\
\
		case OPT_PonderNextMove:  /* Toggle option setting */\
			Toggle(&appData.ponderNextMove, "ponder");\
			break;\
\
		case OPT_AnimateMoving:  /* Toggle option setting */\
			Toggle(&appData.animate, "animate moving");\
			break;\
\
		case OPT_AutoFlag:  /* Toggle option setting */\
			Toggle(&appData.autoCallFlag, "auto flag");\
			break;\
\
		case OPT_AlwaysQueen:  /* Toggle option setting */\
			Toggle(&appData.alwaysPromoteToQueen, "always promote to queen");\
			break;\
\
		case OPT_TestLegality:  /* Toggle option setting */\
			Toggle(&appData.testLegality, "legality testing");\
			break;\
\
		case OPT_HideThinkFromHuman:  /* Toggle option setting */\
			Toggle(&appData.hideThinkingFromHuman, "hide thinking");\
			ShowThinkingEvent();\
			break;\
\
		case OPT_SaveExtPGN:  /* Toggle option setting */\
			Toggle(&appData.saveExtendedInfoInPGN, "extended P G N info");\
			break;\
\
		case OPT_ExtraInfoInMoveHistory:  /* Toggle option setting */\
			Toggle(&appData.showEvalInMoveHistory, "extra info in move histoty");\
			break;\
\


#define JAWS_ACCEL \
	!(!frozen && TranslateAccelerator(hwndMain, hAccelJAWS, &msg)) &&

#define JAWS_INIT if (!InitJAWS()) return (FALSE);

#define JAWS_DELETE(X)

#define JAWS_SILENCE if(suppressClocks) return;

#define JAWS_COPYRIGHT \
	SetDlgItemText(hDlg, OPT_MESS, "Auditory/Keyboard Enhancements  By:  Ed Rodriguez (sort of)");

#define SAY(S) SayString((S), TRUE)

#define SAYMACHINEMOVE() SayMachineMove(0)

// After inclusion of this file somewhere early in winboard.c, the remaining part of the patch
// is scattered over winboard.c for actually calling the routines.
