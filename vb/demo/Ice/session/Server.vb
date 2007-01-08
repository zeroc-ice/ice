' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports System.Threading

Module SessionS
    Class Server
        Inherits Ice.Application

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("SessionFactory")

            Dim reaper As ReapThread = New ReapThread
            Dim reaperThread As Thread = New Thread(New ThreadStart(AddressOf reaper.run))
            reaperThread.Start()

            adapter.add(New SessionFactoryI(reaper), communicator().stringToIdentity("SessionFactory"))
            adapter.activate()
            communicator().waitForShutdown()

            reaper.terminate()
            reaperThread.Join()

            Return 0
        End Function
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Server = New Server
        Dim status As Integer = app.main(args, "config.server")
        System.Environment.Exit(status)
    End Sub
End Module
