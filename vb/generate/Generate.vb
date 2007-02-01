' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

'
' Simple poor-man's make replacement to automatically generate VB from Slice definitions.
'
' Usage: generate.exe dir [build|rebuild|clean]
'
' For example:
'
'    generate.exe . build
'
' locates all Slice files in the sub-tree rooted at the current directory and compiles them.
'
' When a Slice file is found in a directory, generate.exe checks whether the directory containing
' the Slice file contains a directory called "generated". If so, it places the generated .vb file
' into the "generated" directory; otherwise, the generated .vb file is placed into the directory
' that contains the Slice file.
'

Imports System
Imports System.Diagnostics
Imports System.IO
Imports System.Text
Imports System.Threading
Imports Microsoft.VisualBasic

Module Generate
    Private proc As Process
    Private progName As String

#If __MonoBASIC__ Then

    Private Function ShortPath(ByVal longPath As String) As String
        Return longpath
    End Function

    Private Function ShortExePath(ByVal longPath As String) As String
        Return longPath
    End Function

#Else
    Declare Auto Function GetShortPathName Lib "kernel32.dll" (ByVal longPath As String, _
        ByVal shortPath As StringBuilder, ByVal size As Long) As Long

    Private Function ShortPath(ByVal longPath as String) As String
        Dim sp As StringBuilder = New StringBuilder
        sp.Length = 1024    ' TODO: This should be replaced with MAX_PATH for Win32
        Dim result As Long = GetShortPathName(longPath, sp, sp.Capacity)
        If result = 0 Or result > sp.capacity Then
            ' Can occur if short pathnames are not supported on the specified filesystem
            Return longPath
        End If
        Return sp.ToString
    End Function

    Private Function ShortExePath(ByVal longPath As String) As String
        If Not longPath.EndsWith(".exe") Then
            longPath = longPath + ".exe"
        End If
        return ShortPath(longPath)
    End Function
