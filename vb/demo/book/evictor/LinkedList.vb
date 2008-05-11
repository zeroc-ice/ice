Imports System
Imports System.Collections
Imports System.Diagnostics

Namespace Evictor

    Public Class LinkedList
        Implements ICollection, ICloneable

        Public Sub New()
            _head = Nothing
            _tail = Nothing
            _count = Nothing
        End Sub

        Public ReadOnly Property Count() As Integer Implements ICollection.Count
            Get
                Return _count
            End Get
        End Property

        Public ReadOnly Property IsSynchronized() As Boolean Implements ICollection.IsSynchronized
            Get
                Return False
            End Get
        End Property

        Public ReadOnly Property SyncRoot() As Object Implements ICollection.SyncRoot
            Get
                Return Me
            End Get
        End Property

        Public Sub CopyTo(ByVal array As System.Array, ByVal index As Integer) Implements ICollection.CopyTo
            '
            ' Check preconditions.
            '
            If array Is Nothing Then
                Throw New ArgumentNullException("array", "array parameter must not be null")
            End If
            If index < 0 Then
                Throw New ArgumentOutOfRangeException("index", _count, "index must not be less than zero")
            End If
            If index >= array.Length Then
                Throw New ArgumentException("index out of bounds for array", "index")
            End If
            If array.Length - index > _count Then
                Throw New ArgumentException("insufficient room in array", "array")
            End If
            If array.Rank <> 1 Then
                Throw New ArgumentException("array must be one-dimensional", "array")
            End If

            '
            ' Copy the elements.
            '
            Dim n As Node = _head
            While Not n Is Nothing
                array.SetValue(n.val, index)
                index += 1
                n = CType(n.next, Node)
            End While
        End Sub

        Public Function GetEnumerator() As IEnumerator Implements IEnumerable.GetEnumerator
            Return New Enumerator(Me)
        End Function

        Public Function Clone() As Object Implements ICloneable.Clone
            Dim l As LinkedList = New LinkedList
            Dim cursor As Node = _head
            While Not cursor Is Nothing
                l.Add(cursor.val)
                cursor = cursor.next
            End While
            Return l
        End Function

        Public Sub Add(ByVal value As Object)
            Dim n As Node = New Node
            n.val = value
            If _tail Is Nothing Then
                n.prev = Nothing
                _head = n
            Else
                n.prev = _tail
                _tail.next = n
            End If
            n.next = Nothing
            _tail = n
            _count += 1
        End Sub

        Public Sub AddFirst(ByVal value As Object)
            Dim n As Node = New Node
            n.val = value
            If _head Is Nothing Then
                n.next = Nothing
                _tail = n
            Else
                n.next = _head
                _head.prev = n
            End If
            n.prev = Nothing
            _head = n
            _count += 1
        End Sub

        Private Sub Remove(ByVal n As Node)
            Debug.Assert(Not n Is Nothing)
            Debug.Assert(_count <> 0)

            If Not n.prev Is Nothing Then
                n.prev.next = n.next
            Else
                _head = n.next
            End If
            If Not n.next Is Nothing Then
                n.next.prev = n.prev
            Else
                _tail = n.prev
            End If
            _count -= 1
        End Sub

        Friend Class Node
            Friend [next] As Node
            Friend prev As Node
            Friend val As Object
        End Class

        Private _head As Node
        Private _tail As Node
        Private _count As Integer

        Public Class Enumerator
            Implements IEnumerator

            Friend Sub New(ByVal list As LinkedList)
                _list = list
                _current = Nothing
                _movePrev = Nothing
                _moveNext = Nothing
                _removed = False
            End Sub

            Public Sub Reset() Implements IEnumerator.Reset
                _current = Nothing
                _movePrev = Nothing
                _moveNext = Nothing
                _removed = False
            End Sub

            Public ReadOnly Property Current() As Object Implements IEnumerator.Current
                Get
                    If _current Is Nothing Then
                        Throw New InvalidOperationException("iterator not positioned on an element")
                    End If
                    Return _current.val
                End Get
            End Property

            Public Function MoveNext() As Boolean Implements IEnumerator.MoveNext
                If _removed Then
                    _current = _moveNext
                    _moveNext = Nothing
                    _movePrev = Nothing
                    _removed = False
                Else
                    If _current Is _list._tail Then  ' Make sure the iterator "sticks" if on last element.
                        Return False
                    End If
                    If _current Is Nothing Then
                        _current = _list._head
                    Else
                        _current = _current.next
                    End If
                End If
                Return Not _current Is Nothing
            End Function

            Public Function MovePrev() As Boolean
                If _removed Then
                    _current = _movePrev
                    _movePrev = Nothing
                    _moveNext = Nothing
                    _removed = False
                Else
                    If _current Is _list._head Then ' Make sure the iterator "sticks" if on first element.
                        Return False
                    End If
                    If _current Is Nothing Then
                        _current = _list._tail
                    Else
                        _current = _current.prev
                    End If
                End If
                Return Not _current Is Nothing
            End Function

            Public Sub Remove()
                If _current Is Nothing Then
                    Throw New InvalidOperationException("iterator is not positioned on an element")
                End If
                _removed = True
                _moveNext = _current.next ' Remember where to move next for call to MoveNext().
                _movePrev = _current.prev ' Remember where to move next for call to MovePrev().
                _list.Remove(_current)
                _current = Nothing
            End Sub

            Private _list As LinkedList ' The list we are iterating over.
            Private _current As Node    ' Current iterator position.
            Private _moveNext As Node   ' Remembers node that followed a removed element.
            Private _movePrev As Node   ' Remembers node that preceded a removed element.
            Private _removed As Boolean ' True after a call to Remove(), false otherwise.

        End Class

    End Class

End Namespace
