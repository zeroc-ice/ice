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
Imports Demo

Module sessionC

    Private Sub menu()
	Console.Out.WriteLine("usage:")
	Console.Out.WriteLine("t: send greeting as twoway")
	Console.Out.WriteLine("o: send greeting as oneway")
	Console.Out.WriteLine("O: send greeting as batch oneway")
	Console.Out.WriteLine("d: send greeting as datagram")
	Console.Out.WriteLine("D: send greeting as batch datagram")
	Console.Out.WriteLine("f: flush all batch requests")
	Console.Out.WriteLine("T: set a timeout")
	Console.Out.WriteLine("x: exit")
	Console.Out.WriteLine("?: help")
    End Sub

    Private Function run(ByVal args() As String, ByVal communicator As Ice.Communicator) As Integer
	Dim properties As Ice.Properties = communicator.getProperties()
	' 
	'  Do Glacier setup.
	' 
	Dim glacierStarterEndpointsProperty As String = "Glacier.Starter.Endpoints"
	Dim glacierStarterEndpoints As String = properties.getProperty(glacierStarterEndpointsProperty)
	Debug.Assert(glacierStarterEndpoints.Length <> 0)
	Dim starterBase As Ice.ObjectPrx = communicator.stringToProxy("Glacier/starter:" & glacierStarterEndpoints)
	Dim starter As Glacier.StarterPrx = Glacier.StarterPrxHelper.checkedCast(starterBase)
	If starter Is Nothing Then
	    Console.Error.WriteLine("Client: endpoints `" & glacierStarterEndpoints & _
	 "' do not refer to a glacier router starter")
	    Return 1
	End If

	Dim privateKey As Byte()
	Dim publicKey As Byte()
	Dim routerCert As Byte()

	Dim router As Glacier.RouterPrx = Nothing
	Dim id As String
	Dim pw As String

	While True
	    Try
		Console.Out.Write("user id: ")
		Console.Out.Flush()
		id = Console.In.ReadLine()
		Console.Out.Write("password: ")
		Console.Out.Flush()
		pw = Console.In.ReadLine()

		router = starter.startRouter(id, pw, privateKey, publicKey, routerCert)
	    Catch ex As Glacier.PermissionDeniedException
		Console.Error.WriteLine("password is invalid, try again")
	    Catch ex As Exception
		Console.Error.WriteLine(ex)
	    End Try
	    Exit While
	End While

	communicator.setDefaultRouter(router)

	Dim session As Glacier.SessionPrx = Nothing
	Try
	    session = router.createSession()
	Catch ex As Glacier.NoSessionManagerException
	    Console.Error.WriteLine(ex)
	    Return 1
	End Try

	Dim base As HelloSessionPrx = HelloSessionPrxHelper.checkedCast(session)

	Dim twoway As HelloSessionPrx = _
	    HelloSessionPrxHelper.checkedCast(base.ice_twoway().ice_timeout(-1).ice_secure(False))
	If twoway Is Nothing Then
	    Console.Error.WriteLine("invalid object reference")
	    Return 1
	End If
	Dim oneway As HelloSessionPrx = HelloSessionPrxHelper.uncheckedCast(twoway.ice_oneway())
	Dim batchOneway As HelloSessionPrx = HelloSessionPrxHelper.uncheckedCast(twoway.ice_batchOneway())
	Dim datagram As HelloSessionPrx = HelloSessionPrxHelper.uncheckedCast(twoway.ice_datagram())
	Dim batchDatagram As HelloSessionPrx = HelloSessionPrxHelper.uncheckedCast(twoway.ice_batchDatagram())

	Dim timeout As Integer = -1

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
		If line.Equals("t") Then
		    twoway.hello()
		ElseIf line.Equals("o") Then
                    oneway.hello()
		ElseIf line.Equals("O") Then
                    batchOneway.hello()
		ElseIf line.Equals("d") Then
                    datagram.hello()
		ElseIf line.Equals("D") Then
		    batchDatagram.hello()
		ElseIf line.Equals("f") Then
		    communicator.flushBatchRequests()
		ElseIf line.Equals("T") Then
		    If timeout = -1 Then
			timeout = 2000
		    Else
			timeout = -1
		    End If

		    twoway = HelloSessionPrxHelper.uncheckedCast(twoway.ice_timeout(timeout))
		    oneway = HelloSessionPrxHelper.uncheckedCast(oneway.ice_timeout(timeout))
		    batchOneway = HelloSessionPrxHelper.uncheckedCast(batchOneway.ice_timeout(timeout))

		    If timeout = -1 Then
			Console.Out.WriteLine("timeout is now switched off")
		    Else
			Console.Out.WriteLine("timeout is now set to 2000ms")
		    End If
		ElseIf line.Equals("x") Then
		    '  Nothing to do
		ElseIf line.Equals("?") Then
		    menu()
		Else
		    Console.Out.WriteLine("unknown command `" & line & "'")
		    menu()
		End If
	    Catch ex As Exception
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
