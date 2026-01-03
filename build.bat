@echo off

REM Remember that paths should not end in '\' character on Windows because fuck you
SET lib_glad_inc="..\thirdparty\glad\include"
SET lib_glad_src="..\thirdparty\glad\src"
SET lib_glfw_inc="..\thirdparty\glfw-3.4\include"
SET lib_glfw_lib="..\thirdparty\glfw-3.4\lib-vc2022"
SET lib_glm_inc="..\thirdparty\glm-1.0.3"

SET build_directory="compiled"

if not exist %build_directory% mkdir %build_directory%

pushd %build_directory%

cl /EHsc^
	/Zi^
	/Wall^
	/wd5045^
	/Fe"brave-new-world.exe"^
	..\src\brave-new-world.cpp^
	%lib_glad_src%\glad.c^
	/DGLFW_STATIC^
	/MT^
	/I%lib_glad_inc%^
	/I%lib_glfw_inc%^
	/I%lib_glm_inc%^
	/link /NODEFAULTLIB:msvcrt.lib libcmt.lib user32.lib %lib_glfw_lib%\glfw3_mt.lib gdi32.lib opengl32.lib Shell32.lib

if %ERRORLEVEL% neq 0 (
	echo Error: MSVC failed with return code %ERRORLEVEL%.
	popd
	exit /b %ERRORLEVEL%
) else (
	echo Succes: MSVC executed successfully.
)

popd
