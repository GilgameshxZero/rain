@ECHO OFF
REM Increments build version number at compile-time.

REM LF newline variable.
(SET \n=^

)

SETLOCAL ENABLEDELAYEDEXPANSION
SET FILE_PATH=%~dp0..\include\rain\build.hpp
FOR /F "tokens=1-3 delims= " %%A IN (%FILE_PATH%) DO (
	SET /A BUILD=%%C+1

	REM ECHO with custom LF-only newline.
	<NUL SET /P=%%A %%B !BUILD!!\n!> %FILE_PATH%
)
ECHO VERSION_BUILD: !BUILD!.
ENDLOCAL
