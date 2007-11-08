' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Demo

Public NotInheritable Class CallbackSenderI
    Inherits CallbackSenderDisp_

    Public Overloads Overrides Sub initiateCallback(ByVal proxy As CallbackReceiverPrx, ByVal current As Ice.Current)
        System.Console.Out.WriteLine("initiating callback")
        Try
            proxy.callback(current.ctx)
        Catch ex As System.Exception
            System.Console.Error.WriteLine(ex)
        End Try
    End Sub

    Public Overloads Overrides Sub shutdown(ByVal current As Ice.Current)
        System.Console.Out.WriteLine("Shutting down...")
        Try
            current.adapter.getCommunicator().shutdown()
        Catch ex As System.Exception
            System.Console.Error.WriteLine(ex)
        End Try
    End Sub

End Class
