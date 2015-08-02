/*
 * accessibility.c -- Common accessibility functions for X and Windows NT
 *
 * Copyright 1991 by Digital Equipment Corporation, Maynard,
 * Massachusetts.
 *
 * Enhancements Copyright 1992-2001, 2002, 2003, 2004, 2005, 2006,
 * 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Free Software Foundation, Inc.
 *
 * Enhancements Copyright 2005 Alessandro Scotti
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

#ifdef WIN32
#include <windows.h>

    int flock(int f, int code);
#   define LOCK_EX 2
#   define SLASH '\\'

#   ifdef ARC_64BIT
#       define EGBB_NAME "egbbdll64.dll"
#   else
#       define EGBB_NAME "egbbdll.dll"
#   endif

#else

#   include <sys/file.h>
#   define SLASH '/'

#   include <dlfcn.h>
#   ifdef ARC_64BIT
#       define EGBB_NAME "egbbso64.so"
#   else
#       define EGBB_NAME "egbbso.so"
#   endif
    // kludge to allow Windows code in back-end by converting it to corresponding Linux code 
#   define CDECL
#   define HMODULE void *
#   define LoadLibrary(x) dlopen(x, RTLD_LAZY)
#   define GetProcAddress dlsym

#endif

#include <stdio.h>
#include <ctype.h>

#include "config.h"

#include "common.h"
#include "frontend.h"
#include "backend.h"
#include "moves.h"

#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
# include <stdarg.h>
#else /* not STDC_HEADERS */
# if HAVE_STRING_H
#  include <string.h>
# else /* not HAVE_STRING_H */
#  include <strings.h>
# endif /* not HAVE_STRING_H */
#endif /* not STDC_HEADERS */

#if HAVE_SYS_FCNTL_H
# include <sys/fcntl.h>
#else /* not HAVE_SYS_FCNTL_H */
# if HAVE_FCNTL_H
#  include <fcntl.h>
# endif /* HAVE_FCNTL_H */
#endif /* not HAVE_SYS_FCNTL_H */

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <math.h>

extern long whiteTimeRemaining, blackTimeRemaining, timeControl, timeIncrement, lastWhite, lastBlack, activePartnerTime;
extern char *commentList[];
extern char lastMsg[MSG_SIZ];
extern char moveList[MAX_MOVES][MOVE_LEN];
extern char parseList[MAX_MOVES][MOVE_LEN*2];



char *pieceToName[] = {
    "White Pawn", "White Knight", "White Bishop", "White Rook", "White Queen",
    "White Ferz", "White Alfil", "White Angel", "White Marshall", "White Wazir", "White Man",
    "White Cannon", "White Nightrider", "White Cardinal", "White Dragon", "White Grasshopper",
    "White Silver", "White Falcon", "White Lance", "White Cobra", "White Unicorn", "White Lion",
    "White Tokin", "White Claw", "White PCardinal", "White PDragon", "White Cat",
    "White PSword", "White Monarch", "White Mother", "White Nothing", "White PRook", "White PDagger",
    "White Dolphin", "White Stag", "White Horned", "White Eagle", "White Sword",
    "White Crown", "White HCrown", "White Horse", "White Drunk", "White PBishop", "White King",
    "Black Pawn", "Black Knight", "Black Bishop", "Black Rook", "Black Queen",
    "Black Ferz", "Black Alfil", "Black Angel", "Black Marshall", "Black Wazir", "Black Man",
    "Black Cannon", "Black Nightrider", "Black Cardinal", "Black Dragon", "Black Grasshopper",
    "Black Silver", "Black Falcon", "Black Lance", "Black Cobra", "Black Unicorn", "Black Lion",
    "Black Tokin", "Black Claw", "Black PCardinal", "Black PDragon", "Black Cat",
    "Black PSword", "Black Monarch", "Black Mother", "Black Nothing", "Black PRook", "Black PDagger",
    "Black Dolphin", "Black Stag", "Black Horned", "Black Eagle", "Black Sword",
    "Black Crown", "Black HCrown", "Black Horse", "Black Drunk", "Black PBishop", "Black King",
    "EmptySquare", "DarkSquare",
    "NoRights", // [HGM] gamestate: for castling rights hidden in board[CASTLING]
    "ClearBoard", "WhitePlay", "BlackPlay", "PromotePiece", "DemotePiece" /*for use on EditPosition menus*/
	};
	
