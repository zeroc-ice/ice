
Const ForReading = 1, ForWriting = 2, ForAppending =8

Dim line, fs, baseName, outFile, depends, shell, srcPrefix

Set depends = CreateObject("Scripting.Dictionary")

Set fs = CreateObject("Scripting.FileSystemObject")

Set shell = WScript.CreateObject("WScript.Shell")

baseName = WScript.Arguments(0)

If InStr(1, shell.ExpandEnvironmentStrings("%WINRT%"), "yes") Then
    baseName = Replace(baseName, "..\", "")
End If

Set outFile = fs.OpenTextFile(".depend.mak", ForAppending, True)

outFile.WriteLine("")

If WScript.Arguments.Count > 1 Then
    srcPrefix = WScript.Arguments(1)
Else
    srcPrefix = ""
End If

Do While Not WScript.StdIn.AtEndOfStream
    line = WScript.StdIn.ReadLine()
    line = Trim(line)
    line = Replace(line, "..\..\..\..\..\slice", "$(slicedir)")
    line = Replace(line, "..\..\..\..\slice", "$(slicedir)")
    line = Replace(line, "..\..\..\slice", "$(slicedir)")
    line = Replace(line, "..\..\slice", "$(slicedir)")

    If len(line) > 0 Then
        If InStr(len(line) - 2, line, ": \") Then
            outFile.WriteLine(srcPrefix & line)
        Else
            If InStr(1, line, "$(slicedir)") Then
                line = """" & Replace(line, ".ice", ".ice""")
            End If
            line = "    " & line
            If Not depends.Exists(line) Then
                depends.Add line, "" 
                outFile.WriteLine(line)
            End If
        End If
    End If
Loop
