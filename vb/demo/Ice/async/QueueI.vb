' **********************************************************************
'
' Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Demo
Imports System
Imports System.Collections

Public Class QueueI
    Inherits QueueDisp_

    Public Overloads Overrides Sub get_async(ByVal getCB As AMD_Queue_get, ByVal current As Ice.Current)
        SyncLock Me
	    If not _messageQueue.Count = 0 Then
	        Try
		    getCB.ice_response(_messageQueue(0))
		    _messageQueue.RemoveAt(0)
		Catch ex As Ice.Exception
		    Console.Error.WriteLine(ex)
		End Try
	    Else
	        _requestQueue.Add(getCB)
	    End If
	End SyncLock
    End Sub

    Public Overloads Overrides Sub add(ByVal message As String, ByVal current As Ice.Current)
        SyncLock Me
	    If not _requestQueue.Count = 0 Then
	        Try
		    Dim cb As AMD_Queue_get = _requestQueue(0)
		    cb.ice_response(message)
		Catch ex As Ice.Exception
		    Console.Error.WriteLine(ex)
		End Try
		_requestQueue.RemoveAt(0)
	    Else
	        _messageQueue.Add(message)
	    End If
	End SyncLock
    End Sub

    private _messageQueue As ArrayList = New ArrayList
    private _requestQueue As ArrayList = New ArrayList
End Class
