'  **********************************************************************
' 
'  Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
' 
'  This copy of Ice is licensed to you under the terms described in the
'  ICE_LICENSE file included in this distribution.
' 
'  **********************************************************************

Imports Demo

Public NotInheritable Class HelloSessionI
    Inherits _HelloSessionDisp

    Public Sub New(ByVal userId As String, ByVal manager As HelloSessionManagerI)
	_userId = userId
	_manager = manager
    End Sub

    Public Overloads Overrides Sub hello(ByVal current As Ice.Current)
	System.Console.Out.WriteLine("Hello " & _userId)
    End Sub

    Public Overloads Overrides Sub destroy(ByVal current As Ice.Current)
	_manager.remove(current.id)
    End Sub

    Private _userId As String
    Private _manager As HelloSessionManagerI

End Class
