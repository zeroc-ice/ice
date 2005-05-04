' **********************************************************************
'
' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports System.Threading

Class SessionServer
    Inherits Ice.Application

    Public Overloads Overrides Function run(ByVal args() As String) As Integer
        Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("SessionFactory")

        Dim reaper As ReapThread = New ReapThread
        Dim reaperThread As Thread = New Thread(New ThreadStart(AddressOf reaper.run))
        reaperThread.Start()

        adapter.add(New SessionFactoryI(reaper), Ice.Util.stringToIdentity("SessionFactory"))
        adapter.activate()
        communicator().waitForShutdown()

        reaper.terminate()
        reaperThread.Join()

        Return 0
    End Function
End Class