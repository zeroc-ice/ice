' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Demo

Public Class HelloI
    Inherits HelloDisp_

    Public Overloads Overrides Sub sayHello(ByVal delay As Integer, ByVal current As Ice.Current)
        If delay > 0 Then
            System.Threading.Thread.Sleep(delay)
        End If
        System.Console.Out.WriteLine("Hello World!")
    End Sub

    Public Overloads Overrides Sub shutdown(ByVal current As Ice.Current)
        System.Console.Out.WriteLine("Shutting down...")
        current.adapter.getCommunicator().shutdown()
    End Sub
End Class
