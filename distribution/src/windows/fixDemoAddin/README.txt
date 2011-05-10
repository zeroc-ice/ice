Description
-----------

These Visual Studio Add-In is used to fix our Visual Studio demos.

The add-in check the project settings and ensure they are consistent
across our demos.

The add-in should be used before each release.


Install
-------

Before build ensure that FixDemoConnect.cs version member is set 
to the correct value.

The add-in used that property to build plugin dll names, like
helloplugin34d.dll and similar.

Build VS 2008 & 2010 versions using the corresponding solution files
copy the binaries to Ice bin directory.

copy conf/*.AddIn files to VS add-in directory, typicaly

C:\Users\<username>\Documents\Visual Studio 2010\Addins
C:\Users\<username>\Documents\Visual Studio 2008\Addins


Usage
-----


1) Open VS 2008, open Add-In Manager (Tools > Add-in Manager), 
disable "Ice Visual Studio Add-In", enable 
"Ice Fix Demo Visual Studio Add-In".

2) restart VS 2008, and open cpp/demo/demo-vs2008.sln

# This 2 steps just ensure that new added demos use the same settings
# as other demos, if there isn't new demos we can skip this 2 steps.

3) Delete VS 2010 demo projects, filters and solution, note that in VS 2010
filters are placed in separate files.

cd cpp/demo
find . -iname *.vcxproj* | xargs rm 
rm demo-vs2010.*

4) Copy VS 2008 solution as VS 2010 solution

cp demo-vs2008.sln demo-vs2010.sln

4) Open VS 2010, open Add-In Manager (Tools > Add-in Manager), 
disable both add-ins "Ice Visual Studio Add-In" & 
"Ice Fix Demo Visual Studio Add-In".

5) Restart VS 2010, and open demo-vs2010.sln, and follow the conversion
wizard steps.

6) Go to Add-In Manager and enable "Ice Fix Demo Visual Studio Add-In",
restart VS 2010 and open demo-vs2010.sln, close VS 2010 and save changes
when promtep.

7) Go to Add-In Manager and disable "Ice Fix Demo Visual Studio Add-In",
enable "Ice Visual Studio Add-In", restart Visual Studio and 
open demo-vs2010.sln, this time the add-in will fix the demo configurations 
for VC100, save changes and close visual studio.

8) Commit changes to git, for new demos ensure to add *.vcxproj and 
*.vcxproj.filters, ice.props to git.


What the add-in does?
---------------------

VS 2008

 * Our demo projects were using UpgradeFromVC60.vsprops property sheet
 that isn't necessary, the add-in remove that property sheet if present.

VS 2010

 * The conversion wizard has a bug that produces a configuration named 
 "Template" see:

    http://connect.microsoft.com/VisualStudio/feedback/details/540363/

  The add-in remove the "Template" project configuration.

 * In VS 2010 C++ projects configuration has a new property "Target Name"
 the wizard set that property equals to the project name, that was causing
 VS 2010 warnings and problems to start the program in the 
 debugger, this property should match the linker output filename without the
 extension. The add-in takes care of this.

VS 2008 & 2010 ensure that the following properties has the correct value:

  * General:
   * Build Log File
   * Intermediate Directory
  * Compiler:
    * Object File
    * Assembler Listing Location
    * Program DataBase File Name
    * Precompiled Header File
  * Linker:
   * OutputFile  (.exe | .dll) 
   * Program Database File (.pdb)
   * Manifest File


The add-in needs the project name match the output file, for example client, server,
helloservice, helloplugin

The add-in will generate the corresponding Target Name, Output File 
 Debug/Release: (client client.exe), 
 Debug/Release: (server server.exe), 
 Debug: (helloserviced, helloserviced.dll), Release: (helloservice, helloservice.dll), 
 Debug: (helloplugin34d | helloplugin34d.dll) Release: (helloplugin34 | helloplugin34.dll)

The add-in checks the project "Configuration Type" to decide if the project is an exe or dll,
to guess if a library is a plugin it check if project name ends with plugin, and in that case
uses the version attribute in the name.