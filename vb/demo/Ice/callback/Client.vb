' **********************************************************************
'
' Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************
Imports System
Imports Demo

Module CallbackC
    Class Client
        Inherits Ice.Application

        Private Sub menu()
            Console.Out.WriteLine("usage:")
            Console.Out.WriteLine("t: send callback")
            Console.Out.WriteLine("s: shutdown server")
            Console.Out.WriteLine("x: exit")
            Console.Out.WriteLine("?: help")
        End Sub

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            If args.Length > 0 Then
                Console.Error.WriteLine(appName() & ": too many arguments")
                Return 1
            End If

            Dim sender As CallbackSenderPrx = CallbackSenderPrxHelper.checkedCast(communicator().propertyToProxy("Callback.CallbackServer").ice_twoway().ice_timeout(-1).ice_secure(False))
            If sender Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                Return 1
            End If

            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Callback.Client")
            adapter.add(New CallbackReceiverI, communicator().stringToIdentity("callbackReceiver"))
            adapter.activate()

            Dim receiver As CallbackReceiverPrx = CallbackReceiverPrxHelper.uncheckedCast( _
                adapter.createProxy(communicator().stringToIdentity("callbackReceiver")))

            menu()

            Dim line As String = Nothing
            Do
                Try
                    Console.Out.Write("==> ")
                    Console.Out.Flush()
                    line = Console.In.ReadLine()
                    If line Is Nothing Then
                        Exit Do
                    End If
                    If line.Equals("t") Then
                        sender.initiateCallback(receiver)
                    ElseIf line.Equals("s") Then
                        sender.shutdown()
                    ElseIf line.Equals("x") Then
                        '  Nothing to do
                    ElseIf line.Equals("?") Then
                        menu()
                    Else
                        Console.Out.WriteLine("unknown command `" & line & "'")
                        menu()
                    End If
                Catch ex As System.Exception
                    Console.Error.WriteLine(ex)
                End Try
            Loop While Not line.Equals("x")

            Return 0
        End Function

    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Client = New Client
        Dim status As Integer = app.main(args, "config.client")
        System.Environment.Exit(status)
    End Sub

End Module
