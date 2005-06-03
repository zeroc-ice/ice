' **********************************************************************
'
' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Class Server
    Inherits Ice.Application

    Public Overloads Overrides Function run(ByVal args() As String) As Integer
	Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Value")
	Dim [object] As Ice.Object = New InitialI(adapter)
	adapter.add([object], Ice.Util.stringToIdentity("initial"))
	adapter.activate()
	communicator().waitForShutdown()
	Return 0
    End Function

End Class

Module valueS

    Public Sub Main(ByVal args() as String)
        Dim app as Server = new Server
        Dim status as Integer = app.main(args, "config")
	System.Environment.Exit(status)
    End Sub

End Module
