
Dim line, i, fs, baseName, outPath, outFile, depends, errors, shell
Set depends = CreateObject("Scripting.Dictionary")

errors = False

Set fs = CreateObject("Scripting.FileSystemObject")

Set shell = WScript.CreateObject("WScript.Shell")

baseName = WScript.Arguments(0)

If WScript.Arguments.Count > 1 Then
    prefix = WScript.Arguments(1)
Else
    prefix = ""
End If

If InStr(1, shell.ExpandEnvironmentStrings("%WINRT%"), "yes") Then
    baseName = Replace(baseName, "..\", "")
End If

outPath = ".depend\" & Replace(baseName, ".cpp", ".d.mak")

Set outFile = fs.CreateTextFile(outPath, True)

outFile.WriteLine(prefix & Replace(baseName, ".cpp", ".obj") & ": \")

outFile.WriteLine("	" & WScript.Arguments(0) & " \")


Do While Not WScript.StdIn.AtEndOfStream
    line = WScript.StdIn.ReadLine()
    i = InStr(1, line, "Note: including file:")
    If i > 0 Then
        i = i + Len("Note: including file:")
        line = Mid(line, i)
        line = Trim(line)

        line = "	""" & line & """ \"
        If Not depends.Exists(line) Then
            depends.Add line, "" 
            outFile.WriteLine(line)
        End If

    Else
        If InStr(1, line, "error") Then
            errors = True
        End If
        WScript.Echo line
    End If
Loop

If errors Then
    WScript.Quit 1
End If
