Imports System
Imports System.Collections
Imports Filesystem

Public Class DirectoryI
    Inherits DirectoryDisp_

    Public Sub New(ByVal name As String, ByVal parent As DirectoryI)
        _name = name
        _parent = parent

        ' Create an identity. The
        ' parent has the fixed identity "/"
        '
        Dim myId As Ice.Identity
        If Not _parent Is Nothing Then
            myId = _adapter.getCommunicator().stringToIdentity(Ice.Util.generateUUID())
        Else
            myId = _adapter.getCommunicator().stringToIdentity("RootDir")
        End If

        ' Add the identity to the object adapter
        '
        _adapter.add(Me, myId)

        ' Create a proxy for the new node and
        ' add it as a child to the parent
        '
        Dim thisNode As NodePrx = NodePrxHelper.uncheckedCast(_adapter.createProxy(myId))
        If Not _parent Is Nothing Then
            _parent.addChild(thisNode)
        End If
    End Sub

    Public Sub addChild(ByVal child As NodePrx)
        _contents.Add(child)
    End Sub

    Public Overloads Overrides Function name(ByVal current As Ice.Current) As String
        Return _name
    End Function

    Public Overloads Overrides Function list(ByVal current As Ice.Current) As NodePrx()
        Return CType(_contents.ToArray(GetType(NodePrx)), NodePrx())
    End Function

    Public Shared _adapter As Ice.ObjectAdapter
    Private _name As String
    Private _parent As DirectoryI
    Private _contents As ArrayList = New ArrayList

End Class
