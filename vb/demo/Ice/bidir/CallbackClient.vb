'  **********************************************************************
' 
'  Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
' 
'  This copy of Ice is licensed to you under the terms described in the
'  ICE_LICENSE file included in this distribution.
' 
'  **********************************************************************

Imports BidirDemo

Class CallbackClient
    Inherits Ice.Application

    Public Overloads Overrides Function run(ByVal args() As String) As Integer

        Dim properties As Ice.Properties = communicator().getProperties()
        Dim proxyProperty As String = "Callback.Client.CallbackServer"
        Dim proxy As String = properties.getProperty(proxyProperty)
        If proxy.Length = 0 Then
            System.Console.Error.WriteLine("property `" & proxyProperty & "' not set")
            Return 1
        End If

        Dim base As Ice.ObjectPrx = communicator().stringToProxy(proxy)
        Dim server As CallbackSenderPrx = CallbackSenderPrxHelper.checkedCast(base)
        If server Is Nothing Then
            System.Console.Error.WriteLine("invalid proxy")
            Return 1
        End If

        Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Callback.Client")
        Dim ident As Ice.Identity = New Ice.Identity
        ident.name = Ice.Util.generateUUID()
        ident.category = ""
        adapter.add(New CallbackReceiverI, ident)
        adapter.activate()
        server.ice_connection().setAdapter(adapter)
        server.addClient(ident)
        communicator().waitForShutdown()

        Return 0
    End Function

End Class
