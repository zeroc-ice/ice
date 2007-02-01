' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Module SessionActivationS

    Class Server
        Inherits Ice.Application

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Hello")
            Dim properties As Ice.Properties = communicator().getProperties()
            Dim id As Ice.Identity = communicator().stringToIdentity(properties.getProperty("Identity"))
            adapter.add(New HelloI(properties.getProperty("Ice.ServerId")), id)
            adapter.activate()
            communicator.waitForShutdown()
            Return 0
        End Function
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Server = New Server
        Dim status As Integer = app.Main(args)
        System.Environment.Exit(status)
    End Sub
End Module
