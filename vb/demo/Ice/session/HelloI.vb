' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports sessionDemo.Demo
Imports System

Class HelloI
    Inherits HelloDisp_

    Public Sub New(ByVal name As String, ByVal id As Integer)
        _name = name
        _id = id
    End Sub

    Public Overloads Overrides Sub sayHello(ByVal current As Ice.Current)
        Console.Out.WriteLine("Hello object #" & _id & " for session `" & _name & "' says:")
        Console.Out.WriteLine("Hello " & _name & "!")
    End Sub

    Private _name As String
    Private _id As Integer

End Class
