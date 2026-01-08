# IOCP 서버 실행 가이드

## 사전 요구사항

### 1. 데이터베이스 설정
- **MySQL** 서버가 실행 중이어야 합니다
  - 호스트: `127.0.0.1:3306`
  - 사용자: `root`
  - 비밀번호: `rainbow@@`
  - 데이터베이스: `rainbow`

### 2. Redis 설정
- **Redis** 서버가 실행 중이어야 합니다
  - 호스트: `127.0.0.1:6379`
  - 비밀번호: `1234`

### 3. 필요한 DLL 파일
다음 DLL 파일들이 `x64/Debug/` 폴더에 있어야 합니다:
- `libcrypto-3-x64.dll`
- `libssl-3-x64.dll`
- `mysqlcppconn-10-vs14.dll`
- `mysqlcppconnx-2-vs14.dll`

## 실행 방법

### 방법 1: Cursor에서 실행 (권장)

1. **F5 키**를 누르거나
2. 왼쪽 사이드바의 **"Run and Debug"** 아이콘 클릭
3. **"Debug Server"** 또는 **"Run Server (No Debug)"** 선택 후 실행

### 방법 2: 배치 파일로 실행

1. `run_server.bat` 파일을 더블클릭하거나
2. 터미널에서 실행:
   ```powershell
   .\run_server.bat
   ```

### 방법 3: 직접 실행

이미 빌드된 실행 파일이 있다면:
```powershell
cd x64\Debug
.\Server.exe
```

## 빌드 방법

### 방법 1: Cursor에서 빌드
- **Ctrl+Shift+B** 키를 누르면 자동으로 빌드됩니다

### 방법 2: 배치 파일로 빌드
```powershell
.\build_server.bat
```

### 방법 3: Visual Studio에서 빌드
1. `Server.sln` 파일을 Visual Studio에서 엽니다
2. **빌드 > 솔루션 빌드** (Ctrl+Shift+B)

## 서버 정보

- **포트**: 3030
- **최대 사용자 수**: 1000
- **IOCP 스레드 수**: 8
- **패킷 처리 스레드 수**: 8

## 문제 해결

### 서버가 시작되지 않는 경우
1. MySQL 서버가 실행 중인지 확인
2. Redis 서버가 실행 중인지 확인
3. 필요한 DLL 파일들이 `x64/Debug/` 폴더에 있는지 확인
4. 포트 3030이 이미 사용 중인지 확인

### 빌드 오류가 발생하는 경우
1. Visual Studio 2022가 설치되어 있는지 확인
2. 프로젝트 속성에서 경로 설정 확인:
   - MySQL Connector: `C:\mysql_connector_cpp\`
   - OpenSSL: `C:\Program Files\OpenSSL-Win64\`

### 데이터베이스 연결 오류
- MySQL 서버가 실행 중인지 확인
- 데이터베이스 이름이 `rainbow`인지 확인
- 사용자 이름과 비밀번호가 올바른지 확인

### Redis 연결 오류
- Redis 서버가 실행 중인지 확인
- Redis 비밀번호가 `1234`인지 확인
- Redis가 포트 6379에서 실행 중인지 확인

## 설정 변경

데이터베이스나 Redis 연결 정보를 변경하려면 `MainServer.cpp` 파일의 다음 부분을 수정하세요:

```cpp
// MySQL 연결 정보 (16번째 줄)
m_DBManager->Initialize("tcp://127.0.0.1:3306", "root", "rainbow@@", "rainbow");

// Redis 연결 정보 (18번째 줄)
if (RedisManager::Instance().Connect("127.0.0.1", 6379, "1234"))
```

포트를 변경하려면 `Define.h` 파일의 `PORT` 정의를 수정하세요:

```cpp
#define PORT 3030
```
