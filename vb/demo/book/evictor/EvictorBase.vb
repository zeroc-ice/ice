' **********************************************************************
'
' Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System.Runtime.InteropServices

Namespace Evictor

    Public MustInherit Class EvictorBase
        Inherits Ice.LocalObjectImpl
        Implements Ice.ServantLocator

        Public Sub New()
            _size = 1000
        End Sub

        Public Sub New(ByVal size As Integer)
            If size < 0 Then
                _size = 1000
            Else
                _size = size
            End If
        End Sub

        Public MustOverride Function add(ByVal c As Ice.Current, <Out()> ByRef cookie As Ice.LocalObject) As Ice.Object

        Public MustOverride Sub evict(ByVal servant As Ice.Object, ByVal cookie As Ice.LocalObject)

        Public Function locate(ByVal c As Ice.Current, <Out()> ByRef cookie As Ice.LocalObject) As Ice.Object Implements Ice.ServantLocator.locate
            SyncLock (Me)
                '
                ' Create a cookie.
                '
                Dim ec As EvictorCookie = New EvictorCookie

                '
                ' Check if we a servant in the map already.
                '
                ec.entry = CType(_map(c.id), EvictorEntry)
                Dim newEntry As Boolean = ec.entry Is Nothing
                If Not newEntry Then
                    '
                    ' Got an entry already, dequeue the entry from
                    ' its current position.
                    '
                    ec.entry.pos.Remove()
                Else
                    '
                    ' We do not have an entry. Ask the derived class to
                    ' instantiate a servant and add a new entry to the map.
                    '
                    ec.entry = New EvictorEntry
                    Dim theCookie As Ice.LocalObject
                    ec.entry.servant = add(c, theCookie) ' Down-call
                    If ec.entry.servant Is Nothing Then
                        Throw New Ice.ObjectNotExistException
                    End If
                    ec.entry.userCookie = theCookie
                    ec.entry.useCount = 0
                    _map(c.id) = ec.entry
                End If

                '
                ' Increment the use count of the servant and enqueue
                ' the entry at the front, so we get LRU order.
                '
                ec.entry.useCount += 1
                _queue.AddFirst(c.id)
                ec.entry.pos = CType(_queue.GetEnumerator(), LinkedList.Enumerator)
                ec.entry.pos.MovePrev()
                cookie = ec

                Return ec.entry.servant
            End SyncLock
        End Function

        Public Sub finished(ByVal c As Ice.Current, ByVal o As Ice.Object, ByVal cookie As Ice.LocalObject) Implements Ice.ServantLocator.finished
            SyncLock (Me)
                Dim ec As EvictorCookie = CType(cookie, EvictorCookie)

                '
                ' Decrement use count and check if
                ' there is something to evict.
                '
                ec.entry.useCount -= 1
                evictServants()
            End SyncLock
        End Sub

        Public Sub deactivate(ByVal category As String) Implements Ice.ServantLocator.deactivate
            SyncLock (Me)
                _size = 0
                evictServants()
            End SyncLock
        End Sub

        Private Class EvictorEntry
            Friend servant As Ice.Object
            Friend userCookie As Ice.LocalObject
            Friend pos As LinkedList.Enumerator
            Friend useCount As Integer
        End Class

        Private Class EvictorCookie
            Inherits Ice.LocalObjectImpl
            Public entry As EvictorEntry
        End Class

        Private Sub evictServants()
            '
            ' If the evictor queue has grown larger than the limit,
            ' look at the excess elements to see whether any of them
            ' can be evicted.
            '
            Dim p As LinkedList.Enumerator = CType(_queue.GetEnumerator(), LinkedList.Enumerator)
            For i As Integer = _map.Count - _size To 1 Step -1
                p.MovePrev()
                Dim id As Ice.Identity = CType(p.Current, Ice.Identity)
                Dim e As EvictorEntry = CType(_map(id), EvictorEntry)
                If e.useCount = 0 Then
                    evict(e.servant, e.userCookie) ' Down-call
                    p.Remove()
                    _map.Remove(id)
                End If
            Next
        End Sub

        Private _map As System.Collections.Hashtable = New System.Collections.Hashtable
        Private _queue As LinkedList = New LinkedList
        Private _size As Integer
    End Class

End Namespace
