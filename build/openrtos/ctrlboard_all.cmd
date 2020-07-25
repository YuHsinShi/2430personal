@echo off

set BOOTLOADER=1
set ALT_CPU=1
set ARMLITECODEC=1

setlocal EnableDelayedExpansion
set TARGETCMD=%~n0.cmd
call %TARGETCMD:_all=!!%

@if not defined NO_PAUSE pause
