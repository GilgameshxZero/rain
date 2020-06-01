@ECHO OFF

REM Increments RAIN_VERSION_BUILD every build.

SETLOCAL ENABLEDELAYEDEXPANSION
SET versionpath=%~dp0..\include\rain-version-build.hpp
FOR /F "tokens=1-3 delims= " %%A IN (%versionpath%) DO (
	SET /A versionbuild=%%C+1
	ECHO | SET /P="%%A %%B !versionbuild!" > %versionpath%
)
ECHO build !versionbuild!
ENDLOCAL
