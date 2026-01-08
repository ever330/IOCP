# 소스 파일을 UTF-8로 변환하는 스크립트
# PowerShell에서 실행: .\convert_to_utf8.ps1

Write-Host "소스 파일을 UTF-8로 변환 중..." -ForegroundColor Green

$files = Get-ChildItem -Path . -Include *.cpp,*.h -Recurse | Where-Object { $_.FullName -notlike "*\x64\*" -and $_.FullName -notlike "*\.vs\*" }

$converted = 0
$skipped = 0

foreach ($file in $files) {
    try {
        # 파일을 CP949로 읽기
        $content = Get-Content -Path $file.FullName -Encoding Default -Raw
        
        # UTF-8 BOM 없이 저장
        [System.IO.File]::WriteAllText($file.FullName, $content, [System.Text.UTF8Encoding]::new($false))
        
        Write-Host "변환 완료: $($file.Name)" -ForegroundColor Yellow
        $converted++
    }
    catch {
        Write-Host "변환 실패: $($file.Name) - $($_.Exception.Message)" -ForegroundColor Red
        $skipped++
    }
}

Write-Host "`n변환 완료: $converted 개 파일" -ForegroundColor Green
if ($skipped -gt 0) {
    Write-Host "건너뜀: $skipped 개 파일" -ForegroundColor Yellow
}

Write-Host "`n주의: Visual Studio에서 파일을 다시 열 때 UTF-8로 저장하도록 설정하세요." -ForegroundColor Cyan
