@REM Proxy file that calls nmake with INCL set to all header directories.
@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

@REM Careful to not exceed the 8192 string limit.
SET "INCL=..\include\*"
FOR /F "delims=" %%I IN ('DIR /B /S /AD ..\include') DO (
	SET "INCL=%%I\* !INCL!"
)

@REM Suppresses CMD terminate prompt and error code.
nmake /C %* || (
	SET "LEVEL=!ERRORLEVEL!"
	CALL;
	EXIT /B !LEVEL!
)
ENDLOCAL
