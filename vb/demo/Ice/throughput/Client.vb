' **********************************************************************
'
' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports Demo

Module throughputC

    Private Sub menu()
	Console.Out.WriteLine("usage:")
	Console.Out.WriteLine("s: send byte sequence")
	Console.Out.WriteLine("o: send byte sequence as oneway")
	Console.Out.WriteLine("r: receive byte sequence")
	Console.Out.WriteLine("e: echo (send and receive) byte sequence")
	Console.Out.WriteLine("x: exit")
	Console.Out.WriteLine("?: help")
    End Sub

    Private Function run(ByVal args() As String, ByVal communicator As Ice.Communicator) As Integer
	Dim properties As Ice.Properties = communicator.getProperties()
	Dim refProperty As String = "Throughput.Throughput"
	Dim r As String = properties.getProperty(refProperty)
	If r.Length = 0 Then
	    Console.Error.WriteLine("property `" & r & "' not set")
	    Return 1
	End If

	Dim b As Ice.ObjectPrx = communicator.stringToProxy(r)
	Dim throughput As ThroughputPrx = ThroughputPrxHelper.checkedCast(b)
	If throughput Is Nothing Then
	    Console.Error.WriteLine("invalid proxy")
	    Return 1
	End If
	Dim throughputOneway As ThroughputPrx = ThroughputPrxHelper.uncheckedCast(throughput.ice_oneway())

	Dim seq() As Byte = New Byte(seqSize.value) {}

	menu()

	Dim line As String = Nothing
	Do
	    Try
		Console.Out.Write("==> ")
		Console.Out.Flush()
		line = Console.In.ReadLine()
		If line Is Nothing Then
		    Exit Try
		End If

		'  Initial ping to setup the connection.
		throughput.ice_ping()

		Dim tmsec As Long = System.DateTime.Now.Ticks / 10000

		Dim repetitions As Integer = 100

		If line.Equals("s") Or line.Equals("o") Or line.Equals("r") Or line.Equals("e") Then
		    Dim c As Char = line.Chars(0)

		    Select Case c           
			Case "s"
			Case "o"
			    Console.Out.Write("sending")                                           
			Case "r"
			    Console.Out.Write("receiving")
			Case "e"
			    Console.Out.Write("sending and receiving")
		    End Select

		    Console.Out.Write(" " & repetitions & " sequences of size " & seqSize.value)

		    If c = "o" Then
			Console.Out.Write(" as oneway")
		    End If

		    Console.Out.WriteLine("...")

		    For i As Integer = 0 To repetitions - 1
			Select Case c
			    Case "s"
				throughput.sendByteSeq(seq)
			    Case "o"
				throughputOneway.sendByteSeq(seq)
			    Case "r"
				throughput.recvByteSeq()
			    Case "e"
				throughput.echoByteSeq(seq)
			End Select
		    Next

		    Dim dmsec As Double = System.DateTime.Now.Ticks / 10000 - tmsec
		    Console.Out.WriteLine("time for " & repetitions & " sequences: " & dmsec.ToString("F") & "ms")
		    Console.Out.WriteLine("time per sequence: " & CType(dmsec / repetitions, Double).ToString("F") & "ms")
		    Dim mbit As Double = repetitions * seqSize.value * 8.0 / dmsec / 1000.0
		    If c = "e" Then
			mbit *= 2
		    End If
		    Console.Out.WriteLine("throughput: " & mbit.ToString("F") & "MBit/s")
		ElseIf line.Equals("x") Then
		    '  Nothing to do
		ElseIf line.Equals("?") Then
		    menu()
		Else
		    Console.Out.WriteLine("unknown command `" & line & "'")
		    menu()
		End If
	    Catch ex As System.Exception
		Console.Error.WriteLine(ex)
	    End Try
	Loop While Not line.Equals("x")

	Return 0
    End Function

    Public Sub Main(ByVal args() As String)
	Dim status As Integer = 0
	Dim communicator As Ice.Communicator = Nothing

	Try
	    Dim properties As Ice.Properties = Ice.Util.createProperties()
	    properties.load("config")
	    communicator = Ice.Util.initializeWithProperties(args, properties)
	    status = run(args, communicator)
	Catch ex As System.Exception
	    Console.Error.WriteLine(ex)
	    status = 1
	End Try

	If Not communicator Is Nothing Then
	    Try
		communicator.destroy()
	    Catch ex As System.Exception
		Console.Error.WriteLine(ex)
		status = 1
	    End Try
	End If

	System.Environment.Exit(status)
    End Sub

End Module
