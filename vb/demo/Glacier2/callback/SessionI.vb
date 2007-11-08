' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System

Public NotInheritable Class SessionI
    Inherits Glacier2.SessionDisp_

    Public Sub New(ByVal userId As String)
        _userId = userId
    End Sub

    Public Overloads Overrides Sub destroy(ByVal current As Ice.Current)
        Console.WriteLine("destroying session for user `" & _userId & "'")
        current.adapter.remove(current.id)
    End Sub

    Private ReadOnly _userId As String

End Class
