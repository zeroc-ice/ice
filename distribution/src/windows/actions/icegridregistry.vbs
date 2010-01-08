Const ForReading = 1, ForWriting = 2, ForAppending =8

Dim fso, file, tmpFile
Dim tmpFileName, nextLine, customData

customData = Session.Property("CustomActionData")
tokens = Split(customData, "|")
tmpFileName = tokens(0) & ".tmp"

Set fso = CreateObject("Scripting.FileSystemObject")
Set file = fso.OpenTextFile(tokens(0), ForReading, True)
Set tmpFile = fso.OpenTextFile(tmpFileName, ForWriting, True)

Do Until file.AtEndOfStream
   nextLine = file.ReadLine
   tmpFile.WriteLine Replace(nextLine, "@installdir@", tokens(1))
Loop

file.Close
tmpFile.Close

fso.DeleteFile tokens(0)
fso.MoveFile tmpFileName, tokens(0)