char *pieceTypeName[] = {
    "Pawn", "Knight", "Bishop", "Rook", "Queen",
    "Ferz", "Alfil", "Angel", "Marshall", "Wazir", "Man",
    "Cannon", "Nightrider", "Cardinal", "Dragon", "Grasshopper",
    "Silver", "Falcon", "Lance", "Cobra", "Unicorn", "Lion",
    "Tokin", "Claw", "PCardinal", "PDragon", "Cat",
    "PSword", "Monarch", "Mother", "Nothing", "PRook", "PDagger",
    "Dolphin", "Stag", "Horned", "Eagle", "Sword",
    "Crown", "HCrown", "Horse", "Drunk", "PBishop", "King",
    "Pawn", "Knight", "Bishop", "Rook", "Queen",
    "Ferz", "Alfil", "Angel", "Marshall", "Wazir", "Man",
    "Cannon", "Nightrider", "Cardinal", "Dragon", "Grasshopper",
    "Silver", "Falcon", "Lance", "Cobra", "Unicorn", "Lion",
    "Tokin", "Claw", "PCardinal", "PDragon", "Cat",
    "PSword", "Monarch", "Mother", "Nothing", "PRook", "PDagger",
    "Dolphin", "Stag", "Horned", "Eagle", "Sword",
    "Crown", "HCrown", "Horse", "Drunk", "PBishop", "King",
    "EmptySquare", "DarkSquare",
    "NoRights", // [HGM] gamestate: for castling rights hidden in board[CASTLING]
    "ClearBoard", "WhitePlay", "BlackPlay", "PromotePiece", "DemotePiece" /*for use on EditPosition menus*/
	};	
	
char *squareToChar[] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l" };

char *squareToNum[] = {"naught", "1", "2", "3", "4", "5", "6", "7", "8", "9" };	

char *ordinals[] = {"zeroth", "first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eighth", "nineth"};

typedef struct {
    int rf, ff, rt, ft;
    int onlyCaptures;
    int count;
} ReadClosure;

extern int 	oldFromX, oldFromY;

char* PieceToName(ChessSquare p, int i){
	if(i) return pieceToName[(int) p];
		return pieceTypeName[(int) p];
}

char* SquareToChar(x)
			int x;
{
		return squareToChar[x - BOARD_LEFT];
}

char* SquareToNum(y)
			int y;
{
		return squareToNum[y + (gameInfo.boardHeight < 10)];
}


void 
SayWhosTurn()
{
	if(gameMode == MachinePlaysBlack || gameMode == IcsPlayingWhite) {
		if(WhiteOnMove(currentMove))
			set_accessible_description("It is your turn", FALSE);
		else	set_accessible_description("It is your opponents turn", FALSE);
	} else if(gameMode == MachinePlaysWhite || gameMode == IcsPlayingBlack) {
		if(WhiteOnMove(currentMove))
			set_accessible_description("It is your opponents turn", FALSE);
		else	set_accessible_description("It is your turn", FALSE);
	} else {
		if(WhiteOnMove(currentMove))
			set_accessible_description("White is on move here", FALSE);
		else	set_accessible_description("Black is on move here", FALSE);
	}
	set_accessible_description("", TRUE); // flush
}


void
ReadRow()
{
	ChessSquare currentpiece;
	char *piece, *xchar, *ynum ;
	int xPos, count=0;
	ynum = SquareToNum(fromY);

	if(fromY < 0) return;

	for (xPos=BOARD_LEFT; xPos<BOARD_RGHT; xPos++) {
		currentpiece = boards[currentMove][fromY][xPos];
		if(currentpiece != EmptySquare) {
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(xPos);
			set_accessible_description(xchar , FALSE);
			set_accessible_description(ynum, FALSE);
			set_accessible_description(piece, FALSE);
			count++;
		}
	}
	if(count == 0) {
		set_accessible_description("rank", FALSE);
		set_accessible_description(ynum, FALSE);
		set_accessible_description("empty", FALSE);
	}
	set_accessible_description("", TRUE); // flush
}

void 
ReadColumn()
{
	ChessSquare currentpiece;
	char *piece, *xchar, *ynum ;
	int yPos, count=0;
	xchar = SquareToChar(fromX);

	if(fromX < 0) return;

	for (yPos=0; yPos<BOARD_HEIGHT; yPos++) {
		currentpiece = boards[currentMove][yPos][fromX];
		if(currentpiece != EmptySquare) {
			piece = PieceToName(currentpiece,1);
			ynum = SquareToNum(yPos);
			set_accessible_description(xchar , FALSE);
			set_accessible_description(ynum, FALSE);
			set_accessible_description(piece, FALSE);
			count++;
		}
	}
	if(count == 0) {
		set_accessible_description(xchar, FALSE);
		set_accessible_description("file empty", FALSE);
	}
	set_accessible_description("", TRUE); // flush
}

