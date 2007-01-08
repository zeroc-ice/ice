' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System

Module MinimalS

    Public Sub Main(ByVal args() As String)
        Dim status As Integer = 0
        Dim communicator As Ice.Communicator = Nothing

        Try
            communicator = Ice.Util.initialize(args)
            Dim adapter As Ice.ObjectAdapter = communicator.createObjectAdapterWithEndpoints("Hello", "tcp -p 10000")
            adapter.add(New HelloI, communicator.stringToIdentity("hello"))
            adapter.activate()
            communicator.waitForShutdown()
        Catch ex As System.Exception
            Console.Error.WriteLine(ex)
            status = 1
        End Try

        If Not communicator Is Nothing Then
            Try
                communicator.destroy()
            Catch ex As System.Exception
                Console.Error.WriteLine(ex)
                status = 1
            End Try
        End If

        System.Environment.Exit(status)
    End Sub
End Module
