#pragma once
#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <windows.h>
//#include <conio.h>

#define CTC_BLACK 0
#define CTC_BLUE 1
#define CTC_GREEN 2
#define CTC_AQUAMAINE 3
#define CTC_RED 4
#define CTC_PURPLE 5
#define CTC_YELLOW 6
#define CTC_WHITE 7
#define CTC_GREY 8
#define CTC_LIGHTBLACK 8
#define CTC_LIGHTBLUE 9
#define CTC_LIGHTGREEN 10
#define CTC_LIGHTAQUAMAINE 11
#define CTC_LIGHTRED 12
#define CTC_LIGHTPURPLE 13
#define CTC_LIGHTYELLOW 14
#define CTC_LIGHTWHITE 15
/*  颜色说明：
 * 0 = 黑色       8 = 灰色
 * 1 = 蓝色       9 = 淡蓝色
 * 2 = 绿色       10 = 淡绿色
 * 3 = 蓝绿       11 = 淡蓝绿
 * 4 = 红色       12 = 淡红色
 * 5 = 紫色       13 = 淡紫色
 * 6 = 黄色       14 = 淡黄色
 * 7 = 白色       15 = 亮白色
 */

#define KEYDOWN_UP 72
#define KEYDOWN_DOWN 80
#define KEYDOWN_LEFT 75
#define KEYDOWN_RIGHT 77
#define KEYDOWN_RETURN 13 // '\r'
#define KEYDOWN_DELETE 83 // 224 + 83
#define KEYDOWN_BACKSPACE 8
#define KEYDOWN_ESCAPE 27
#define KEYDOWN_ENTER KEYDOWN_RETURN


 /* Declaration */
void ConsoleClear();
void clprintf(int dColor, bool bBold, const char* szFormat, ...);
void cllog(const char* szFormat, ...);
void clwarning(const char* szFormat, ...);
void clerror(const char* szFormat, ...);
void SetConsoleTextColor(int dColor, bool bBold);
//void SetConsoleBkgndColor(int dColor);
//void SetConsoleColor(int colorText, int colorBkgnd);
//void SetConsoleColorReverse();
//void SetConsoleColorNormal();
void ConsolePause();
//void SetConsoleCursorPos(short  x, short y);
//void GetConsoleCursorPos(COORD* pCursor);
//void HideConsoleCursor(bool bflag);
//void GetConsoleSize(COORD* size);
//HANDLE GetConsoleHandle();

void SetConsoleAttr(
	const char* szTitle = NULL,
	int idIcon = -1,
	const SIZE* sizConsole = NULL,
	const SIZE* sizFont = NULL,
	const wchar_t* szFontName = NULL
);

#endif