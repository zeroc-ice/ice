Option Explicit
Dim paths, phpIni, oldIni, newIni, includeDir, fs, line

Const ForReading = 1, ForWriting  = 2

phpIni = Session.Property("CustomActionData")

Set fs = CreateObject("Scripting.FileSystemObject")

If fs.FileExists(phpIni) Then

    Set oldIni = fs.OpenTextFile(phpIni, ForReading , True)
    Set newIni = fs.OpenTextFile(phpIni & ".tmp", ForWriting , True)

    Do Until oldIni.AtEndOfStream
        line = oldIni.ReadLine
        If InStr(line, "PHP_ZEROC_ICE") = 0 And _
           InStr(line, "php_ice_nts.dll") = 0 And _
           InStr(line, "shared\php\ice") = 0 Then
            newIni.WriteLine line
        End If
    Loop

    oldIni.Close
    newIni.Close

    fs.DeleteFile phpIni, True
    fs.MoveFile phpIni & ".tmp", phpIni
End If

If fs.FileExists(phpIni & ".ice.back") Then
    fs.DeleteFile phpIni & ".ice.back"
End If
