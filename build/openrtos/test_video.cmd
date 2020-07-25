@echo off

set CFG_PROJECT=%~n0
set ARMLITECODEC=1


call build.cmd

@if not defined NO_PAUSE pause
