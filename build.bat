@echo off
	rem Build all configurations

	echo Building Debug-Win32 ...
	if exist target\Debug-Win32 goto d32
	mkdir target\Debug-Win32
:d32
	msbuild procctrl.sln /p:Configuration=Debug /p:Platform=Win32 > target\Debug-Win32\build.log
	if errorlevel 1 goto fail
	
	echo Building Debug-x64 ...
	if exist target\Debug-x64 goto d64
	mkdir target\Debug-x64
:d64
	msbuild procctrl.sln /p:Configuration=Debug /p:Platform=x64 > target\Debug-x64\build.log
	if errorlevel 1 goto fail

	echo Building Release-Win32 ...
	if exist target\Release-Win32 goto r32
	mkdir target\Release-Win32
:r32
	msbuild procctrl.sln /p:Configuration=Release /p:Platform=Win32 > target\Release-Win32\build.log
	if errorlevel 1 goto fail

	echo Building Release-x64 ...
	if exist target\Release-x64 goto r64
	mkdir target\Release-x64
:r64
	msbuild procctrl.sln /p:Configuration=Release /p:Platform=x64 > target\Release-x64\build.log
	if errorlevel 1 goto fail

	echo Testing Debug-Win32 ...
	target\Debug-Win32\test > target\Debug-Win32\test.log
	if errorlevel 1 goto fail

	echo Testing Debug-x64 ...
	target\Debug-x64\test > target\Debug-x64\test.log
	if errorlevel 1 goto fail

	echo Testing Release-Win32 ...
	target\Release-Win32\test > target\Release-Win32\test.log
	if errorlevel 1 goto fail

	echo Testing Release-x64 ...
	target\Release-x64\test > target\Release-x64\test.log
	if errorlevel 1 goto fail

	exit 0
:fail
	echo Build failed.
	exit 1