void 
SayUpperDiagnols()
{
	ChessSquare currentpiece;
	char *piece, *xchar, *ynum ;
	int yPos, xPos;

	if(fromX < 0 || fromY < 0) return;

	if(fromX < BOARD_RGHT-1 && fromY < BOARD_HEIGHT-1) {
		set_accessible_description("The diagnol squares to your upper right contain", FALSE);
		yPos = fromY+1;
		xPos = fromX+1;
		while(yPos<BOARD_HEIGHT && xPos<BOARD_RGHT) {
			currentpiece = boards[currentMove][yPos][xPos];
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(xPos);
			ynum = SquareToNum(yPos);
			set_accessible_description(xchar , FALSE);
			set_accessible_description(ynum, FALSE);
			set_accessible_description(piece, FALSE);
			yPos++;
			xPos++;
		}
	}
	else set_accessible_description("There is no squares to your upper right", FALSE);

	if(fromX > BOARD_LEFT && fromY < BOARD_HEIGHT-1) {
		set_accessible_description("The diagnol squares to your upper left contain", FALSE);
		yPos = fromY+1;
		xPos = fromX-1;
		while(yPos<BOARD_HEIGHT && xPos>=BOARD_LEFT) {
			currentpiece = boards[currentMove][yPos][xPos];
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(xPos);
			ynum = SquareToNum(yPos);
			set_accessible_description(xchar , FALSE);
			set_accessible_description(ynum, FALSE);
			set_accessible_description(piece, FALSE);
			yPos++;
			xPos--;
		}
	}
	else set_accessible_description("There is no squares to your upper left", FALSE);
	set_accessible_description("", TRUE); // flush
}

void
SayLowerDiagnols()
{
	ChessSquare currentpiece;
	char *piece, *xchar, *ynum ;
	int yPos, xPos;

	if(fromX < 0 || fromY < 0) return;

	if(fromX < BOARD_RGHT-1 && fromY > 0) {
		set_accessible_description("The diagnol squares to your lower right contain", FALSE);
		yPos = fromY-1;
		xPos = fromX+1;
		while(yPos>=0 && xPos<BOARD_RGHT) {
			currentpiece = boards[currentMove][yPos][xPos];
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(xPos);
			ynum = SquareToNum(yPos);
			set_accessible_description(xchar , FALSE);
			set_accessible_description(ynum, FALSE);
			set_accessible_description(piece, FALSE);
			yPos--;
			xPos++;
		}
	}
	else set_accessible_description("There is no squares to your lower right", FALSE);

	if(fromX > BOARD_LEFT && fromY > 0) {
		set_accessible_description("The diagnol squares to your lower left contain", FALSE);
		yPos = fromY-1;
		xPos = fromX-1;
		while(yPos>=0 && xPos>=BOARD_LEFT) {
			currentpiece = boards[currentMove][yPos][xPos];
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(xPos);
			ynum = SquareToNum(yPos);
			set_accessible_description(xchar , FALSE);
			set_accessible_description(ynum, FALSE);
			set_accessible_description(piece, FALSE);
			yPos--;
			xPos--;
		}
	}
	else set_accessible_description("There is no squares to your lower left", FALSE);
	set_accessible_description("", TRUE); // flush
}


void 
SayClockTime()
{
	char buf1[50], buf2[50];
	char *str1, *str2;
	//static long int lastWhiteTime, lastBlackTime;

	//suppressClocks = 1; // if user is using alt+T command, no reason to display them
	//if(abs(lastWhiteTime - whiteTimeRemaining) < 1000 && abs(lastBlackTime - blackTimeRemaining) < 1000)
	//	suppressClocks = 0; // back on after two requests in rapid succession
	snprintf(buf1, sizeof(buf1)/sizeof(buf1[0]),"%s", TimeString(whiteTimeRemaining));
	str1 = buf1;
	set_accessible_description("White clock", FALSE);
	set_accessible_description(str1, FALSE);
	snprintf(buf2, sizeof(buf2)/sizeof(buf2[0]), "%s", TimeString(blackTimeRemaining));
	str2 = buf2;
	set_accessible_description("Black clock", FALSE);
	set_accessible_description(str2, FALSE);
	//lastWhiteTime = whiteTimeRemaining;
	//lastBlackTime = blackTimeRemaining;
	set_accessible_description("", TRUE); // flush
}

int CoordToNum(c)
		char c;
{
	if(isdigit(c)) return c - ONE;
	if(c >= 'a') return c - AAA;
	return 0;
}

