@echo off

set CFG_PROJECT=%~n0

call build.cmd

@if not defined NO_PAUSE pause
