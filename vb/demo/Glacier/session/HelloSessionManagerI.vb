'  **********************************************************************
' 
'  Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
' 
'  This copy of Ice is licensed to you under the terms described in the
'  ICE_LICENSE file included in this distribution.
' 
'  **********************************************************************

Public NotInheritable Class HelloSessionManagerI
    Inherits Glacier._SessionManagerDisp

    Public Sub New(ByVal adapter As Ice.ObjectAdapter)
	_adapter = adapter
    End Sub

    Public Overloads Overrides Function create(ByVal userId As String, ByVal current As Ice.Current) As Glacier.SessionPrx
	Dim session As Glacier.Session = New HelloSessionI(userId, Me)
	Dim ident As Ice.Identity = New Ice.Identity
	ident.category = userId
	ident.name = "session"

	_adapter.add(session, ident)
	Return Glacier.SessionPrxHelper.uncheckedCast(_adapter.createProxy(ident))
    End Function

    Public Sub remove(ByVal ident As Ice.Identity)
	_adapter.remove(ident)
    End Sub

    Private _adapter As Ice.ObjectAdapter

End Class