void 
SayAllBoard()
{
	int Xpos, Ypos;
	ChessSquare currentpiece;
	char *piece, *ynum ;

	if(gameInfo.holdingsWidth) {
		int first = 0;
		for(Ypos=0; Ypos<gameInfo.holdingsSize; Ypos++) {
			int n = boards[currentMove][Ypos][BOARD_WIDTH-2];
			if(n) {
			  char buf[MSG_SIZ];
			  if(!first++)
			    set_accessible_description("white holds", FALSE);
			  currentpiece = boards[currentMove][Ypos][BOARD_WIDTH-1];
			  piece = PieceToName(currentpiece,0);
			  snprintf(buf, MSG_SIZ,"%d %s%s", n, piece, (n==1 ? "" : "s") );
			  set_accessible_description(buf, FALSE);
			}
		}
		first = 0;
		for(Ypos=BOARD_HEIGHT-1; Ypos>=BOARD_HEIGHT - gameInfo.holdingsSize; Ypos--) {
			int n = boards[currentMove][Ypos][1];
			if(n) {
			  char buf[MSG_SIZ];
			  if(!first++)
			    set_accessible_description("black holds", FALSE);
			  currentpiece = boards[currentMove][Ypos][0];
			  piece = PieceToName(currentpiece,0);
			  snprintf(buf, MSG_SIZ, "%d %s%s", n, piece, (n==1 ? "" : "s") );
			  set_accessible_description(buf, FALSE);
			}
		}
	}

	for(Ypos=BOARD_HEIGHT-1; Ypos>=0; Ypos--) {
		ynum = ordinals[Ypos + (gameInfo.boardHeight < 10)];
		set_accessible_description(ynum, FALSE);
		set_accessible_description("rank", FALSE);
		for(Xpos=BOARD_LEFT; Xpos<BOARD_RGHT; Xpos++) {
			currentpiece = boards[currentMove][Ypos][Xpos];
			if(currentpiece != EmptySquare) {
				int count = 0;
				char buf[50];
				piece = PieceToName(currentpiece,1);
				while(Xpos < BOARD_RGHT && boards[currentMove][Ypos][Xpos] == currentpiece)
					Xpos++, count++;
				if(count > 1)
				  snprintf(buf, sizeof(buf)/sizeof(buf[0]), "%d %ss", count, piece);
				else
				  snprintf(buf, sizeof(buf)/sizeof(buf[0]), "%s", piece);
				Xpos--;
				set_accessible_description(buf, FALSE);
			} else {
				int count = 0, oldX = Xpos;
				while(Xpos < BOARD_RGHT && boards[currentMove][Ypos][Xpos] == EmptySquare)
					Xpos++, count++;
				if(Xpos == BOARD_RGHT && oldX == BOARD_LEFT)
					set_accessible_description("all", FALSE);
				else{
				    if(count > 1) {
					char buf[10];
					snprintf(buf, sizeof(buf)/sizeof(buf[0]),"%d", count);
					set_accessible_description(buf, FALSE);
				    }
				    Xpos--;
				}
				set_accessible_description("empty", FALSE);
			}
		}
	}
	set_accessible_description("", TRUE); // flush
}



void 
SayPieces(ChessSquare p)
{
	ChessSquare currentpiece;
	char *piece, *xchar, *ynum ;
	int yPos, xPos, count = 0;
	char buf[50];

	if(p == WhitePlay)   set_accessible_description("White pieces", FALSE); else
	if(p == BlackPlay)   set_accessible_description("Black pieces", FALSE); else
	if(p == EmptySquare) set_accessible_description("Pieces", FALSE); else {
	  snprintf(buf, sizeof(buf)/sizeof(buf[0]),"%ss", PieceToName(p,1));
		set_accessible_description(buf, FALSE);
	}
	set_accessible_description("are located", FALSE);
	for(yPos=0; yPos<BOARD_HEIGHT; yPos++) {
		for(xPos=BOARD_LEFT; xPos<BOARD_RGHT; xPos++) {
			currentpiece = boards[currentMove][yPos][xPos];
			if(p == BlackPlay && currentpiece >= BlackPawn && currentpiece <= BlackKing ||
			   p == WhitePlay && currentpiece >= WhitePawn && currentpiece <= WhiteKing   )
				piece = PieceToName(currentpiece,0);
			else if(p == EmptySquare && currentpiece != EmptySquare)
				piece = PieceToName(currentpiece,1);
			else if(p == currentpiece)
				piece = NULL;
			else continue;

				if(count == 0) set_accessible_description("at", FALSE);
				xchar = SquareToChar(xPos);
				ynum = SquareToNum(yPos);
				set_accessible_description(xchar , FALSE);
				set_accessible_description(ynum, FALSE);
				if(piece) set_accessible_description(piece, FALSE);
				count++;
		}
	}
	if(count == 0) set_accessible_description("nowhere", FALSE);
	set_accessible_description("", TRUE); // flush
}

