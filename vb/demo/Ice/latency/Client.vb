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

Module Client

    Private Function run(ByVal args() As String, ByVal communicator As Ice.Communicator) As Integer
	Dim properties As Ice.Properties = communicator.getProperties()
	Dim refProperty As String = "Latency.Ping"
	Dim ref As String = properties.getProperty(refProperty)
	If ref.Length = 0 Then
	    Console.Error.WriteLine("property `" & refProperty & "' not set")
	    Return 1
	End If

	Dim base As Ice.ObjectPrx = communicator.stringToProxy(ref)
	Dim ping As PingPrx = PingPrxHelper.checkedCast(base)
	If ping Is Nothing Then
	    Console.Error.WriteLine("invalid proxy")
	    Return 1
	End If

	'  Initial ping to setup the connection.
	ping.ice_ping()

	Dim tv1 As Long = (System.DateTime.Now.Ticks - 621355968000000000) / 10000
	Dim repetitions As Integer = 100000
	Console.Out.WriteLine("pinging server " & repetitions & " times (this may take a while)")
	For i As Integer = 0 To repetitions - 1
	    ping.ice_ping()
	Next

	Dim tv2 As Long = (System.DateTime.Now.Ticks - 621355968000000000) / 10000
	Dim total As Double = tv2 - tv1
	Dim perPing As Double = total / repetitions

	Console.Out.WriteLine("time for " & repetitions & " pings: " & total & "ms")
	Console.Out.WriteLine("time per ping: " & perPing & "ms")

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
