The Internet Communications Engine
----------------------------------

Ice, the Internet Communications Engine, is middleware for the
practical programmer. A high-performance Internet communications
platform, Ice includes a wealth of layered services and plug-ins.
Ice means simplicity, speed, and power.

Ice is available under the terms of the GNU General Public License
(GPL) (see LICENSE file). Commercial licenses are available for
customers who wish to use Ice in proprietary products. Please contact
sales@zeroc.com for more information on licensing Ice.

This file describes how to setup Visual Studio for Ice, and provides
instructions for building and running the sample programs.

See doc/README.html for information on the documentation included with
this distribution.


Setting up Visual Studio 6.0
----------------------------

Before you can use Ice in your C++ applications, you first need to
configure Visual Studio with the locations of the Ice header files,
libraries, and executables.

- In the IDE, choose Tools->Options->Directories

- Select "Include files"

- Add C:\Ice-<version>\include and C:\Ice-<version>\include\stlport.
  Move the stlport include directory to the top of the list, or at
  least before the Visual C++ include directories. This is necessary
  so that the compiler uses the STL headers from the STLport library
  instead of the Visual C++ STL library.

- Select "Library files"

- Add C:\Ice-<version>\lib

- Select "Executable files"

- Add C:\Ice-<version>\bin


Building and running the C++ demos
----------------------------------

The C++ demos are in the demo directory.

To build the C++ demos, start Visual Studio and open the workspace
demo\demo.dsw. Set your active project to "all" using
Project->Set Active Project, then start the compilation using
Build->Build.

To run these demos, you need to add the Ice bin directory to your
PATH, as shown below:

set ICE_HOME=<Ice installation root directory>
set PATH=%ICE_HOME%\bin;%PATH%

Next, open a Command Prompt, change to the desired demo subdirectory,
and type 'server' to start the server. In a separate Command Prompt
window, type 'client' to start the client.

Some demo directories contain README files if additional requirements
are necessary.

Note that extra steps may be required when running the IceBox demo,
depending on how you compiled the demos. If you used the Release
configuration, then no extra steps are necessary. However, if you
used the Debug configuration, you will need to use the debug
version of icebox.exe located in the bin\debug subdirectory.