void SayWhitePieces()
{
	SayPieces(WhitePlay);
}

void SayBlackPieces()
{
	SayPieces(BlackPlay);
}

void 
SayKnightMoves()
{
	ChessSquare currentpiece, oldpiece;
	char *piece, *xchar, *ynum ;

	oldpiece = boards[currentMove][fromY][fromX];
	if(oldpiece == WhiteKnight || oldpiece == BlackKnight)
		set_accessible_description("The possible squares a Knight could move to are", FALSE);
	else
		set_accessible_description("The squares a Knight could possibly attack from are", FALSE);

	if (fromY+2 < BOARD_HEIGHT && fromX-1 >= BOARD_LEFT) {
		currentpiece = boards[currentMove][fromY+2][fromX-1];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX-1);
			ynum = SquareToNum(fromY+2);
			set_accessible_description(xchar , FALSE);
			set_accessible_description(ynum, FALSE);
			set_accessible_description(piece, FALSE);
		}
	}

	if (fromY+2 < BOARD_HEIGHT && fromX+1 < BOARD_RGHT) {
		currentpiece = boards[currentMove][fromY+2][fromX+1];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX+1);
			ynum = SquareToNum(fromY+2);
			set_accessible_description(xchar , FALSE);
			set_accessible_description(ynum, FALSE);
			set_accessible_description(piece, FALSE);
		}
	}

	if (fromY+1 < BOARD_HEIGHT && fromX+2 < BOARD_RGHT) {
		currentpiece = boards[currentMove][fromY+1][fromX+2];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX+2);
			ynum = SquareToNum(fromY+1);
			set_accessible_description(xchar , FALSE);
			set_accessible_description(ynum, FALSE);
			set_accessible_description(piece, FALSE);
		}
	}

	if (fromY-1 >= 0 && fromX+2 < BOARD_RGHT) {
		currentpiece = boards[currentMove][fromY-1][fromX+2];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX+2);
			ynum = SquareToNum(fromY-1);
			set_accessible_description(xchar , FALSE);
			set_accessible_description(ynum, FALSE);
			set_accessible_description(piece, FALSE);
		}
	}

	if (fromY-2 >= 0 && fromX+1 < BOARD_RGHT) {
		currentpiece = boards[currentMove][fromY-2][fromX+1];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX+1);
			ynum = SquareToNum(fromY-2);
			set_accessible_description(xchar , FALSE);
			set_accessible_description(ynum, FALSE);
			set_accessible_description(piece, FALSE);
		}
	}

	if (fromY-2 >= 0 && fromX-1 >= BOARD_LEFT) {
		currentpiece = boards[currentMove][fromY-2][fromX-1];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX-1);
			ynum = SquareToNum(fromY-2);
			set_accessible_description(xchar , FALSE);
			set_accessible_description(ynum, FALSE);
			set_accessible_description(piece, FALSE);
		}
	}

	if (fromY-1 >= 0 && fromX-2 >= BOARD_LEFT) {
		currentpiece = boards[currentMove][fromY-1][fromX-2];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX-2);
			ynum = SquareToNum(fromY-1);
			set_accessible_description(xchar , FALSE);
			set_accessible_description(ynum, FALSE);
			set_accessible_description(piece, FALSE);
		}
	}

	if (fromY+1 < BOARD_HEIGHT && fromX-2 >= BOARD_LEFT) {
		currentpiece = boards[currentMove][fromY+1][fromX-2];
		if(((oldpiece == WhiteKnight) && (currentpiece > WhiteKing))
			|| ((oldpiece == BlackKnight) && (currentpiece < BlackPawn || currentpiece == EmptySquare))
			|| (oldpiece == EmptySquare) && (currentpiece == WhiteKnight || currentpiece == BlackKnight))
		{
			piece = PieceToName(currentpiece,1);
			xchar = SquareToChar(fromX-2);
			ynum = SquareToNum(fromY+1);
			set_accessible_description(xchar , FALSE);
			set_accessible_description(ynum, FALSE);
			set_accessible_description(piece, FALSE);
		}
	}
	set_accessible_description("", TRUE); // flush
}

void 
SayCurrentPos()
{
	ChessSquare currentpiece;
	char *piece, *xchar, *ynum ;
	if(fromX <  BOARD_LEFT) { set_accessible_description("You strayed into the white holdings", FALSE); return; }
	if(fromX >= BOARD_RGHT) { set_accessible_description("You strayed into the black holdings", FALSE); return; }
	currentpiece = boards[currentMove][fromY][fromX];
	piece = PieceToName(currentpiece,1);
	ynum = SquareToNum(fromY);
	xchar = SquareToChar(fromX);
	set_accessible_description("Your current position is", FALSE);
	set_accessible_description(xchar, FALSE);
	set_accessible_description(ynum, FALSE);
	set_accessible_description(piece, FALSE);
	if(((fromX-BOARD_LEFT) ^ fromY)&1)
		set_accessible_description("on a light square",FALSE);
	else
		set_accessible_description("on a dark square",FALSE);

	//PossibleAttacked();
	set_accessible_description("", TRUE); // flush
}

