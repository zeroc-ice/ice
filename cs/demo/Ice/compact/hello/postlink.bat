
@echo copying Content Files from Resource Files folder to Targeted Debug Directory
copy ".\Resources\*.*" "%SG_OUTPUT_ROOT%\oak\target\%_TGTCPU%\%WINCEDEBUG%"  /Y

@echo copying Content Files from Resource Files folder to FlatRelease Directory
copy ".\Resources\*.*" %_FLATRELEASEDIR%  /Y
@echo copying DLLs FlatRelease Directory
copy "..\..\..\..\Assemblies\cf\Ice.dll" %_FLATRELEASEDIR%  /Y

@echo .
@echo Building .cab file
@echo .

PUSHD
cd %_FLATRELEASEDIR%
IF EXIST hello.inf (
    cabwiz hello.inf
	IF EXIST hello.cab (
		@echo Generated .cab file: hello.cab in FLATRELEASEDIR.
	) else (
		@echo Generation of .cab file: hello.cab failed.
	)
)else (
	@echo No file hello.inf for .cab file generation
)
 
POPD

@echo .
@echo Done Copying
@echo .

:SKIP

