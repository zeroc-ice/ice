' **********************************************************************
'
' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports BidirDemo
Imports System.Threading

Class CallbackServer
    Inherits Ice.Application

    Public Overloads Overrides Function run(ByVal args() As String) As Integer
        Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Callback.Server")
        Dim sender As CallbackSenderI = New CallbackSenderI
        adapter.add(sender, Ice.Util.stringToIdentity("sender"))
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
