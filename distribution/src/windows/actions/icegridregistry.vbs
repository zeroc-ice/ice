Const ForReading = 1, ForWriting = 2, ForAppending =8

Dim fso
Dim installDir
Dim fileName
Dim file
Dim tmpFileName
Dim tmpFile
Dim nextLine

installDir = Session.Property("CustomActionData")
fileName = installDir & "config\icegridregistry.cfg"
tmpFileName = configFile & ".tmp"

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
