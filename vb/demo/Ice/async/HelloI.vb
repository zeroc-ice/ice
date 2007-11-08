' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Demo
Imports System

Public Class HelloI
    Inherits HelloDisp_

    Public Sub New(ByVal workQueue As WorkQueue)
        _workQueue = workQueue
    End Sub

    Public Overloads Overrides Sub sayHello_async(ByVal cb As AMD_Hello_sayHello, ByVal delay As Integer, ByVal current As Ice.Current)
        If delay = 0 Then
            Console.WriteLine("Hello World!")
            cb.ice_response()
        Else
            _workQueue.Add(cb, delay)
        End If
    End Sub

    Public Overloads Overrides Sub shutdown(ByVal current As Ice.Current)
        _workQueue.destroy()
        current.adapter.getCommunicator().shutdown()
    End Sub
    
    Private _workQueue As WorkQueue
End Class
