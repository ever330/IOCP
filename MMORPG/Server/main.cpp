#include "pch.h"
#include "MainServer.h"

#include <locale>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

int main()
{
	// 콘솔 설정
	// UTF-8 코드페이지 설정
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	// 로케일 설정
	std::locale::global(std::locale(""));
	std::wcout.imbue(std::locale(""));
	std::cout.imbue(std::locale(""));

	// 콘솔 폰트를 한글 지원하는 폰트로 변경
	CONSOLE_FONT_INFOEX cfi = { 0 };
	cfi.cbSize = sizeof(cfi);
	cfi.dwFontSize.Y = 16;
	cfi.FontWeight = FW_NORMAL;
	wcscpy_s(cfi.FaceName, L"맑은 고딕");
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	MainServer::Instance().StartServer();
}
