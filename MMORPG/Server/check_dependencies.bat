@echo off
chcp 65001 >nul
echo ========================================
echo IOCP 서버 의존성 확인 스크립트
echo ========================================
echo.

cd /d "%~dp0"

set ERROR_COUNT=0

echo [1/5] 실행 파일 확인...
if exist "x64\Debug\Server.exe" (
    echo     [OK] Server.exe 존재
) else (
    echo     [오류] Server.exe를 찾을 수 없습니다. 빌드가 필요합니다.
    set /a ERROR_COUNT+=1
)
echo.

echo [2/5] DLL 파일 확인...
if exist "x64\Debug\libcrypto-3-x64.dll" (
    echo     [OK] libcrypto-3-x64.dll 존재
) else (
    echo     [경고] libcrypto-3-x64.dll을 찾을 수 없습니다.
)
if exist "x64\Debug\libssl-3-x64.dll" (
    echo     [OK] libssl-3-x64.dll 존재
) else (
    echo     [경고] libssl-3-x64.dll을 찾을 수 없습니다.
)
if exist "x64\Debug\mysqlcppconn-10-vs14.dll" (
    echo     [OK] mysqlcppconn-10-vs14.dll 존재
) else (
    echo     [경고] mysqlcppconn-10-vs14.dll을 찾을 수 없습니다.
)
if exist "x64\Debug\mysqlcppconnx-2-vs14.dll" (
    echo     [OK] mysqlcppconnx-2-vs14.dll 존재
) else (
    echo     [경고] mysqlcppconnx-2-vs14.dll을 찾을 수 없습니다.
)
echo.

echo [3/5] 포트 3030 사용 가능 여부 확인...
netstat -an | findstr ":3030" >nul
if %ERRORLEVEL% EQU 0 (
    echo     [경고] 포트 3030이 이미 사용 중입니다.
) else (
    echo     [OK] 포트 3030 사용 가능
)
echo.

echo [4/5] MySQL 연결 테스트...
powershell -Command "try { $tcpClient = New-Object System.Net.Sockets.TcpClient; $tcpClient.Connect('127.0.0.1', 3306); $tcpClient.Close(); Write-Host '    [OK] MySQL 서버에 연결 가능' } catch { Write-Host '    [오류] MySQL 서버에 연결할 수 없습니다 (127.0.0.1:3306)' }"
if %ERRORLEVEL% NEQ 0 (
    set /a ERROR_COUNT+=1
)
echo.

echo [5/5] Redis 연결 테스트...
powershell -Command "try { $tcpClient = New-Object System.Net.Sockets.TcpClient; $tcpClient.Connect('127.0.0.1', 6379); $tcpClient.Close(); Write-Host '    [OK] Redis 서버에 연결 가능' } catch { Write-Host '    [오류] Redis 서버에 연결할 수 없습니다 (127.0.0.1:6379)' }"
if %ERRORLEVEL% NEQ 0 (
    set /a ERROR_COUNT+=1
)
echo.

echo ========================================
if %ERROR_COUNT% EQU 0 (
    echo 모든 검사를 통과했습니다!
    echo 서버를 실행할 준비가 되었습니다.
) else (
    echo %ERROR_COUNT%개의 문제가 발견되었습니다.
    echo 위의 오류를 해결한 후 다시 시도해주세요.
)
echo ========================================
echo.

pause
