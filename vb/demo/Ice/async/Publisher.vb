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

    Class Publisher
        Inherits Ice.Application

        Private Sub menu()
            Console.WriteLine("Enter /quit to exit.")
        End Sub

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim queue As QueuePrx = QueuePrxHelper.checkedCast(communicator().propertyToProxy("Queue.Proxy"))
            If queue Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                Return 1
            End If

	    Console.WriteLine("Type a message and hit return to queue a message.")
            menu()

            Dim line As String = Nothing
            Try
                While 1
                    Console.Out.Write("==> ")
                    Console.Out.Flush()
                    line = Console.In.ReadLine()
                    If line Is Nothing Then
                        Exit Try
                    End If
		    If not line.Length = 0 Then
                        If line.Chars(0) = "/" Then
			    If line.Equals("/quit") Then
			        Exit While
			    Else
			        menu()
			    End If
                        Else
                            queue.add(line)
                        End If
		    End If
            End While
            Catch ex As System.Exception
                Console.Error.WriteLine(ex)
            End Try

            Return 0
        End Function
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Publisher = New Publisher
        Dim status As Integer = app.Main(args, "config.client")
        System.Environment.Exit(status)
    End Sub
End Module