#End If

    Private Sub RedirectStandardOutput()
        Dim output As String = proc.StandardOutput.ReadToEnd()
        Console.Out.Write(output)
        Console.Out.Flush()
    End Sub

    Private Sub RedirectStandardError()
        Dim output As String = proc.StandardError.ReadToEnd()
        Console.Error.Write(output)
        Console.Error.Flush()
    End Sub

    Private Function handlePathSpaces(ByVal dir As String) As String
        If dir.IndexOf(" ") > -1 Then
            Return """" + dir + """"
        End If
        Return dir
    End Function

    Enum BuildAction As Integer
        build
        rebuild
        clean
    End Enum

    Class Processor

        Private _slice2vb As String
        Private _includes As String
        Private _action As BuildAction

        Public Sub New(ByVal slice2vb As String, ByVal includes As String, ByVal action As BuildAction)
            _slice2vb = slice2vb
            _includes = includes
            _action = action
        End Sub

        '
        ' Compile a Slice file, redirecting standard output and standard error to the console.
        '
        Private Function doCompile(ByVal sliceFile As String, ByVal outputDir As String) As Integer
            '
            ' Terrible hack: because we don't have a pre-build event in the Visual Basic IDE,
            ' we need some other way to get additional options into slice2vb: for a Slice file
            ' "foo.ice", we look for files named ".foo.<option>". For example, if the directory
            ' containing the Slice file "foo.ice" also contains a file called ".foo.stream", we
            ' add the "--stream" option to the command line.
            '
            Dim dir As String = Path.GetDirectoryName(sliceFile)
            Dim stem As String = Path.GetFileNameWithoutExtension(sliceFile)
            Dim optionFiles() As String = Directory.GetFiles(dir, "." & stem & ".*")
            Dim sb As StringBuilder = New StringBuilder
            For Each f As String In optionFiles
                sb.Append(" --" & Path.GetExtension(f).Substring(1))
            Next

            Dim cmdArgs As String = "--output-dir " & outputDir & " --ice " & _includes & sb.ToString() & " " & handlePathSpaces(sliceFile)
            Dim info As ProcessStartInfo = New ProcessStartInfo(_slice2vb, cmdArgs)
            info.CreateNoWindow = True
            info.UseShellExecute = False
            info.RedirectStandardOutput = True
            info.RedirectStandardError = True
            proc = Process.Start(info)
            If proc Is Nothing Then
                Console.Error.WriteLine(progName + ": cannot start `" & _slice2vb & " " & cmdArgs & "'")
                Environment.Exit(1)
            End If
            Dim t1 As Thread = New Thread(New ThreadStart(AddressOf RedirectStandardOutput))
            Dim t2 As Thread = New Thread(New ThreadStart(AddressOf RedirectStandardError))
            t1.Start()
            t2.Start()
            proc.WaitForExit()
            Dim rc As Integer = proc.ExitCode
            proc.Close()
            t1.Join()
            t2.Join()
            Return rc
        End Function

        '
        ' Do a recursive decent on the directory hierarchy rooted at "currentDir" and look
        ' for Slice files. Apply the build action to each Slice file found.
        '
        Public Function processDirectory(ByVal currentDir As String) As Integer

            '
            ' Set output directory.
            '
            Const generatedDir As String = "generated"
            Dim outputDir As String = currentDir
            If Directory.Exists(Path.Combine(currentDir, generatedDir)) Then
                outputDir = Path.Combine(currentDir, generatedDir)
            End If

            '
            ' Look for Slice files and apply the build action to each Slice file.
            '
            Dim rc As Integer = 0
            Const slicePat As String = "*.ice"
            Dim sliceFiles() As String = Directory.GetFiles(currentDir, slicePat)
            For Each sliceFile As String In sliceFiles
                If sliceFile.EndsWith(".ice") Then
                    Dim vbFile As String = Path.GetFileName(Path.ChangeExtension(sliceFile, ".vb"))
                    vbFile = Path.Combine(outputDir, vbFile)
                    Select Case _action
                        Case BuildAction.build
                            Dim sliceTime As DateTime = File.GetLastWriteTime(sliceFile)
                            Dim needCompile As Boolean = Not File.Exists(vbFile)
                            If Not needCompile Then
                                needCompile = sliceTime > File.GetLastWriteTime(vbFile)
                            End If
                            If needCompile Then
                                Console.WriteLine(Path.GetFileName(sliceFile))
                                Dim exitCode As Integer = doCompile(sliceFile, outputDir)
                                If rc = 0 Then
                                    rc = exitCode
                                End If
                            End If
                        Case BuildAction.rebuild
                            Console.WriteLine(Path.GetFileName(sliceFile))
                            Dim exitCode As Integer = doCompile(sliceFile, outputDir)
                            If rc = 0 Then
                                rc = exitCode
                            End If
                        Case BuildAction.clean
                            If File.Exists(vbFile) Then
                                File.Delete(vbFile)
                                Console.WriteLine(vbFile & ": deleted")
                            End If
                    End Select
                End If
            Next

            '
            ' Recurse into subdirectories.
            '
            Dim dirs() As String = Directory.GetDirectories(currentDir)
            For Each dir As String In dirs
                If Not dir.Equals("generate") Then
                    Dim exitCode As Integer = processDirectory(dir)
                    If rc = 0 Then
                        rc = exitCode
                    End If
                End If
            Next

            Return rc
        End Function

    End Class

    Private Sub usage()
        Console.Error.WriteLine("usage: {0} solution_dir build|rebuild|clean", progName)
        Environment.Exit(1)
    End Sub

    Public Sub Main(ByVal args() As String)

        Try

            '
            ' Check arguments.
            '
            progName = AppDomain.CurrentDomain.FriendlyName
            If args.Length <> 2 Then
                usage()
            End If

            Const slice2vbName As String = "slice2vb"
            Dim solDir As String = args(0)
            If solDir.EndsWith("\.") And solDir.Length > 2 Then
                solDir = Left(solDir, solDir.Length - 2)
            End If
            Dim action As BuildAction
            If args(1).Equals("build") Then
                action = BuildAction.build
            ElseIf args(1).Equals("rebuild") Then
                action = BuildAction.rebuild
            ElseIf args(1).Equals("clean") Then
                action = BuildAction.clean
            Else
                usage()
            End If

            '
            ' Work out where slice2vb is. If neither in $(SolutionDir) nor in %ICE_HOME%\bin,
            ' assume that slice2vb is in PATH.
            '
            Dim iceHome As String = Environment.GetEnvironmentVariable("ICE_HOME")
            If iceHome Is Nothing Then
                iceHome = Path.Combine(Path.Combine(solDir, ".."), "ice")
                If Not Directory.Exists(iceHome) Then
                    iceHome = Path.Combine(Path.Combine(Path.Combine(solDir, ".."), ".."), "ice")
                    If Not Directory.Exists(iceHome) Then
                        iceHome = Path.Combine(solDir, "..")
                    End If
                End If
            End If

            iceHome = ShortPath(iceHome)

            Dim slice2vb As String = Path.Combine(Path.Combine(solDir, "bin"), slice2vbName)
            If Not File.Exists(slice2vb) And Not File.Exists(slice2vb & ".exe") Then
                If Not iceHome Is Nothing Then
                    slice2vb = Path.Combine(Path.Combine(iceHome, "bin"), slice2vbName)
                    If Not File.Exists(slice2vb) And Not File.Exists(slice2vb & ".exe") Then
                        slice2vb = slice2vbName
                    End If
                Else
                    slice2vb = slice2vbName
                End If
            End If

            slice2vb = ShortExePath(slice2vb)

            Dim includes As String = ""
            If Directory.Exists(Path.Combine(solDir, "slice")) Then
                includes = handlePathSpaces("-I" & Path.Combine(solDir, "slice"))
            End If
            If Directory.Exists(Path.Combine(iceHome, "slice")) Then
                includes = includes & " " & "-I" & Path.Combine(iceHome, "slice")
            End If

            '
            ' Change to the solution directory and recursively look for Slice files.
            '
            Directory.SetCurrentDirectory(solDir)
            Dim proc As Processor = New Processor(slice2vb, includes, action)
            Environment.Exit(proc.processDirectory("."))

        Catch ex As System.ComponentModel.Win32Exception
            If ex.NativeErrorCode = 2 Then
                Console.Error.WriteLine("Cannot find slice2vb.exe: set ICE_HOME or add the slice2vb.exe directory to the list of 'Executable files' directories in the Visual Studio options.")
                System.Environment.Exit(1)
            End If
            Console.Error.WriteLine(ex)
            System.Environment.Exit(1)
        Catch ex As Exception
            Console.Error.WriteLine(ex)
            System.Environment.Exit(1)
        End Try
    End Sub

End Module
