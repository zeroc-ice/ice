' **********************************************************************
'
' Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Demo
Imports System
Imports System.Collections
Imports System.Threading

Public Class WorkQueue

    Private Class CallbackEntry
        Public cb As AMD_Hello_sayHello
        Public delay As Integer
    End Class

    Public Sub Join()
        _thread.Join()
    End Sub

    Public Sub Start()
        _thread = New Thread(New ThreadStart(AddressOf Me.Run))
        _thread.Start()
    End Sub

    Public Sub Run()
        SyncLock Me
            While Not _done
                If _callbacks.Count = 0 Then
                    Monitor.Wait(Me)
                End If

                If Not _callbacks.Count = 0 Then
                    Dim entry As CallbackEntry = _callbacks(0)
                    Monitor.Wait(Me, entry.delay)

                    If Not _done Then
                        _callbacks.RemoveAt(0)
                        Console.WriteLine("Belated Hello World!")
                        entry.cb.ice_response()
                    End If
                End If
            End While

            Dim e As CallbackEntry
            For Each e In _callbacks
                e.cb.ice_exception(New RequestCanceledException())
            Next
        End SyncLock
    End Sub

    Public Sub Add(ByVal cb As AMD_Hello_sayHello, ByVal delay As Integer)
        SyncLock Me
            If Not _done Then
                Dim entry As CallbackEntry = New CallbackEntry
                entry.cb = cb
                entry.delay = delay

                If _callbacks.Count = 0 Then
                    Monitor.Pulse(Me)
                End If
                _callbacks.Add(entry)
            Else
                cb.ice_exception(New RequestCanceledException())
            End If
        End SyncLock
    End Sub

    Public Sub destroy()
        SyncLock Me
            _done = True
            Monitor.Pulse(Me)
        End SyncLock
    End Sub
    
    Private _thread As Thread
    Private _done As Boolean 
    Private _callbacks As ArrayList = New ArrayList
End Class
