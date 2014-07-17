
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
IF EXIST bidir.inf (
    cabwiz bidir.inf
	IF EXIST bidir.cab (
		@echo Generated .cab file: bidir.cab in FLATRELEASEDIR.
	) else (
		@echo Generation of .cab file: bidir.cab failed.
	)
)else (
	@echo No file bidir.inf for .cab file generation
)
 
POPD

@echo .
@echo Done Copying
@echo .

:SKIP