extern void ReadCallback P((Board board, int flags, ChessMove kind,
				int rf, int ff, int rt, int ft,
				VOIDSTAR closure));


void ReadCallback(board, flags, kind, rf, ff, rt, ft, closure)
     Board board;
     int flags;
     ChessMove kind;
     int rf, ff, rt, ft;
     VOIDSTAR closure;
{
    register ReadClosure *cl = (ReadClosure *) closure;
    ChessSquare possiblepiece;
    char *piece, *xchar, *ynum ;

//if(appData.debugMode) fprintf(debugFP, "%c%c%c%c\n", ff+AAA, rf+ONE, ft+AAA, rt+ONE);
    if(cl->ff == ff && cl->rf == rf) {
	possiblepiece = board[rt][ft];
	if(possiblepiece != EmptySquare) {
		piece = PieceToName(possiblepiece,1);
		xchar = SquareToChar(ft);
		ynum  = SquareToNum(rt);
		set_accessible_description(xchar , FALSE);
		set_accessible_description(ynum, FALSE);
		set_accessible_description(piece, FALSE);
		cl->count++;
	}
    }
    if(cl->ft == ft && cl->rt == rt) {
	possiblepiece = board[rf][ff];
		piece = PieceToName(possiblepiece,1);
		xchar = SquareToChar(ff);
		ynum  = SquareToNum(rf);
		set_accessible_description(xchar , FALSE);
		set_accessible_description(ynum, FALSE);
		set_accessible_description(piece, FALSE);
		cl->count++;
    }
}


int PosFlags(int nr);

void 
PossibleAttacked()
{
	ReadClosure cl;
	ChessSquare piece = EmptySquare, victim;

	if(fromY < 0 || fromY >= BOARD_HEIGHT) return;
	if(fromX < BOARD_LEFT || fromX >= BOARD_RGHT) { set_accessible_description("holdings",TRUE); return; }

	if(oldFromX >= 0 && oldFromY >= 0) { // if piece is selected, remove it
		piece = boards[currentMove][oldFromY][oldFromX];
		boards[currentMove][oldFromY][oldFromX] = EmptySquare;
	}

	set_accessible_description("Pieces that can capture you are", FALSE);

	victim = boards[currentMove][fromY][fromX]; // put dummy piece on target square, to activate Pawn captures
	boards[currentMove][fromY][fromX] = WhiteOnMove(currentMove) ? WhiteQueen : BlackQueen;
	cl.count = 0; cl.rt = fromY; cl.ft = fromX; cl.rf = cl.ff = -1;
	
	//GenLegal(boards[currentMove], PosFlags(currentMove), Mark, (void*) marker, EmptySquare);
	GenLegal(boards[currentMove], PosFlags(currentMove+1), ReadCallback, (VOIDSTAR) &cl,EmptySquare);
	if(cl.count == 0) set_accessible_description("None", FALSE);

	set_accessible_description("You are defended by", FALSE);

	boards[currentMove][fromY][fromX] = WhiteOnMove(currentMove) ? BlackQueen : WhiteQueen;
	cl.count = 0; cl.rt = fromY; cl.ft = fromX; cl.rf = cl.ff = -1;
	GenLegal(boards[currentMove], PosFlags(currentMove), ReadCallback, (VOIDSTAR) &cl,EmptySquare);
	if(cl.count == 0) set_accessible_description("None", FALSE);
	set_accessible_description("", TRUE); // flush
	boards[currentMove][fromY][fromX] = victim; // put back original occupant

	if(oldFromX >= 0 && oldFromY >= 0) { // put back possibl selected piece
		boards[currentMove][oldFromY][oldFromX] = piece;
	}
}

