Const ForReading = 1, ForWriting = 2, ForAppending =8

Dim fso
Dim installDir
Dim vs2013File

Dim files
Dim file
Dim fileName
Dim tmpFileName
Dim tmpFile
Dim nextLine
Dim paths

paths = Split(Session.Property("CustomActionData"), "|")

installDir = paths(0)
vs2013File = paths(1) & "Microsoft\VisualStudio\12.0\Addins\Ice-VS2013.AddIn"

files = Array(vs2013File)

For Each fileName in files
    tmpFileName = fileName & ".tmp"

    Set fso = CreateObject("Scripting.FileSystemObject")
    Set file = fso.OpenTextFile(fileName, ForReading, True)
    Set tmpFile = fso.OpenTextFile(tmpFileName, ForWriting, True)

    Do Until file.AtEndOfStream
       nextLine = file.ReadLine
       tmpFile.WriteLine Replace(nextLine, "@installdir@\", installDir)
    Loop

    file.Close
    tmpFile.Close

    fso.DeleteFile fileName
    fso.MoveFile tmpFileName, fileName
Next
