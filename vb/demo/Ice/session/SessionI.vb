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
Imports System.Collections

Public Class SessionI
    Inherits SessionDisp_

    Public Sub New(ByVal name As String)

        _name = name
        _timestamp = System.DateTime.Now
        _nextId = 0
        _destroy = False
        _objs = New ArrayList

        Console.Out.Write("The session " + _name)
        Console.Out.WriteLine(" is now created.")
    End Sub

    Public Overloads Overrides Function createHello(ByVal c As Ice.Current) As HelloPrx

        SyncLock Me
            If _destroy Then
                Throw New Ice.ObjectNotExistException
            End If

            Dim hello As HelloPrx = HelloPrxHelper.uncheckedCast(c.adapter.addWithUUID(New HelloI(_name, _nextId)))
            _nextId = _nextId + 1
            _objs.Add(hello)
            Return hello
        End SyncLock
    End Function

    Public Overloads Overrides Sub refresh(ByVal c As Ice.Current)
        SyncLock Me
            If _destroy Then
                Throw New Ice.ObjectNotExistException
            End If

            _timestamp = System.DateTime.Now
        End SyncLock
    End Sub
    Public Overloads Overrides Function getName(ByVal c As Ice.Current) As String
        SyncLock Me
            If _destroy Then
                Throw New Ice.ObjectNotExistException
            End If

            Return _name

        End SyncLock
    End Function

    Public Overloads Overrides Sub destroy(ByVal c As Ice.Current)
        SyncLock Me
            If _destroy Then
                Throw New Ice.ObjectNotExistException
            End If
            _destroy = True

            Console.Out.Write("The session " + _name)
            Console.Out.WriteLine(" is now destroyed.")
            Try
                c.adapter.remove(c.id)
                For Each p As HelloPrx In _objs
                    c.adapter.remove(p.ice_getIdentity())
                Next
            Catch e As Ice.ObjectAdapterDeactivatedException

                ' This method is called on shutdown of the server, in which
                ' case this exception is expected.
            End Try
        End SyncLock

        _objs.Clear()
    End Sub

    Public Function timestamp() As System.DateTime
        SyncLock Me
            If _destroy Then
                Throw New Ice.ObjectNotExistException
            End If
            Return _timestamp
        End SyncLock
    End Function

    Private _name As String
    Private _timestamp As System.DateTime ' The last time the session was refreshed.
    Private _nextId As Integer ' The per-session id of the next hello object. This is used for tracing purposes.
    Private _objs As ArrayList  ' List of per-session allocated hello objects.
    Private _destroy As Boolean
End Class