' **********************************************************************
'
' Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System

Public Module HelloIcePackServer

    Public Class Server
        Inherits Ice.Application

        Public Overrides Function run(ByVal args As String()) As Integer
            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Hello")

            Dim id As String = communicator().getProperties().getProperty("Identity")

            Dim [object] As Ice.Object = New HelloFactoryI
            adapter.add([object], Ice.Util.stringToIdentity(id))
            adapter.activate()

            communicator().waitForShutdown()
            Return 0
        End Function

    End Class

    Public Sub Main(ByVal args As String())
        Dim app As Server = New Server
        Dim status As Integer = app.main(args)
        Environment.Exit(status)
    End Sub

End Module
