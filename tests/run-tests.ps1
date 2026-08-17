# wbwlib 冒烟测试脚本
# 用法：pwsh tests/run-tests.ps1
#       以 -std=c++14 与 -std=c++17 分别编译并运行 tests/compile-test.cpp。
# 可选参数：-GXX 指定编译器，-Std 只测单一标准（如 -Std c++17），
#           -Filter 只跑指定小节（逗号分隔：core,math,ds,str,graph,dp,geo,misc）。

param(
    [string]$GXX = "g++",
    [string]$Std = "",
    [string]$Filter = ""
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$src = Join-Path $PSScriptRoot "compile-test.cpp"

$stds = if ($Std -ne "") { @($Std) } else { @("c++14", "c++17") }

foreach ($s in $stds) {
    $tag = $s -replace '\+', 'p'
    $exe = Join-Path $env:TEMP "wbwlib_test_$tag.exe"

    Write-Host "== 编译 (-std=$s) =="
    & $GXX -O2 "-std=$s" -Wall -Wextra -I $root $src -o $exe
    if ($LASTEXITCODE -ne 0) {
        Write-Error "编译失败：-std=$s"
        exit 1
    }

    Write-Host "== 运行 (-std=$s) =="
    if ($Filter -ne "") { & $exe $Filter } else { & $exe }
    if ($LASTEXITCODE -ne 0) {
        Write-Error "测试失败：-std=$s（退出码 $LASTEXITCODE）"
        exit 1
    }
    Remove-Item -Force $exe -ErrorAction SilentlyContinue
}

Write-Host "全部通过：$($stds -join ' / ')"
