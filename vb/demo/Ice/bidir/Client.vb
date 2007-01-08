' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports BidirDemo

Module BidirC
    Class Client
        Inherits Ice.Application

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim server As CallbackSenderPrx = CallbackSenderPrxHelper.checkedCast(communicator().propertyToProxy("Callback.Client.CallbackServer"))
            If server Is Nothing Then
                System.Console.Error.WriteLine("invalid proxy")
                Return 1
            End If

            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("")
            Dim ident As Ice.Identity = New Ice.Identity
            ident.name = Ice.Util.generateUUID()
            ident.category = ""
            adapter.add(New CallbackReceiverI, ident)
            adapter.activate()
            server.ice_getConnection().setAdapter(adapter)
            server.addClient(ident)
            communicator().waitForShutdown()

            Return 0
        End Function

    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Client = New Client
        Dim status As Integer = app.main(args, "config.client")
        System.Environment.Exit(status)
    End Sub

End Module
