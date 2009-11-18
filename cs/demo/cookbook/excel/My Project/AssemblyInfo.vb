Imports System
Imports System.Reflection
Imports System.Runtime.InteropServices
Imports System.Security
Imports Microsoft.Office.Tools.Excel

' General Information about an assembly is controlled through the following 
' set of attributes. Change these attribute values to modify the information
' associated with an assembly.

' Review the values of the assembly attributes

<Assembly: AssemblyTitle("cookbook.excel")> 
<Assembly: AssemblyDescription("")> 
<Assembly: AssemblyCompany("ZeroC Inc.")> 
<Assembly: AssemblyProduct("cookbook.excel")> 
<Assembly: AssemblyCopyright("Copyright © ZeroC, Inc. 2009")> 
<Assembly: AssemblyTrademark("")> 

' Setting ComVisible to false makes the types in this assembly not visible 
' to COM components.  If you need to access a type in this assembly from 
' COM, set the ComVisible attribute to true on that type.
<Assembly: ComVisible(False)>

'The following GUID is for the ID of the typelib if this project is exposed to COM
<Assembly: Guid("d2904f19-2b25-4a7b-a7a4-567fd7d503d6")> 

' Version information for an assembly consists of the following four values:
'
'      Major Version
'      Minor Version 
'      Build Number
'      Revision
'
' You can specify all the values or you can default the Build and Revision Numbers 
' by using the '*' as shown below:
' <Assembly: AssemblyVersion("1.0.*")> 

<Assembly: AssemblyVersion("1.0.0.0")> 
<Assembly: AssemblyFileVersion("1.0.0.0")> 

' The ExcelLocale1033 attribute controls the locale that is passed to the Excel
' object model. Setting ExcelLocale1033 to true causes the Excel object model to 
' act the same in all locales, which matches the behavior of Visual Basic for 
' Applications. Setting ExcelLocale1033 to false causes the Excel object model to
' act differently when users have different locale settings, which matches the 
' behavior of Visual Studio Tools for Office, Version 2003. This can cause unexpected 
' results in locale-sensitive information such as formula names and date formats.

<Assembly: ExcelLocale1033(True)>

<Assembly: SecurityTransparent()>
