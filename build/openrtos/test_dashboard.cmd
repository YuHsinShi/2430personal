@echo off

set CFG_PROJECT=%~n0
set BOOTLOADER=1
set ARMLITECODEC=1

call build.cmd

@if not defined NO_PAUSE pause
