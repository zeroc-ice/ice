' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Demo

Module NestedS
    Class Server
        Inherits Ice.Application

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Nested.Server")
            Dim self As NestedPrx = NestedPrxHelper.uncheckedCast(adapter.createProxy(communicator().stringToIdentity("nestedServer")))
            adapter.add(New NestedI(self), communicator().stringToIdentity("nestedServer"))
            adapter.activate()
            communicator().waitForShutdown()
            Return 0
        End Function

    End Class

    Sub Main(ByVal args() As String)
        Dim app As Server = New Server
        Dim status As Integer = app.main(args, "config.server")
        System.Environment.Exit(status)
    End Sub

End Module
