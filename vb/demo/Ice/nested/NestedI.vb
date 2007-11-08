' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Demo

Public Class NestedI
    Inherits NestedDisp_

    Public Sub New(ByVal self As NestedPrx)
        _self = self
    End Sub

    Public Overloads Overrides Sub nestedCall(ByVal level As Integer, ByVal proxy As NestedPrx, ByVal current As Ice.Current)
        System.Console.Out.WriteLine("" & level)
        level -= 1
        If level > 0 Then
            proxy.nestedCall(level, _self, current.ctx)
        End If
    End Sub

    Private _self As NestedPrx

End Class
