' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************
Imports BidirDemo
Imports System.Threading

Module BidirS
    Class Server
        Inherits Ice.Application

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Callback.Server")
            Dim sender As CallbackSenderI = New CallbackSenderI
            adapter.add(sender, communicator().stringToIdentity("sender"))
            adapter.activate()

            Dim t As Thread = New Thread(New ThreadStart(AddressOf sender.Run))
            t.Start()

            Try
                communicator().waitForShutdown()
            Finally
                sender.destroy()
                t.Join()
            End Try

            Return 0
        End Function

    End Class

    Sub Main(ByVal args() As String)
        Dim app As Server = New Server
        Dim status As Integer = app.main(args, "config.server")
        System.Environment.Exit(status)
    End Sub

End Module
