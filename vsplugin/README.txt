This archive contains the source code for the Ice Visual Studio
Extension.

For installation instructions, please refer to the INSTALL.txt file.

If you would like to install the Ice Visual Studio Extension without
compiling any source code, you can use the binary distributions
available at ZeroC's web site (http://www.zeroc.com).


Description
-----------

The Ice Visual Studio Extension allows integration of Ice projects
into the Visual Studio IDE. The extension supports C++, .NET, and
Silverlight projects.


Activating the plug-in for a project
------------------------------------

After installing the plug-in, right-click on the project in
Solution Explorer and choose "Ice Configuration..." or go to "Ice Configuration..."
in "Tools" menu

This will open a dialog where you could configure Ice build properties.


Project properties
------------------

  * Ice Home: The directory where Ice is installed

    The extension expects the same directory layout as that of an
    Ice binary distribution.

  * Enable Ice: Permit Slice identifiers with an "Ice" prefix (--ice option)
  
  * Enable Tie: Generate tie classes (--tie option).
    This property only applies to .NET projects.

  * Enable Streaming: Generate code to support the dynamic streaming
    API (--stream option)

  * Slice Include Path: The list of directories to search for included
    Slice files (-I option)
    
  * Preprocessor Macros: List of macro definitions passed to the Slice compiler
  
  * Ice Components: The list of Ice libraries to link with
  


Adding Slice files to a project
-------------------------------

You could use "Add -> New Item..." dialog to add slice files to a project once
the extension has been installed.

There will be a new template "Slice source"

To add an existing Slice file to a project, use "Add -> Existing Item..."


Generating code
---------------

The extension compiles a Slice file whenever you save the file. If
you save a file that is included by other Slice files, the including
files are compiled as well.

The generated code is placed in the same directory as the corresponding
Slice file.

For example for Demo.ice, you will see Demo.cpp and Demo.h automatically
added to your C++ project, or Demo.cs to your .NET project. 

Slice Compilation errors are displayed in the Visual Studio
"Output" and "Error List" panels.
