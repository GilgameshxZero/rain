@IF NOT DEFINED _ECHO ECHO OFF

@REM build.bat takes two parameters and builds the test, targeting x64 platforms.
@REM 1. Name of the test to be built.
@REM 2. If release, then the build will be release.

@REM Call vcvarsall to set environment variables if it hasn't been done already.
@REM Calling vcvarsall multiple times will error.
IF NOT DEFINED VSCMD_VER (
	CALL vcvarsall.default x64
	CALL vcvarsall x64
)

@REM Create relevant directories.
IF NOT EXIST ..\bin\ (
	MD ..\bin\
)
IF NOT EXIST ..\obj\%1\ (
	MD ..\obj\%1\
)

@REM Increment build version number.
CALL increment-version-build

@REM Call different command line options based on whether we are building debug or release.
SETLOCAL ENABLEDELAYEDEXPANSION

SET "COMPILE_OP_COMMON=/I../include/ /std:c++17 /D _CONSOLE /Fa..\obj\%1\ /Fd..\obj\%1\%1.pdb /Fo..\obj\%1\%1 /Fp..\obj\%1\%1 /fp:fast /MP /permissive- /Zc:wchar_t /Zc:forScope /Zc:inline /GS /W3 /WX- /sdl /diagnostics:column /Zf /EHsc /Gm- /nologo"
SET "COMPILE_OP_DEBUG=/D _DEBUG /MDd /Od /RTC1 /JMC /ZI"
SET "COMPILE_OP_RELEASE=/D NDEBUG /MT /O2 /Oi /GL /Gy /Zi"
SET "COMPILE_FILES=..\test\%1.cpp"

SET "LINK_OP_COMMON=/link /OUT:..\bin\%1.exe /PDB:..\obj\%1\%1.pdb /ILK:..\obj\%1\%1.ilk /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /MANIFESTFILE:..\obj\%1\%1.exe.intermediate.manifest /LTCGOUT:..\obj\%1\%1.iobj /SUBSYSTEM:CONSOLE /NOLOGO"
SET "LINK_OP_DEBUG=/DEBUG"
SET "LINK_OP_RELEASE=/INCREMENTAL:NO /OPT:ICF /OPT:REF /LTCG:incremental"
SET "LINK_LIBRARIES="

@REM Compile and link.
IF "%2"=="release" (
	SET "COMPILE_OP=%COMPILE_OP_COMMON% %COMPILE_OP_RELEASE%"
	SET "LINK_OP=%LINK_OP_COMMON% %LINK_OP_RELEASE%"
) ELSE (
	SET "COMPILE_OP=%COMPILE_OP_COMMON% %COMPILE_OP_DEBUG%"
	SET "LINK_OP=%LINK_OP_COMMON% %LINK_OP_DEBUG%"
)

cl %COMPILE_OP% %COMPILE_FILES% %LINK_OP% %LINK_LIBRARIES%
