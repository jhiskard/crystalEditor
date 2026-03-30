@echo off
setlocal EnableExtensions

rem Single-file helper for loading emsdk env in a PowerShell session.
rem Usage (from repo root):
rem   .\emsdk.bat
rem Optional:
rem   .\emsdk.bat C:\Users\user\Downloads\emsdk

set "REPO_ROOT=%~dp0"
for %%I in ("%REPO_ROOT%.") do set "REPO_ROOT=%%~fI"

if /i "%~1"=="--emit" goto emit_only

set "EMSDK_ROOT=%~1"
if "%EMSDK_ROOT%"=="" (
  if exist "%REPO_ROOT%\..\emsdk\emsdk_env.bat" (
    set "EMSDK_ROOT=%REPO_ROOT%\..\emsdk"
  ) else if exist "%REPO_ROOT%\emsdk\emsdk_env.bat" (
    set "EMSDK_ROOT=%REPO_ROOT%\emsdk"
  )
)

if "%EMSDK_ROOT%"=="" (
  >&2 echo ERROR: emsdk directory not found.
  >&2 echo Expected one of:
  >&2 echo   %REPO_ROOT%\..\emsdk
  >&2 echo   %REPO_ROOT%\emsdk
  >&2 echo Or pass explicit path:
  >&2 echo   .\emsdk.bat C:\path\to\emsdk
  exit /b 1
)

for %%I in ("%EMSDK_ROOT%") do set "EMSDK_ROOT=%%~fI"
set "EMSDK_ENV_BAT=%EMSDK_ROOT%\emsdk_env.bat"

if not exist "%EMSDK_ENV_BAT%" (
  >&2 echo ERROR: emsdk_env.bat not found: "%EMSDK_ENV_BAT%"
  exit /b 1
)

echo [INFO] Loading emsdk variables from:
echo [INFO]   %EMSDK_ENV_BAT%
echo [INFO] Opening a PowerShell shell with emsdk environment applied...

powershell -NoLogo -NoExit -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop';" ^
  "$cmds = & '%~f0' --emit '%EMSDK_ROOT%';" ^
  "if ($LASTEXITCODE -ne 0) { throw 'Failed to emit emsdk environment commands.' }" ^
  "if (-not $cmds) { throw 'No emsdk environment commands were emitted.' }" ^
  "Invoke-Expression ($cmds -join \"`n\");" ^
  "Set-Location '%REPO_ROOT%';" ^
  "Write-Host 'EMSDK environment loaded in this shell.' -ForegroundColor Green"

exit /b %ERRORLEVEL%

:emit_only
set "EMSDK_ROOT=%~2"
if "%EMSDK_ROOT%"=="" (
  >&2 echo ERROR: internal call requires emsdk path.
  exit /b 1
)
for %%I in ("%EMSDK_ROOT%") do set "EMSDK_ROOT=%%~fI"
set "EMSDK_ENV_BAT=%EMSDK_ROOT%\emsdk_env.bat"
if not exist "%EMSDK_ENV_BAT%" (
  >&2 echo ERROR: emsdk_env.bat not found: "%EMSDK_ENV_BAT%"
  exit /b 1
)

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$batPath = '%EMSDK_ENV_BAT%';" ^
  "$cmd = ('set EMSDK_QUIET=1&& call \"{0}\" >nul 2>nul && set' -f $batPath);" ^
  "$lines = cmd /d /c $cmd;" ^
  "if ($LASTEXITCODE -ne 0) { throw ('Failed to load emsdk env: ' + $batPath) }" ^
  "$lines | ForEach-Object {" ^
  "  $pair = $_ -split '=', 2;" ^
  "  if ($pair.Length -eq 2) {" ^
  "    $name = $pair[0];" ^
  "    $value = $pair[1].Replace(\"'\", \"''\");" ^
  "    \"[Environment]::SetEnvironmentVariable('{0}','{1}','Process')\" -f $name, $value" ^
  "  }" ^
  "}"

exit /b %ERRORLEVEL%
