#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <windows.h>
#include <conio.h>
#include <string>
#include <cstdarg>
#include "console.h"

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

#define CONSOLECOL 50
#define CONSOLELINE 20
#define _CONSOLECOL "50"
#define _CONSOLELINE "20"

//std::string _IntToString(int x)
//{
//	return x >= 10 ? _IntToString(x / 10) + char(x % 10 + '0') : std::string("") + char(x % 10 + '0');
//}

struct SIZE_SHORT {
	SHORT cx,
		cy;
};

void SetConsoleAttr(
	const char* szTitle, 
	int idIcon, 
	const SIZE_SHORT*sizConsole,
	const SIZE_SHORT*sizFont,
	const wchar_t* szFontName
	)
{
	HWND hwConsole = GetConsoleWindow();
	//SetWindowLong(hwConsole, GWL_STYLE, (GetWindowLong(hwConsole, GWL_STYLE) & ~WS_SIZEBOX) & ~WS_MAXIMIZEBOX);
	if (szTitle != NULL)
		SetConsoleTitle(szTitle);

	// 设置控制台图标
	if (idIcon != -1)
	{
		SendMessage(hwConsole, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(idIcon)));
		SendMessage(hwConsole, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(idIcon)));
	}

	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	if (sizConsole != NULL)
	{
		SMALL_RECT rcConsole = { 0, 0, sizConsole->cx, sizConsole->cy }; // 设置控制台大小
		SetConsoleWindowInfo(hOutput, true, &rcConsole);
	}
	CONSOLE_FONT_INFOEX ConsoleCurrentFontEx; // 设置字体
	ConsoleCurrentFontEx.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	ConsoleCurrentFontEx.nFont = 0;
	if (sizFont != NULL)
	{
		ConsoleCurrentFontEx.dwFontSize.X = sizFont->cx;
		ConsoleCurrentFontEx.dwFontSize.Y = sizFont->cy;
	}
	ConsoleCurrentFontEx.FontFamily = TMPF_VECTOR;
	ConsoleCurrentFontEx.FontWeight = 200;
	if (szFontName != NULL)
	{
		wcscpy(ConsoleCurrentFontEx.FaceName, szFontName);
	}
	SetCurrentConsoleFontEx(hOutput, FALSE, &ConsoleCurrentFontEx);
}

//void InitConsole(int cols, int lines)
//{
//	system(
//		("mode con:cols=" + _IntToString(cols) + " lines=" + _IntToString(lines)).c_str()
//	);
//}

void SetConsoleTextColor(int dColor, bool bBold)
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, (bBold ? FOREGROUND_INTENSITY : 0) | dColor);
}

void ConsoleClear()
{
	system("cls");
}

void ConsolePause()
{
	char _ = _getch();
}

void SetConsoleCursorPos(SHORT  x, SHORT y)
{
	COORD point = { x, y };
	HANDLE HOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(HOutput, point);
}

void GetConsoleCursorPos(COORD* pCursor)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hOutput, &csbi);
	*pCursor = csbi.dwCursorPosition;
}

void clprintf(int dColor, bool bBold, const char* szFormat, ...)
{
	SetConsoleTextColor(dColor, bBold);
	va_list vararglist;
	va_start(vararglist, szFormat);
	vprintf(szFormat, vararglist);
	va_end(vararglist);
	SetConsoleTextColor(CTC_WHITE, 0);
}

void clvprintf(int dColor, bool bBold, const char* szFormat, va_list _ArgList) {
	SetConsoleTextColor(dColor, bBold);
	vprintf(szFormat, _ArgList);
	SetConsoleTextColor(CTC_WHITE, 0);
}

void cllog(const char* szFormat, ...)
{
	va_list vararglist;
	va_start(vararglist, szFormat);
	clvprintf(CTC_BLUE, 1, szFormat, vararglist);
	va_end(vararglist);
}

void clerror(const char* szFormat, ...)
{
	va_list vararglist;
	va_start(vararglist, szFormat);
	clvprintf(CTC_RED, 0, szFormat, vararglist);
	va_end(vararglist);
}

void clwarning(const char* szFormat, ...)
{
	va_list vararglist;
	va_start(vararglist, szFormat);
	clvprintf(CTC_YELLOW, 0, szFormat, vararglist);
	va_end(vararglist);
}