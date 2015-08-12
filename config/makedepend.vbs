
Const ForReading = 1, ForWriting = 2, ForAppending =8

Dim line, i, fs, baseName, outFile, depends, errors, shell, topSrcDir, cppSrcDir, cppIncludeDir, cppTestIncludeDir, workDir, objPrefix

Set depends = CreateObject("Scripting.Dictionary")

errors = False

Set fs = CreateObject("Scripting.FileSystemObject")

Set shell = WScript.CreateObject("WScript.Shell")

baseName = WScript.Arguments(0)


topSrcDir = fs.GetAbsolutePathName(WScript.Arguments(1) & "\..")
cppThirdPartyDir = fs.GetAbsolutePathName(WScript.Arguments(1) & "\third-party-packages\")
cppSrcDir = fs.GetAbsolutePathName(WScript.Arguments(1) & "\src\")
cppIncludeDir = fs.GetAbsolutePathName(WScript.Arguments(1) & "\include\")
cppTestIncludeDir = fs.GetAbsolutePathName(WScript.Arguments(1) & "\test\include\")
workDir = fs.GetAbsolutePathName(".")

iceCppIncludeDir = fs.GetAbsolutePathName(WScript.Arguments(1) & "\..\cpp\include\")

objPrefix = ""

If InStr(1, shell.ExpandEnvironmentStrings("%WINRT%"), "yes") Then
    baseName = Replace(baseName, "..\..\IceDiscovery\", "IceDiscovery\")
    baseName = Replace(baseName, "..\..\IceLocatorDiscovery\", "IceLocatorDiscovery\")
    baseName = Replace(baseName, "..\", "")
    objPrefix = "$(ARCH)\$(CONFIG)\"
End If

Set outFile = fs.OpenTextFile(".depend.mak", ForAppending, True)

outFile.WriteLine("")

outFile.WriteLine(objPrefix & Replace(baseName, ".cpp", ".obj") & ": \")
outFile.WriteLine("	" & WScript.Arguments(0) & " \")

Set stream = fs.OpenTextFile(Replace(basename, ".cpp", ".d"), ForReading)

Do While Not stream.AtEndOfStream
    Do
        line = stream.ReadLine()

        i = InStr(1, line, "Note: including file:")
        If i > 0 Then
            i = i + Len("Note: including file:")
            line = Mid(line, i)
            line = Trim(line)

            line = fs.GetAbsolutePathName(line)
            
            If InStr(1, line, cppThirdPartyDir) Then Exit Do

            If InStr(1, line, topSrcDir) Then
                If InStr(1, line, workDir & "\") > 0 Then
                    line = Right(line, len(line) - len(workDir) -1)
                Elseif InStr(1, line, cppSrcDir) > 0 Then
                    line = WScript.Arguments(1) & "\src" & Right(line, len(line) - len(cppSrcDir))
                Elseif InStr(1, line, cppIncludeDir) > 0 Then
                    line = "$(includedir)" & Right(line, len(line) - len(cppIncludeDir))
                Elseif InStr(1, line, iceCppIncludeDir) > 0 Then
                    line = "$(ice_cpp_dir)\include" & Right(line, len(line) - len(iceCppIncludeDir))
                Elseif InStr(1, line, cppTestIncludeDir) > 0 Then
                    line = "$(top_srcdir)\test\include" & Right(line, len(line) - len(cppTestIncludeDir))
                End If

                line = "    """ & line & """ \"
                If Not depends.Exists(line) Then
                    depends.Add line, ""
                    outFile.WriteLine(line)
                End If
            End If
        End If
    Loop While False
Loop

stream.Close()

If errors Then
    WScript.Quit 1
End If
