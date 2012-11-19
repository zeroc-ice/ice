Const ForReading = 1, ForWriting = 2, ForAppending =8

Dim fso
Dim installDir
Dim file
Dim fileName
Dim tmpFileName
Dim tmpFile
Dim nextLine
Dim paths

Set argv = WScript.Arguments

installDir = argv(0)
fileName = argv(1)

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
