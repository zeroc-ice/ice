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

    Public Overloads Overrides Sub get_async(ByVal cb As AMD_Queue_get, ByVal id As String, ByVal current As Ice.Current)
        SyncLock Me
	    If not _messageQueue.Count = 0 Then
	        Try
		    cb.ice_response(_messageQueue(0))
		    _messageQueue.RemoveAt(0)
		Catch ex As Ice.Exception
		    Console.Error.WriteLine(ex)
		End Try
	    Else
	        Dim request As Request = New Request
		request.id = id
		request.cb = cb
	        _requestQueue.Add(request)
	    End If
	End SyncLock
    End Sub

    Public Overloads Overrides Sub add(ByVal message As String, ByVal current As Ice.Current)
        SyncLock Me
	    If not _requestQueue.Count = 0 Then
	        Try
		    Dim request As Request = _requestQueue(0)
		    request.cb.ice_response(message)
		Catch ex As Ice.Exception
		    Console.Error.WriteLine(ex)
		End Try
		_requestQueue.RemoveAt(0)
	    Else
	        _messageQueue.Add(message)
	    End If
	End SyncLock
    End Sub
    
    Public Overloads Overrides Sub cancel_async(ByVal cb As AMD_Queue_cancel, ByVal ids As String(), ByVal current As Ice.Current)
	cb.ice_response()

	SyncLock Me
	    For i As Integer = 0 To ids.Length
		Dim r As Request
	        For Each r In _requestQueue
		    If r.id.Equals(ids(i)) Then
		        Try
			    r.cb.ice_exception(New RequestCanceledException())
			Catch ex As Ice.Exception
			    ' Ignore
			End try

		        _requestQueue.Remove(r)
			Exit For
		    End If
		Next
	    Next
	End SyncLock
    End Sub

    Private Class Request
        Public id As String
	Public cb As AMD_Queue_get
    End Class

    Private _messageQueue As ArrayList = New ArrayList
    Private _requestQueue As ArrayList = New ArrayList
End Class
