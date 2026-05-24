@echo off
REM =========================================================================
REM  Kanban Board TUI - Windows OS Native Startup Launcher
REM =========================================================================

REM Step 1: Override the generic terminal window title string
title Kanban Board TUI

REM Step 2: Forcefully adjust the active shell window buffer dimensions
mode con: cols=120 lines=35

REM Step 3: Clear any visual system command artifacts from the terminal history
cls

REM Step 4: Execute the optimized Release binary payload layout quietly
build\kanban_board.exe

REM Step 5: Prevent the console framework from snapping closed instantly on exit
pause
