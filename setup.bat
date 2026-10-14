@echo off
rem One-time setup: picks your Unreal Engine, points Aetheria at it, and opens the editor.
rem Options: setup.bat -Engine 5.6   (choose a version)   setup.bat -NoLaunch   (don't open the editor)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Tools\setup.ps1" %*
pause
