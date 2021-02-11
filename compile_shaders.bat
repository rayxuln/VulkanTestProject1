@echo off
setlocal EnableDelayedExpansion

echo compiling shaders from %1 to %2...
set fromDir=%1
set toDir=%2
set glValidation=%3

if not defined fromDir (
    echo from dir is empty
    exit
)

if not defined toDir (
    echo to dir is empty
    exit
)

if not defined glValidation (
    echo glslValidation tool is empty
    exit
)

echo glsl validation: %glValidation%

for %%f in (%fromDir%/*.*) do (
    set "fileName=%%~nxf"
    echo compiling !fileName!...
    %glValidation% -V %fromDir%/!fileName! -o %toDir%/!fileName:.=_!.spv
)