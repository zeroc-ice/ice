' **********************************************************************
'
' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports IcePackSimpleDemo

Public Class IcePackSimpleI
    Inherits _HelloDisp

    Public Overloads Overrides Sub sayHello(ByVal current As Ice.Current)
        Console.WriteLine("Hello World!")
    End Sub

    Public Overloads Overrides Sub shutdown(ByVal current As Ice.Current)
        Console.WriteLine("Shutting down...")
        current.adapter.getCommunicator().shutdown()
    End Sub

End Class
