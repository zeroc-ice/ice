Unlike Visual C++ and C#, Visual Basic .NET does not support a
pre-build step as part of a project file. In order to make sure that
Slice definition files are recompiled as necessary, this makefile
project is used. It invokes generatevb.exe with a "build", "rebuild",
or "clean" argument. In turn, generatevb.exe recurses through the
directory hierarchy and looks for Slice files that need rebuilding.

You can run generatevb.exe by hand. For example

    generatevb.exe ..\someDir build

will locate all Slice files underneath ..\someDir and recompile them.

To recompile a Slice file, generatevb.exe first looks to see if the
directory containing the .ice file contains a directory named
"generated".  If so, the generated .vb file is placed into that
directory; if not, the generated .vb file is placed into the directory
that contains the .ice file.
