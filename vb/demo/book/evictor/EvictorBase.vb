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

        Protected MustOverride Function add(ByVal c As Ice.Current, <Out()> ByRef cookie As Ice.LocalObject) _
                As Ice.Object

        Protected MustOverride Sub evict(ByVal servant As Ice.Object, ByVal cookie As Ice.LocalObject)

        Public Function locate(ByVal c As Ice.Current, <Out()> ByRef cookie As Ice.LocalObject) As Ice.Object _
                Implements Ice.ServantLocator.locate
            SyncLock (Me)
                '
                ' Check if we a servant in the map already.
                '
                Dim entry As EvictorEntry
                entry = CType(_map(c.id), EvictorEntry)
                If Not entry Is Nothing Then
                    '
                    ' Got an entry already, dequeue the entry from
                    ' its current position.
                    '
                    entry.queuePos.Remove()
                Else
                    '
                    ' We do not have an entry. Ask the derived class to
                    ' instantiate a servant and add a new entry to the map.
                    '
                    entry = New EvictorEntry
                    entry.servant = add(c, entry.userCookie) ' Down-call
                    If entry.servant Is Nothing Then
                        cookie = Nothing
                        Return Nothing
                    End If
                    entry.useCount = 0
                    _map(c.id) = entry
                End If

                '
                ' Increment the use count of the servant and enqueue
                ' the entry at the front, so we get LRU order.
                '
                entry.useCount += 1
                _queue.AddFirst(c.id)
                entry.queuePos = CType(_queue.GetEnumerator(), LinkedList.Enumerator)
                entry.queuePos.MoveNext()

                cookie = entry

                Return entry.servant
            End SyncLock
        End Function

        Public Sub finished(ByVal c As Ice.Current, ByVal o As Ice.Object, ByVal cookie As Ice.LocalObject) _
                Implements Ice.ServantLocator.finished
            SyncLock (Me)
                Dim entry As EvictorEntry = CType(cookie, EvictorEntry)

                '
                ' Decrement use count and check if
                ' there is something to evict.
                '
                entry.useCount -= 1
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
            Inherits Ice.LocalObjectImpl
            Friend servant As Ice.Object
            Friend userCookie As Ice.LocalObject
            Friend queuePos As LinkedList.Enumerator
            Friend useCount As Integer
        End Class

        Private Sub evictServants()
            '
            ' If the evictor queue has grown larger than the limit,
            ' look at the excess elements to see whether any of them
            ' can be evicted.
            '
            Dim p As LinkedList.Enumerator = CType(_queue.GetEnumerator(), LinkedList.Enumerator)
            Dim excessEntries As Integer = _map.Count - _size
            For i As Integer = 0 To excessEntries - 1
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
