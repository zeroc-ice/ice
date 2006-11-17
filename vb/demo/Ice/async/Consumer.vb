' **********************************************************************
'
' Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports Demo

Module AsyncC

    Class Consumer
        Inherits Ice.Application

	Class AMI_Queue_getI
	    Inherits AMI_Queue_get

	    Public Overloads Overrides Sub ice_response(ByVal message As String)
	        Console.WriteLine(message)
	    End Sub

	    Public Overloads Overrides Sub ice_exception(ByVal ex As Ice.Exception)
	        Console.Error.WriteLine(ex)
	    End Sub
	End Class

        Private Sub menu()
            Console.WriteLine("usage:")
            Console.WriteLine("g: get a message")
            Console.WriteLine("x: exit")
            Console.WriteLine("?: help")
        End Sub

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim properties As Ice.Properties = communicator().getProperties()
            Dim proxyProperty As String = "Queue.Proxy"
            Dim proxy As String = properties.getProperty(proxyProperty)
            If proxy.Length = 0 Then
                Console.Error.WriteLine("property `" & proxyProperty & "' not set")
                Return 1
            End If

            Dim queue As QueuePrx = QueuePrxHelper.checkedCast(communicator().stringToProxy(proxy))
            If queue Is Nothing Then
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
                        Exit Try
                    End If
                    If line.Equals("g") Then
                        queue.get_async(new AMI_Queue_getI)
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
        Dim app As Consumer = New Consumer
        Dim status As Integer = app.Main(args, "config.client")
        System.Environment.Exit(status)
    End Sub
End Module
