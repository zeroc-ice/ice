' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports SessionActivation.Demo

Public Class HelloI
    Inherits HelloDisp_

    Public Sub New(ByVal name As String)
        _name = name
    End Sub

    Public Overloads Overrides Sub sayHello(ByVal current As Ice.Current)
        System.Console.Out.WriteLine(_name + " says Hello World!")
    End Sub

    Public Overloads Overrides Sub shutdown(ByVal current As Ice.Current)
        System.Console.Out.WriteLine(_name + " shutting down...")
        current.adapter.getCommunicator().shutdown()
    End Sub

    Private _name As String

End Class
