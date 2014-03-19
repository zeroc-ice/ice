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

Module AsyncC

    Class Client
        Inherits Ice.Application

        Public Sub success()
        End Sub

        Public Sub exception(ByVal ex As Ice.Exception)
            If TypeOf ex Is RequestCanceledException Then
                Console.Error.WriteLine("RequestCanceledException")
            Else
                Console.Error.WriteLine("sayHello AMI call failed:")
                Console.Error.WriteLine(ex)
            End If
        End Sub

        Private Sub menu()
            Console.WriteLine("usage:")
            Console.WriteLine("i: send immediate greeting")
            Console.WriteLine("d: send delayed greeting")
            Console.WriteLine("s: shutdown server")
            Console.WriteLine("x: exit")
            Console.WriteLine("?: help")
        End Sub

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            If args.Length > 0 Then
                Console.Error.WriteLine(appName() & ": too many arguments")
                Return 1
            End If

            Dim hello As HelloPrx = HelloPrxHelper.checkedCast(communicator().propertyToProxy("Hello.Proxy"))
            If hello Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                Return 1
            End If

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
                    If line.Equals("i") Then
                        hello.sayHello(0)
                    ElseIf line.Equals("d") Then
                        hello.begin_sayHello(5000).whenCompleted(AddressOf success, AddressOf exception)
                    ElseIf line.Equals("s") Then
                        hello.shutdown()
                    ElseIf line.Equals("x") Then
                        ' Nothing to do
                    ElseIf line.Equals("?") Then
                        menu()
                    Else
                        Console.WriteLine("unknown command `" & line & "'")
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
        Dim status As Integer = app.Main(args, "config.client")
        System.Environment.Exit(status)
    End Sub
End Module
