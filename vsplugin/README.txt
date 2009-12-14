This archive contains the source code for the Ice Visual Studio
Extension.

For installation instructions, please refer to the INSTALL.txt file.

If you would like to install the Ice Visual Studio Extension without
compiling any source code, you can use the binary distribution
available at ZeroC's web site (http://www.zeroc.com).


Description
-----------

The Ice Visual Studio Extension integrates Ice projects into the
Visual Studio IDE. The extension supports C++, .NET, and Silverlight
projects.


Activating the plug-in for a project
------------------------------------

After installing the plug-in, right-click on the project in Solution
Explorer and choose "Ice Configuration..." or go to "Ice
Configuration..." in the "Tools" menu. This opens a dialog where you
can configure Ice build properties.

Note that after adding new configurations or platforms to your
project, it may be necessary to disable and then re-enable the plug-in
in order for the new configuration/platform to get the correct Ice
settings.


Project properties
------------------

  * Ice Home

    Set the directory where Ice is installed.

  * Slice Compiler Options

    Tick the corresponding check boxes to pass options such as --ice,
    --stream, --checksum, or --tie (.NET only) to the Slice compiler.

    Tick "Console Output" if you want compiler output to appear in the
    Output window.

  * Extra Compiler Options

    Add extra Slice compiler options that are not explicitly supported
    above.

    These options must be entered the same as they would be on the command
    line to the Slice compiler. For example, preprocessor macros can be
    defined by entering the following:

    -DFOO -DBAR

  * Slice Include Path

    Set the list of directories to search for included Slice files
    (-I option).

    The checkbox for each path specifies whether the path should be
    relative to the project directory or an absolute path. Note that
    paths that use environment variables are not changed by this
    feature.

  * DLL Export Symbol (C++ only)

    Set the symbol to use for DLL exports (--dll-export option).

  * Ice Components

    Set the list of Ice libraries to link with.


Environment Variables
---------------------

The "Ice Home", "Extra Compiler Options", and "Slice Include Path"
settings support the use of environment variables. Use the $(VAR)
syntax to refer to an environment variable named VAR. For example,
for "Ice Home" you could use $(ICE_HOME).


Adding Slice files to a project
-------------------------------

Use "Add -> New Item..." to create a Slice file and add it to a
project.  Use "Slice File (.ice)" as the file type. To add an existing
Slice file, use "Add -> Existing Item...".


Generating code
---------------

The extension compiles a Slice file whenever you save the file. The
extension tracks dependencies among Slice files in the project and
recompiles only those files that require it after a change.

Generated files are automatically added to the project. For example,
for Demo.ice, the extension for C++ adds Demo.cpp and Demo.h to the
project, whereas the extension for C# adds Demo.cs to the project.

Errors that occur during Slice compilation are displayed in the Visual
Studio "Output" and "Error List" panels.


VC++ Pre-compiled headers
-------------------------

For C++ projects, pre-compiled headers are detected automatically.
(The extension automatically passes the required --add-header option
to slice2cpp.)

If you change the pre-compiled header setting of a project, you must
rebuild the project.
