' **********************************************************************
'
' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Module HelloS

    Private Function run(ByVal args() As String, ByVal communicator As Ice.Communicator) As Integer
	Dim adapter As Ice.ObjectAdapter = communicator.createObjectAdapter("Hello")
	Dim obj As Ice.Object = New HelloI
	adapter.add(obj, Ice.Util.stringToIdentity("hello"))
	adapter.activate()
	communicator.waitForShutdown()
	Return 0
    End Function

    Sub Main(ByVal args() As String)
	Dim status As Integer = 0
	Dim communicator As Ice.Communicator = Nothing

	Try
	    Dim properties As Ice.Properties = Ice.Util.createProperties()
	    properties.load("config")
	    communicator = Ice.Util.initializeWithProperties(args, properties)
	    status = run(args, communicator)
	Catch ex As System.Exception
	    System.Console.Error.WriteLine(ex)
	    status = 1
	End Try

	If Not communicator Is Nothing Then
	    Try
		communicator.destroy()
	    Catch ex As System.Exception
		System.Console.Error.WriteLine(ex)
		status = 1
	    End Try
	End If

	System.Environment.Exit(status)
    End Sub

End Module
