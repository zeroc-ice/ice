'  **********************************************************************
' 
'  Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
' 
'  This copy of Ice is licensed to you under the terms described in the
'  ICE_LICENSE file included in this distribution.
' 
'  **********************************************************************

Imports System
Imports System.Diagnostics
Imports System.IO
Imports System.Text
Imports System.Threading

Module Generate
    Enum BuildAction As Integer
	build
	rebuild
	clean
    End Enum

    Private Sub usage(ByVal progName As String)
	Console.Error.WriteLine("usage: {0} solution_dir build|rebuild|clean", progName)
	Environment.Exit(1)
    End Sub

    Dim p As Process

    Private Sub RedirectStandardOutput()
	Dim output As String = p.StandardOutput.ReadToEnd()
	Console.Out.Write(output)
	Console.Out.Flush()
    End Sub

    Private Sub RedirectStandardError()
	Dim output As String = p.StandardError.ReadToEnd()
	Console.Error.Write(output)
	Console.Error.Flush()
    End Sub

    Class Processor

	Private _slice2vb As String
	Private _action As BuildAction

	Public Sub New(ByVal slice2vb As String, ByVal action As BuildAction)
	    _action = action
	    _slice2vb = slice2vb
	End Sub

	'
	' Compile a Slice file, redirecting standard output and standard error to the console.
	'
	Private Sub doCompile(ByVal sliceFile As String, ByVal outputDir As String)
	    Dim cmdArgs As String = "--output-dir " & outputDir & " " & sliceFile
	    Dim info As ProcessStartInfo = New ProcessStartInfo(_slice2vb, cmdArgs)
	    info.CreateNoWindow = True
	    info.UseShellExecute = False
	    info.RedirectStandardOutput = True
	    info.RedirectStandardError = True
	    p = Process.Start(info)
	    Dim t1 As Thread = New Thread(New ThreadStart(AddressOf RedirectStandardOutput))
	    Dim t2 As Thread = New Thread(New ThreadStart(AddressOf RedirectStandardError))
	    t1.Start()
	    t2.Start()
	    p.WaitForExit()
	    t1.Join()
	    t2.Join()
	End Sub

	'
	' Do a recursive decent on the directory hierarchy rooted at "currentDir" and look
	' for Slice files. Apply the build action to each Slice file found.
	'
	Public Sub processDirectory(ByVal currentDir As String)

	    '
	    ' Set output directory.
	    '
	    Const generatedDir = "generated"
	    Dim outputDir As String = currentDir
	    If Directory.Exists(Path.Combine(currentDir, generatedDir)) Then
		outputDir = Path.Combine(currentDir, generatedDir)
	    End If

	    '
	    ' Look for Slice files and apply the build action to each Slice file.
	    '
	    Const slicePat As String = "*.ice"
	    Dim sliceFiles() As String = Directory.GetFiles(currentDir, slicePat)
	    For Each sliceFile As String In sliceFiles
		Dim vbFile As String = Path.GetFileName(Path.ChangeExtension(sliceFile, ".vb"))
		vbFile = Path.Combine(outputDir, vbFile)
		Select Case _action
		    Case BuildAction.build
			Dim sliceTime As DateTime = File.GetLastWriteTime(sliceFile)
			Dim needCompile = Not File.Exists(vbFile)
			If Not needCompile Then
			    needCompile = sliceTime > File.GetLastWriteTime(vbFile)
			End If
			If needCompile Then
			    Console.WriteLine(Path.GetFileName(sliceFile))
			    doCompile(sliceFile, outputDir)
			End If
		    Case BuildAction.build.rebuild
			Console.WriteLine(Path.GetFileName(sliceFile))
			doCompile(sliceFile, outputDir)
		    Case BuildAction.build.clean
			If File.Exists(vbFile) Then
			    File.Delete(vbFile)
			    Console.WriteLine(vbFile & ": deleted")
			End If
		End Select
	    Next

	    '
	    ' Recurse into subdirectories.
	    '
	    Dim dirs() As String = Directory.GetDirectories(currentDir)
	    For Each dir As String In dirs
		If Not dir.Equals("generate") Then
		    processDirectory(dir)
		End If
	    Next

	End Sub

    End Class


    Public Sub Main(ByVal args() As String)

	'
	' Check arguments.
	'
	Dim progName As String = AppDomain.CurrentDomain.FriendlyName
	If args.Length <> 2 Then
	    usage(progName)
	End If

	Dim solDir As String = args(0)
	Dim action As BuildAction
	If args(1).Equals("build") Then
	    action = BuildAction.build
	ElseIf args(1).Equals("rebuild") Then
	    action = BuildAction.build.rebuild
	ElseIf args(1).Equals("clean") Then
	    action = BuildAction.build.clean
	Else
	    usage(progName)
	End If

	'
	' Work out where slice2vb is. If neither in ${SolutionDir} nor in %ICE_HOME%\bin,
	' assume that slice2vb is in PATH.
	'
	Const slice2vbName As String = "slice2vb"
	Dim iceHome As String = Environment.GetEnvironmentVariable("ICE_HOME")

	Dim slice2vb As String = Path.Combine(Path.Combine(solDir, "bin"), slice2vbName)
	If Not File.Exists(slice2vb) And Not File.Exists(slice2vb & ".exe") Then
	    If Not iceHome Is Nothing Then
		slice2vb = Path.Combine(Path.Combine(iceHome, "bin"), slice2vbName)
		If Not File.Exists(slice2vb) And Not File.Exists(slice2vb & ".exe") Then
		    slice2vb = slice2vbName
		End If
	    End If
	Else
	    slice2vb = slice2vbName
	End If

	'
	' Change to the solution directory and recursively look for Slice files.
	'
	Directory.SetCurrentDirectory(solDir)
	Dim proc As Processor = New Processor(slice2vb, action)
	proc.processDirectory(".")

	Environment.Exit(0)

    End Sub

End Module
