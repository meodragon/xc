@echo off

if "%1%"=="env" goto env
if "%1%"=="run" (
    goto run
) else (
    :: usage
    echo build env - load environment variables to current terminal
    echo build run - compile objects then run the program
    goto done
)

:env
SET ENV="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
CALL %ENV%
goto done

:run
if not exist "build/" mkdir build
set "shader_file_list=assimp.vert assimp.frag"
for %%a in (%shader_file_list%) do (
    %VULKAN_SDK%\Bin\glslc.exe -c shader/%%a -o build/%%a.spv
    echo %%a
)
set FLAGS=/Fo"build/" /Fe:"build/"
set SOURCES=main.cpp surface/*.cpp grx/*.cpp model/*.cpp %VK_BOOTSTRAP_SDK%/*.cpp
set LIBRARIES=vulkan-1.lib user32.lib assimp-vc143-mtd.lib

rc surface/icon.rc
cl /std:c++20 /EHsc %FLAGS% %SOURCES% /I %VULKAN_SDK%/Include /I surface /I grx /I %ASSIMP_SDK%/include /I %VK_BOOTSTRAP_SDK% /link /SUBSYSTEM:CONSOLE /LIBPATH:"%VULKAN_SDK%/Lib" /LIBPATH:"%ASSIMP_SDK%/lib/Debug" %LIBRARIES% surface/icon.res && (
    echo BUILD: SUCCESS
    .\build\main
) || (
    echo BUILD: FAILURE
)

:done