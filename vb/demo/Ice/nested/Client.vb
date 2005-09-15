' **********************************************************************
'
' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
            Dim properties As Ice.Properties = communicator().getProperties()
            Dim proxyProperty As String = "Nested.Client.NestedServer"
            Dim proxy As String = properties.getProperty(proxyProperty)
            If proxy.Length = 0 Then
                Console.Error.WriteLine("property `" & proxyProperty & "' not set")
                Return 1
            End If

            Dim nested As NestedPrx = NestedPrxHelper.checkedCast(communicator().stringToProxy(proxy))
            If nested Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                Return 1
            End If

            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Nested.Client")
            Dim self As NestedPrx = NestedPrxHelper.uncheckedCast(adapter.createProxy(Ice.Util.stringToIdentity("nestedClient")))
            adapter.add(New NestedI(self), Ice.Util.stringToIdentity("nestedClient"))
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
                        Exit Try
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
        Dim status As Integer = app.main(args, "config")
        System.Environment.Exit(status)
    End Sub

End Module
