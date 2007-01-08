' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports System.Collections
Imports Demo

Module AsyncC

    Class Consumer
        Inherits Ice.Application

	Class AMI_Queue_getI
	    Inherits AMI_Queue_get

	    Public Sub New(ByVal id As String)
	        _id = id

		SyncLock _requests.SyncRoot
		    _requests.Add(_id)
		End SyncLock
	    End Sub

	    Public Overloads Overrides Sub ice_response(ByVal message As String)
		SyncLock _requests.SyncRoot
		    _requests.Remove(_id)
		End SyncLock
	        
	        Console.WriteLine(message)
	    End Sub

	    Public Overloads Overrides Sub ice_exception(ByVal ex As Ice.Exception)
		SyncLock _requests.SyncRoot
		    _requests.Remove(_id)
		End SyncLock
	        
		If TypeOf ex Is RequestCanceledException Then
	            Console.Error.WriteLine("Request canceled")
		Else
	            Console.Error.WriteLine(ex)
		End If
	    End Sub

	    Dim _id As String
	End Class

        Private Sub menu()
            Console.WriteLine("usage:")
            Console.WriteLine("g: get a message")
            Console.WriteLine("x: exit")
            Console.WriteLine("?: help")
        End Sub

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim queue As QueuePrx = QueuePrxHelper.checkedCast(communicator().propertyToProxy("Queue.Proxy"))
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
		        Dim id As String = Ice.Util.generateUUID()
                        queue.get_async(new AMI_Queue_getI(id), id)
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

	    SyncLock _requests.SyncRoot
	        If not _requests.Count = 0 Then
		    Try
		        queue.cancel(_requests.ToArray(GetType(String)))
		    Catch ex As System.Exception
		        ' Ignore
		    End Try
		End If
	    End SyncLock

            Return 0
        End Function

	Dim Shared _requests As ArrayList = New ArrayList
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Consumer = New Consumer
        Dim status As Integer = app.Main(args, "config.client")
        System.Environment.Exit(status)
    End Sub
End Module
