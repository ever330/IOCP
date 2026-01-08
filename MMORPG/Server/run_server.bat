@echo off
chcp 65001 >nul
REM UTF-8 코드 페이지 설정
echo ========================================
echo IOCP Server 실행 스크립트
echo ========================================
echo.

cd /d "%~dp0"

if not exist "x64\Debug\Server.exe" (
    echo [오류] Server.exe를 찾을 수 없습니다.
    echo 빌드를 먼저 수행해주세요.
    pause
    exit /b 1
)

echo [정보] 서버를 시작합니다...
echo [정보] 포트: 3030
echo [정보] MySQL: 127.0.0.1:3306
echo [정보] Redis: 127.0.0.1:6379
echo.
echo 서버를 중지하려면 Ctrl+C를 누르세요.
echo.

cd x64\Debug
Server.exe

pause
