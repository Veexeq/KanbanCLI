@echo off
REM =========================================================================
REM  Kanban Board TUI - Adaptive Fullscreen Startup Launcher
REM =========================================================================

REM Step 1: Detect if running in maximized context; if not, self-relaunch flags
if not "%1"=="max" (
    start /max cmd /c ""%~dpnx0" max"
    exit
)

REM Step 2: Override the generic terminal window title string
title Kanban Board TUI

REM Step 3: Forcefully adjust the active shell window buffer dimensions
mode con: cols=120 lines=35

REM Step 4: Clear any visual system command artifacts from the terminal history
cls

REM Step 5: Adaptive path routing - Detect environment (Dev vs Production Bundle)
if exist "build\kanban_board.exe" (
    build\kanban_board.exe
) else if exist "kanban_board.exe" (
    kanban_board.exe
) else (
    echo [ERROR] Critical execution payload context missing!
    echo Could not locate 'kanban_board.exe' in active environment directories.
    echo Please ensure the package is fully unzipped before launching.
    echo.
    pause
    exit /b 1
)

REM Step 6: Prevent the console framework from snapping closed instantly on exit
pause
