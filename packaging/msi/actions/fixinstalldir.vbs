Const ForReading = 1, ForWriting = 2, ForAppending =8

Dim fso
Dim installDir
Dim registryFile
Dim file
Dim tmpFileName
Dim tmpFile
Dim nextLine

installDir = Session.Property("CustomActionData")

registryFile = installDir & "config\icegridregistry.cfg"

Set fso = CreateObject("Scripting.FileSystemObject")

If fso.FileExists(registryFile) Then
    tmpFileName = registryFile & ".tmp"

    Set file = fso.OpenTextFile(registryFile, ForReading, True)
    Set tmpFile = fso.OpenTextFile(tmpFileName, ForWriting, True)

    Do Until file.AtEndOfStream
       nextLine = file.ReadLine
       tmpFile.WriteLine Replace(nextLine, "@installdir@\", installDir)
    Loop

    file.Close
    tmpFile.Close

    fso.DeleteFile registryFile
    fso.MoveFile tmpFileName, registryFile
End If
