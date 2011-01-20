For installation instructions, please refer to the INSTALL.txt file.

If you would like to install the Ice Visual Studio Add-In without
compiling any source code, you can use the binary distribution
available at ZeroC's web site (http://www.zeroc.com).


Description
-----------

The Ice Visual Studio Add-In integrates Ice projects into the
Visual Studio IDE. The add-in supports C++, .NET, VB and
Silverlight projects.


Activating the add-in for a project
------------------------------------

After installing the add-in, right-click on the project in Solution
Explorer and choose "Ice Configuration..." or go to "Ice
Configuration..." in the "Tools" menu. This opens a dialog where you
can configure Ice build properties.

To enable the add-in for your project, click "Enable Ice Builder" and
then click "Apply" or "Ok" to commit the changes.

Note that after adding new configurations or platforms to your
project, it may be necessary to disable and then re-enable the Ice 
add-in for that project, in order for the new configuration/platform 
to have the correct Ice settings.

To do that, open the "Ice Configuration..." dialog, uncheck "Enable
Ice Builder", click "Apply", then check "Enable Ice Builder" and click
"Apply" or "Ok".


Project properties
------------------

* Ice Home

  Set the directory where Ice is installed.

* Output Dir

  Set the base directory where generated files will be placed.

* Slice Compiler Options

  Tick the corresponding check boxes to pass options such as --ice,
  --stream, --checksum, or --tie (.NET only) to the Slice compiler.

  You can change the verbosity of messages printed to the output
  window by selecting a different Trace Level, where Error is less
  verbose and Debug is more verbose.

* Extra Compiler Options

  Add extra Slice compiler options that are not explicitly supported
  above.

  These options must be entered the same as they would be on the
  command line to the Slice compiler. For example, preprocessor
  macros can be defined by entering the following:

  -DFOO -DBAR

* Slice Include Path

  Set the list of directories to search for included Slice files
  (-I option).

  The checkbox for each directory indicates whether it should be
  stored as an absolute path or converted to a path that is relative
  to the project directory. The add-in stores an absolute path if
  the box is checked, otherwise the add-in attempts to convert the
  directory into a relative path. If the directory cannot be converted
  into a relative path, the directory is stored as an absolute path.
  Directories that use environment variables (see below) are not
  affected by this feature.

* DLL Export Symbol (C++ only)

  Set the symbol to use for DLL exports (--dll-export option).

* Ice Components

  Set the list of Ice libraries to link with.


Environment Variables
---------------------

The "Ice Home", "Output Dir", "Extra Compiler Options", and "Slice
Include Path" settings support the use of environment variables. Use
the $(VAR) syntax to refer to an environment variable named VAR. For
example,if you have defined the ICE_HOME environment variable, you
could use $(ICE_HOME) in the "Ice Home" field.

You cannot use environment variables in the "--header-ext" and 
"--source-ext" options in "Extra Compiler Options".

If you change environment variable values outside of Visual Studio,
you will need to restart Visual Studio to use the new environment
values.


Adding Slice files to a project
-------------------------------

Use "Add -> New Item..." to create a Slice file and add it to a
project. Use "Slice File (.ice)" as the file type. To add an existing
Slice file, use "Add -> Existing Item...".


Generating code
---------------

The add-in compiles a Slice file whenever you save the file. The
add-in tracks dependencies among Slice files in the project and
recompiles only those files that require it after a change.

Generated files are automatically added to the project. For example,
for Demo.ice, the add-in adds Demo.cpp and Demo.h to a C++ project,
whereas the add-in adds Demo.cs to a C# project.

The default is to place the generated files in the same directory
as the corresponding Slice file. You can change the directory where
the generated files are placed using the "Output Dir" option in the
configuration dialog; the path set here will be used as the base
directory for storing the generated files.

Errors that occur during Slice compilation are displayed in the Visual
Studio "Output" and "Error List" panels.


VC++ Pre-compiled headers
-------------------------

For C++ projects, pre-compiled headers are detected automatically.
(The add-in automatically passes the required --add-header option
to slice2cpp.)

If you change the pre-compiled header setting of a project, you must
rebuild the project.


.NET DEVPATH support
--------------------

The add-in detects if a .NET project is configured for development
mode by inspecting the <application-name>.config.exe file. If a
project is in development mode, the Ice bin directory is automatically
added to the DEVPATH environment variable when the demo is run;
references to Ice components are also set with Copy Local to false to
avoid copying Ice references to the project's output directory. Note
that the Copy Local setting is not changed for references that are
already added to the project.

For more information about .NET DEVPATH see:

  http://msdn.microsoft.com/en-us/library/cskzh7h6.aspx


Command-line builds
-------------------

The add-in supports command-line builds using devenv. For example:

devenv MyProject.sln /build

Note that for this to work, command-line builds must be enabled for
the add-in in the IDE; see "Tools -> Add-in Manager" and check 
"Command Line" for Ice.
