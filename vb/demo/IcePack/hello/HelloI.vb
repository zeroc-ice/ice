' **********************************************************************
'
' Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports System.Diagnostics
Imports IcePackHelloDemo

Public Class IcePackHelloI
    Inherits Hello

    Public Sub New(ByVal n As String)
        name = n
    End Sub

    Public Overloads Overrides Sub sayHello(ByVal current As Ice.Current)
        Console.WriteLine(name & " says Hello World!")
    End Sub

    Public Overloads Overrides Sub destroy(ByVal current As Ice.Current)
        Dim adapter As Ice.ObjectAdapter = current.adapter
        Dim communicator As Ice.Communicator = adapter.getCommunicator()

        '
        ' Get the IcePack Admin interface and remove the Hello object
        ' from the IcePack object registry.
        ' 
        Dim admin As IcePack.AdminPrx = IcePack.AdminPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Admin"))
        Try
            admin.removeObject(adapter.createProxy(current.id))
        Catch ex As IcePack.ObjectNotExistException
            Debug.Assert(False)
        End Try

        '
        ' Remove the Hello object from the object adapter.
        '
        adapter.remove(current.id)

        Dim id As String = communicator.getProperties().getProperty("Identity")

        Console.WriteLine("HelloFactory-" & id & ": destroyed Hello object named '" & name & "'")
    End Sub

End Class
