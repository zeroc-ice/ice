Const ForReading = 1, ForWriting = 2, ForAppending =8

Dim documentsDir
Dim vs2008File
Dim vs2010File
Dim fso

documentsDir = Session.Property("CustomActionData")

vs2008File = documentsDir & "Visual Studio 2008\Addins\Ice-VS2008.AddIn"
vs2010File = documentsDir & "Visual Studio 2010\Addins\Ice-VS2010.AddIn"

Set fso = CreateObject("Scripting.FileSystemObject")

If fso.FileExists(vs2008File) Then
    fso.DeleteFile vs2008File
End If

If fso.FileExists(vs2010File) Then
    fso.DeleteFile vs2010File
End If

Const HKEY_CURRENT_USER = &H80000001
strComputer = "."

Set registryObj = GetObject("winmgmts:{impersonationLevel=impersonate}!\\" & _
 strComputer & "\root\default:StdRegProv")


vs2008Key = "Software\Microsoft\VisualStudio\9.0\PreloadAddinStateManaged"
registryObj.EnumValues HKEY_CURRENT_USER, vs2008Key, arrSubKeys
If Not TypeName(arrSubKeys) = "Null" Then
    For Each Subkey in arrSubKeys
        If StrComp(Left(Subkey, Len("Ice.VisualStudio.Connect")), "Ice.VisualStudio.Connect", vbTextCompare) = 0 Then
            registryObj.DeleteValue HKEY_CURRENT_USER, vs2008key, Subkey
        End If
    Next
End If

vs2010Key = "Software\Microsoft\VisualStudio\10.0\PreloadAddinStateManaged"
registryObj.EnumValues HKEY_CURRENT_USER, vs2010Key, arrSubKeys
If Not TypeName(arrSubKeys) = "Null" Then
    For Each Subkey in arrSubKeys
        If StrComp(Left(Subkey, Len("Ice.VisualStudio.Connect")), "Ice.VisualStudio.Connect", vbTextCompare) = 0 Then
            registryObj.DeleteValue HKEY_CURRENT_USER, vs2010key, Subkey
        End If
    Next
End If