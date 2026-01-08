@echo off
chcp 65001 >nul
echo ========================================
echo IOCP Server 빌드 스크립트
echo ========================================
echo.

cd /d "%~dp0"

REM Visual Studio 2022 Developer Command Prompt 찾기
set "VS2022_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
if not exist "%VS2022_PATH%" (
    set "VS2022_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat"
)
if not exist "%VS2022_PATH%" (
    set "VS2022_PATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat"
)

if exist "%VS2022_PATH%" (
    echo [정보] Visual Studio 환경 설정 중...
    call "%VS2022_PATH%"
    echo.
    echo [정보] 프로젝트 빌드 중...
    msbuild Server.sln /property:Configuration=Debug /property:Platform=x64 /t:Build /m
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo [성공] 빌드가 완료되었습니다!
        echo 실행 파일 위치: x64\Debug\Server.exe
    ) else (
        echo.
        echo [오류] 빌드에 실패했습니다.
        exit /b 1
    )
) else (
    echo [오류] Visual Studio를 찾을 수 없습니다.
    echo Visual Studio 2022가 설치되어 있는지 확인해주세요.
    echo.
    echo 또는 수동으로 Visual Studio에서 Server.sln을 열어 빌드하세요.
    exit /b 1
)

pause
