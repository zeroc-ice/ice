Description
-----------

The Ice Visual Studio Extension allows integration of Ice projects
into the Visual Studio IDE. The extension supports C++, .NET, and
Silverlight projects.


Activating the plug-in for a project
------------------------------------

After installing the plug-in, right-click on the project in
Solution Explorer and choose "Ice Configuration..." or go to 
"Ice Configuration..." in the "Tools" menu. This opens a dialog where
you can configure Ice build properties.


Project properties
------------------

  * Ice Home: The directory where Ice is installed

  * Slice Compiler Options

    Tick the correspond check boxes to add --ice, --stream, or --tie
    (.NET only).

  * Preprocessor Macros: List of macro definitions passed to the Slice
    compiler

    Enter preprocess macros separated by semicolons. For example, 
    enter

    FOO;BAR=99

    to pass

    -DFOO -DBAR=99

    to the Slice compiler.

  * Slice Include Path: The list of directories to search for included
    Slice files (-I option)

  * Ice Components: The list of Ice libraries with which to link


Adding Slice files to a project
-------------------------------

Use "Add -> New Item..." to create a Slice file to a project. Use
"Slice source" as the file type. To add an existing Slice file, use
"Add -> Existing Item...".


Generating code
---------------

The extension compiles a Slice file whenever you save the file. The
extension tracks dependencies among Slice files and recompiles only
those files that require it after a change.

Generated files are automatically added to the project. For example,
for Demo.ice, the extension for C++ adds Demo.cpp and Demo.h to the
project, whereas the extension for C# adds Demo.cs to the project.

Slice Compilation errors are displayed in the Visual Studio "Output"
and "Error List" panels.

VC++ Pre-compiled headers
-------------------------

For c++ projects the pre-compiler headers is detect automatically and
slice compiler will add the relevant --add-header command line option.

If you change the pre-compiler header setting of a projec you should
rebuild the project.
