
Dim line, i, depends, winrt, shell
Set depends = CreateObject("Scripting.Dictionary")

Set shell = WScript.CreateObject("WScript.Shell")
If InStr(1, shell.ExpandEnvironmentStrings("%WINRT%"), "yes") Then
	winrt = True
Else
	winrt = False
End If

For i = 0 to WScript.Arguments.Count - 1
    line = WScript.Arguments(i)

    If winrt Then
        line = Replace(line, "x86\", "")
        line = Replace(line, "x64\", "")
        line = Replace(line, "arm\", "")
        line = Replace(line, "Debug\", "")
        line = Replace(line, "Retail\", "")
    End If

    If Not depends.Exists(line) Then
        depends.Add line, ""
        WScript.Echo "!if exist(.depend\" & line & ")"
        WScript.Echo "!include .depend\" & line
        WScript.Echo "!endif"
        WScript.Echo ""
    End If
Next
