'  **********************************************************************
' 
'  Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
' 
'  This copy of Ice is licensed to you under the terms described in the
'  ICE_LICENSE file included in this distribution.
' 
'  **********************************************************************

Imports System

Module sessionS

    Friend Class PermissionsVerifierI
	Inherits Glacier._PermissionsVerifierDisp

	Friend Sub New(ByVal adapter As Ice.ObjectAdapter)
	    _adapter = adapter
	End Sub

	Public Overloads Overrides Function checkPermissions(ByVal userId As String, ByVal password As String, ByRef reason As String, ByVal current As Ice.Current) As Boolean
	    If Not userId.Equals(password) Then
		reason = "user ID does not match password"
		Return False
	    End If
	    reason = ""
	    Return True
	End Function

	Private _adapter As Ice.ObjectAdapter
    End Class

    Private Function run(ByVal args() As String, ByVal communicator As Ice.Communicator) As Integer
	Dim adapter As Ice.ObjectAdapter = communicator.createObjectAdapter("HelloSessionManager")

	' 
	'  Create the SessionManager.
	' 
	Dim [object] As Ice.Object = New HelloSessionManagerI(adapter)
	adapter.add([object], Ice.Util.stringToIdentity("HelloSessionManager"))

	' 
	'  Create the PermissionsVerifier.
	' 
	[object] = New PermissionsVerifierI(adapter)
	adapter.add([object], Ice.Util.stringToIdentity("PermissionsVerifier"))

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