void 
PossibleAttackMove()
{
	ReadClosure cl;
	ChessSquare piece, victim;
	int removedSelectedPiece = 0, swapColor;

//if(appData.debugMode) fprintf(debugFP, "PossibleAttackMove %d %d %d %d\n", fromX, fromY, oldFromX, oldFromY);
	if(fromY < 0 || fromY >= BOARD_HEIGHT) return;
	if(fromX < BOARD_LEFT || fromX >= BOARD_RGHT) { set_accessible_description("holdings",TRUE); return; }

	piece = boards[currentMove][fromY][fromX];
	if(piece == EmptySquare) { // if square is empty, try to substitute selected piece
	    if(oldFromX >= 0 && oldFromY >= 0) {
		piece = boards[currentMove][oldFromY][oldFromX];
		boards[currentMove][oldFromY][oldFromX] = EmptySquare;
		removedSelectedPiece = 1;
		set_accessible_description("Your", FALSE);
		set_accessible_description(PieceToName(piece, 0), FALSE);
		set_accessible_description("would have", FALSE);
	    } else { set_accessible_description("You must select a piece first", TRUE); return; }
	}

	victim = boards[currentMove][fromY][fromX];
	boards[currentMove][fromY][fromX] = piece; // make sure piece is actally there
	set_accessible_description("possible captures from here are", FALSE);

	swapColor = piece <  (int)BlackPawn && !WhiteOnMove(currentMove) ||
		    piece >= (int)BlackPawn &&  WhiteOnMove(currentMove);
	cl.count = 0; cl.rf = fromY; cl.ff = fromX; cl.rt = cl.ft = -1;
	GenLegal(boards[currentMove], PosFlags(currentMove + swapColor), ReadCallback, (VOIDSTAR) &cl,EmptySquare);
	if(cl.count == 0) set_accessible_description("None", FALSE);
	set_accessible_description("", TRUE); // flush
	boards[currentMove][fromY][fromX] = victim; // repair

	if( removedSelectedPiece ) boards[currentMove][oldFromY][oldFromX] = piece;
}


void SayMoveDetailed(int move_number);

void 
SayMachineMove(int evenIfDuplicate)
{
	int len, moveNr, secondSpace = 0, n;
	char *p;
	char buf[MSG_SIZ], comment[MSG_SIZ];
	static int previousMove = 0;

	if(appData.debugMode) fprintf(debugFP, "Message = '%s'\n", lastMsg);
	if(gameMode == BeginningOfGame) return;
	if(lastMsg[0] == '[') return;
	comment[0]= 0;
	    if(isdigit(lastMsg[0])) { // message is move, possibly with thinking output
		int dotCount = 0, spaceCount = 0;
		sscanf(lastMsg, "%d", &moveNr);
		len = 0;
		// [HGM] show: better extraction of move
		while (lastMsg[len] != NULLCHAR) {
		    if(lastMsg[len] == '.' && spaceCount == 0) dotCount++;
		    if(lastMsg[len] == ' ') { if(++spaceCount == 2) secondSpace = len; }
		    if(lastMsg[len] == '{') { // we detected a comment
			if(isalpha(lastMsg[len+1]) ) sscanf(lastMsg+len, "{%[^}]}", comment);
			break;
		    }
		    if(lastMsg[len] == '[') { // we detected thinking output
			int depth; float score=0; char c, lastMover = (dotCount == 3 ? 'B' : 'W');
			if(sscanf(lastMsg+len+1, "%d]%c%f", &depth, &c, &score) > 1) {
			    if(c == ' ') { // if not explicitly specified, figure out source of thinking output
				switch(gameMode) {
				  case MachinePlaysWhite:
				  case IcsPlayingWhite:
				    c = 'W'; break;
				  case IcsPlayingBlack:
				  case MachinePlaysBlack:
				    c = 'B';
				  default:
				    break;
				}
			    }
			    if(c != lastMover && !evenIfDuplicate) return; // line is thinking output of future move, ignore.
			    if(2*moveNr - (dotCount < 2) == previousMove)
				return; // do not repeat same move; likely ponder output
			    snprintf(buf, MSG_SIZ, "score %s %d at %d ply",
					score > 0 ? "plus" : score < 0 ? "minus" : "",
					(int) (fabs(score)*100+0.5),
					depth );
			    set_accessible_description(buf, FALSE); // move + thinking output describing it; say it.
			}
			while(lastMsg[len-1] == ' ') len--; // position just behind move;
			break;
		    }
		    if(lastMsg[len] == '(') { // ICS time printed behind move
			while(lastMsg[len+1] && lastMsg[len] != ')') len++; // skip it
		    }
		    len++;
		}
		
		p = NULL;
		if(StrStr(lastMsg, " 1-0")) p = "white wins"; else
	    if(StrStr(lastMsg, " 0-1")) p = "black wins"; else
	    if(StrStr(lastMsg, " 1/2-1/2")) p = "game ends in a draw";
		
		if(comment[0]) {
		    if(p) {
			if(!StrCaseStr(comment, "draw") &&
			   !StrCaseStr(comment, "white") &&
			   !StrCaseStr(comment, "black") ) {
				set_accessible_description(p, FALSE);
				set_accessible_description("due to", FALSE);
			}
		    }
		    set_accessible_description(comment, FALSE); // alphabetic comment (usually game end)
	    } 
		
		n = 2*moveNr - (dotCount < 2);
		SayMoveDetailed(n-1);
		
	    } else {
		/* starts not with digit */
		if(StrCaseStr(lastMsg, "illegal")) PlayIcsUnfinishedSound();
		SayMoveDetailed(currentMove-1);
		//set_accessible_description(lastMsg, TRUE);
	    }
}

