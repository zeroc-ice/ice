Imports System
Imports System.Diagnostics
Imports Filesystem

Public Class FileI
    Inherits FileDisp_

    Public Sub New(ByVal name As String, ByVal parent As DirectoryI)
        _name = name
        _parent = parent

        Debug.Assert(Not _parent Is Nothing)

        ' Create an identity
        '
        Dim myId As Ice.Identity = _adapter.getCommunicator().stringToIdentity(Ice.Util.generateUUID())

        ' Add the identity to the object adapter
        '
        _adapter.add(Me, myId)

        ' Create a proxy for the new node and
        ' add it as a child to the parent
        '
        Dim thisNode As NodePrx = NodePrxHelper.uncheckedCast(_adapter.createProxy(myId))
        _parent.addChild(thisNode)
    End Sub

    ' Slice Node::name() operation

    Public Overloads Overrides Function name(ByVal current As Ice.Current) As String
        Return _name
    End Function

    ' Slice File::read() operation

    Public Overloads Overrides Function read(ByVal current As Ice.Current) As String()
        Return _lines
    End Function

    ' Slice File::write() operation

    Public Overloads Overrides Sub write(ByVal text As String(), ByVal current As Ice.Current)
        _lines = text
    End Sub

    Public Shared _adapter As Ice.ObjectAdapter
    Private _name As String
    Private _parent As DirectoryI
    Private _lines As String()

End Class
