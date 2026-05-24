@echo off
REM =========================================================================
REM  Kanban Board TUI - Adaptive Windows OS Native Startup Launcher
REM =========================================================================

REM Step 1: Override the generic terminal window title string
title Kanban Board TUI

REM Step 2: Forcefully adjust the active shell window buffer dimensions
mode con: cols=120 lines=35

REM Step 3: Clear any visual system command artifacts from the terminal history
cls

REM Step 4: Adaptive path routing - Detect environment (Dev vs Production Bundle)
if exist "build\kanban_board.exe" (
    REM Local development path execution
    build\kanban_board.exe
) else if exist "kanban_board.exe" (
    REM Flat deployment release path execution
    kanban_board.exe
) else (
    REM Error failsafe handling
    echo [ERROR] Critical execution payload context missing!
    echo Could not locate 'kanban_board.exe' in active environment directories.
    echo Please ensure the package is fully unzipped before launching.
    echo.
    pause
    exit /b 1
)

REM Step 5: Prevent the console framework from snapping closed instantly on exit
pause