void 
SayMoveDetailed(int move_number)
{
	int len, xPos, yPos, castle = 0;
	ChessSquare currentpiece;
	char *piece, *xchar, *ynum, *p, checkMark = 0;
	char c, buf[MSG_SIZ], comment[MSG_SIZ];
	len = strlen(parseList[move_number]);
	comment[0]= 0;	
	if(parseList[move_number][len-1] == '+' || parseList[move_number][len-1] == '#') {
		/* you are in checkmate */
		len--; // strip off check or mate indicator
		checkMark = parseList[move_number][len]; // make sure still seen after we stip off promo piece 
	}
	
	if(parseList[move_number][len-2] == '=') {  /* promotion */
			len-=2; // strip off promotion piece
			set_accessible_description("promotion", FALSE);	}
		
	
	yPos = CoordToNum(parseList[move_number][len-1]);  /* turn char coords to ints */
	xPos = CoordToNum(parseList[move_number][len-2]);
	if(xPos < 0 || xPos > 11) return; // prevent crashes if no coord string available to speak
	if(yPos < 0 || yPos > 9)  return;	
	currentpiece = boards[move_number+1][yPos][xPos];
	piece = PieceToName(currentpiece,0);
	ynum = SquareToNum(yPos);
	xchar = SquareToChar(xPos);

	c = parseList[move_number][len-3];
	if(c == 'x') c = parseList[move_number][len-4];
	if(!isdigit(c) && c < 'a' && c != '@') c = 0;

	set_accessible_description(WhiteOnMove(move_number+1) ? "Black" : "White", FALSE);	
	sprintf(buf,"%d.",(move_number+1)/2+((move_number+1)%2));
	set_accessible_description(buf, FALSE);
		    
	p = StrStr(parseList[move_number], "O-O-O");
	if(p && p-parseList[move_number] < len) {
		set_accessible_description("queen side castling",FALSE);
		castle = 1;
	}
	else
	{
		p = StrStr(parseList[move_number], "O-O");
		if(p && p-parseList[move_number] < len) {
			set_accessible_description("king side castling",FALSE);
			castle = 1;
		}
	}
	if(!castle) {
		set_accessible_description(piece, FALSE);
		if(c == '@') set_accessible_description("dropped on", FALSE);
		else
		{
			int ayPos = CoordToNum(moveList[move_number][1]);
			int axPos = CoordToNum(moveList[move_number][0]);
			char *aynum = SquareToNum(ayPos);
			char *axchar = SquareToChar(axPos);		
			set_accessible_description(axchar, FALSE);
			set_accessible_description(aynum, FALSE);
			set_accessible_description("-", FALSE);
		}
		
		set_accessible_description(xchar, FALSE);
		set_accessible_description(ynum, FALSE);
		if(parseList[move_number][len-3] == 'x') {
				currentpiece = boards[move_number][yPos][xPos];
				if(currentpiece != EmptySquare) {
					piece = PieceToName(currentpiece,0);
					set_accessible_description("Capturing a",FALSE);
					set_accessible_description(piece, FALSE);
				} else set_accessible_description("Capturing onn passann",FALSE);
			}
	}

	if(checkMark == '+') set_accessible_description("check", FALSE); else
	if(checkMark == '#') {
		set_accessible_description("finishing off", FALSE);
		set_accessible_description(WhiteOnMove(move_number+1) ? "White." : "Black.", FALSE);
	}
	
	/* say comment after move, possibly with result */
	if(StrStr(parseList[move_number], " 1-0")) set_accessible_description("white wins", FALSE); else
	if(StrStr(parseList[move_number], " 0-1")) set_accessible_description("black wins", FALSE); else
	if(StrStr(parseList[move_number], " 1/2-1/2")) set_accessible_description("game ends in a draw", FALSE);

	set_accessible_description("", TRUE); // flush
}

void
SayMove(int b)
{
	static int move_cursor = -1000;
	if( move_cursor == -1000)
		move_cursor = currentMove-1;

	if (move_cursor+b >= 0 && move_cursor+b < currentMove )
	{
		move_cursor += b;
		SayMoveDetailed(move_cursor);
	}
}

void
SayNextMove()
{
	SayMove(+1);
}

void
SayPreviousMove()
{
	SayMove(-1);
}

