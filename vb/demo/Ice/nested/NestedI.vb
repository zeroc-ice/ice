' **********************************************************************
'
' Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
            '
            ' Ensure the invocation times out if the nesting level is too
            ' high and there are no more threads in the thread pool to
            ' dispatch the call.
            ' 
      	    NestedPrxHelper.uncheckedCast(proxy.ice_invocationTimeout(5000)).nestedCall(level, _self, current.ctx)
        End If
    End Sub

    Private _self As NestedPrx

End Class
