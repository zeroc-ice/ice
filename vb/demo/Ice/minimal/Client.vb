' **********************************************************************
'
' Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports Demo

Module MinimalC

    Public Sub Main(ByVal args() As String)
        Dim status As Integer = 0
        Dim communicator As Ice.Communicator = Nothing

        Try
            communicator = Ice.Util.initialize(args)
	    If args.Length > 0 Then
		Console.Error.WriteLine("too many arguments")
		System.Environment.Exit(1)
	    End If

            Dim hello As HelloPrx = HelloPrxHelper.checkedCast(communicator.stringToProxy("hello:tcp -p 10000"))
            If hello Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                status = 1
            Else
                hello.sayHello()
            End If
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
