
Dim fs, targets

Set depends = CreateObject("Scripting.Dictionary")

Set fs = CreateObject("Scripting.FileSystemObject")

targets = ""

If WScript.Arguments.Count > 0 Then
	targets = WScript.Arguments(0)
End If

Do While Not WScript.StdIn.AtEndOfStream
	line = WScript.StdIn.ReadLine()
	If InStr(1, line, ".cpp") > 0 And targets <> "" Then
		line = targets
	End If
    WScript.Echo line
Loop