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

Module NestedC

    Class Client
        Inherits Ice.Application

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            If args.Length > 0 Then
                Console.Error.WriteLine(appName() & ": too many arguments")
                Return 1
            End If

            Dim nested As NestedPrx = NestedPrxHelper.checkedCast(communicator().propertyToProxy("Nested.Proxy"))
            If nested Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                Return 1
            End If

            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Nested.Client")
            Dim self As NestedPrx = NestedPrxHelper.uncheckedCast(adapter.createProxy(communicator().stringToIdentity("nestedClient")))
            adapter.add(New NestedI(self), communicator().stringToIdentity("nestedClient"))
            adapter.activate()

            Console.Out.WriteLine("Note: The maximum nesting level is sz * 2, with sz being")
            Console.Out.WriteLine("the maximum number of threads in the server thread pool. If")
            Console.Out.WriteLine("you specify a value higher than that, the application will")
            Console.Out.WriteLine("block or timeout.")
            Console.Out.WriteLine()

            Dim S As String = Nothing
            Do
                Try
                    Console.Out.Write("enter nesting level or 'x' for exit: ")
                    Console.Out.Flush()
                    S = Console.In.ReadLine()
                    If S Is Nothing Then
                        Exit Do
                    End If
                    Dim level As Integer = System.Int32.Parse(S)
                    If level > 0 Then
                        nested.nestedCall(level, self)
                    End If
                Catch ex As System.FormatException
                    ' Ignore
                Catch ex As System.Exception
                    Console.Error.WriteLine(ex)
                End Try
            Loop While Not S.Equals("x")

            Return 0
        End Function

    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Client = New Client
        Dim status As Integer = app.main(args, "config.client")
        System.Environment.Exit(status)
    End Sub

End Module
