Const ForReading = 1, ForWriting = 2, ForAppending =8

Dim documentsDir
Dim vs2008File
Dim vs2010File
Dim fso

documentsDir = Session.Property("CustomActionData")

vs2008File = documentsDir & "Visual Studio 2008\Addins\Ice-VS2008.AddIn"
vs2010File = documentsDir & "Visual Studio 2010\Addins\Ice-VS2010.AddIn"

Set fso = CreateObject("Scripting.FileSystemObject")

fso.DeleteFile vs2008File
fso.DeleteFile vs2010File
