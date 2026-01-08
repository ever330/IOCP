# 한글 인코딩 문제 해결 가이드

## 문제
Visual Studio에서는 한글이 정상적으로 보이지만 Cursor에서는 한글이 깨져서 보입니다.

## 원인
- Visual Studio는 기본적으로 소스 파일을 **CP949 (EUC-KR)** 인코딩으로 저장합니다
- Cursor는 기본적으로 **UTF-8** 인코딩으로 파일을 읽습니다
- 인코딩이 맞지 않아 한글이 깨집니다

## 해결 방법

### 방법 1: 파일을 UTF-8로 변환 (권장)

PowerShell에서 다음 명령을 실행하세요:

```powershell
cd c:\Projects\IOCP\MMORPG\Server
.\convert_to_utf8.ps1
```

이 스크립트는 모든 `.cpp`와 `.h` 파일을 UTF-8로 변환합니다.

### 방법 2: Visual Studio에서 UTF-8로 저장

각 파일을 Visual Studio에서 열고:
1. **파일 > 고급 저장 옵션**
2. **인코딩**을 **"UTF-8 with signature"** 또는 **"UTF-8 without signature"**로 선택
3. 저장

### 방법 3: Visual Studio 프로젝트 설정

이미 프로젝트 파일(`Server.vcxproj`)에 `/utf-8` 컴파일 옵션을 추가했습니다.
이제 컴파일러가 소스 파일을 UTF-8로 해석합니다.

## Cursor 설정

`.vscode/settings.json`에 다음 설정이 추가되었습니다:
- `files.encoding`: "utf8"
- `files.autoGuessEncoding`: true (자동으로 인코딩 감지)

## 확인 방법

1. Cursor를 재시작하세요
2. `IOCP.cpp` 파일을 열어서 한글이 정상적으로 보이는지 확인하세요
3. 여전히 깨진다면 `convert_to_utf8.ps1` 스크립트를 실행하세요

## 주의사항

- 파일을 UTF-8로 변환한 후 Visual Studio에서도 정상적으로 보여야 합니다
- 팀원들과 협업할 때는 인코딩을 통일하는 것이 중요합니다
- Git에서 파일 변경사항이 많이 표시될 수 있습니다 (인코딩만 변경된 경우)
