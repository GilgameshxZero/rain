@ECHO OFF

REM Increments RAIN_VERSION_BUILD every build.

SETLOCAL ENABLEDELAYEDEXPANSION
SET file=rain-version.hpp
SET path=%~dp0..\include\%file%
IF EXIST "%path%.tmp" DEL "%path%.tmp"
FOR /F "tokens=*" %%R IN (%path%) DO (
	IF "%%R"=="" ECHO. >> "%path%.tmp"
	SET isbuildline=0
	FOR /F "tokens=1-3 delims= " %%A IN ("%%R") DO (
		IF "%%B"=="RAIN_VERSION_MAJOR" SET versionmajor=%%C
		IF "%%B"=="RAIN_VERSION_MINOR" SET versionminor=%%C
		IF "%%B"=="RAIN_VERSION_REVISION" SET versionrevision=%%C
		IF "%%B"=="RAIN_VERSION_BUILD" SET isbuildline=1
		IF !isbuildline! EQU 1 (
			SET /A versionbuild=%%C+1
			ECHO %%A %%B !versionbuild!>> "%path%.tmp"
			ECHO rain v!versionmajor!.!versionminor!.!versionrevision!.!versionbuild!
		) ELSE (
			ECHO %%R>> "%path%.tmp"
		)
	)
)
DEL "%path%"
REN "%path%.tmp" "%file%"
ENDLOCAL
