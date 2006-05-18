' **********************************************************************
'
' Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports SessionActivation.Demo
Imports System
Imports System.Threading

Module SessionActivationC

    Class Client
        Inherits Ice.Application

        Class SessionKeepAliveThread

            Public Sub New(ByVal session As IceGrid.SessionPrx)
                _session = session
                _timeout = 5000
                _terminated = False
            End Sub

            Public Sub run()
                SyncLock Me
                    While Not _terminated
                        System.Threading.Monitor.Wait(Me, _timeout)
                        If _terminated Then
			    Exit While
                        End If
                        Try
                            _session.keepAlive()
                        Catch ex As Ice.Exception
			    Exit While
                        End Try
                    End While
                End SyncLock
            End Sub

            Public Sub terminate()
                SyncLock Me
                    _terminated = True
                    System.Threading.Monitor.Pulse(Me)
                End SyncLock
            End Sub

            Private _session As IceGrid.SessionPrx
            Private _timeout As Integer
            Private _terminated As Boolean
        End Class

	Private Sub menu()
	    Console.WriteLine("usage:")
	    Console.WriteLine("t: send greeting")
	    Console.WriteLine("x: exit")
	    Console.WriteLine("?: help")
	End Sub

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
	    Dim sessionManager As IceGrid.SessionManagerPrx
	    sessionManager = IceGrid.SessionManagerPrxHelper.checkedCast(communicator().stringToProxy("DemoIceGrid/SessionManager"))
 	    If sessionManager Is Nothing Then
	        Console.Error.WriteLine(": cound not contact session manager")
	    End If

            Dim id As String = Nothing
            Do
                Console.Out.Write("user id: ")
                Console.Out.Flush()

                id = Console.In.ReadLine()
                If id Is Nothing Then
                    Return 1
                End If
                id = id.Trim()
            Loop While id.Length = 0

	    Dim session As IceGrid.SessionPrx = sessionManager.createLocalSession(id)

            Dim keepAlive As SessionKeepAliveThread = New SessionKeepAliveThread(session)
            Dim keepAliveThread As Thread = New Thread(New ThreadStart(AddressOf keepAlive.run))
            keepAliveThread.Start()

	    Dim hello As HelloPrx = Nothing
	    Try
	        hello = HelloPrxHelper.checkedCast(session.allocateObjectById(communicator().stringToIdentity("hello")))
	    Catch ex As Icegrid.AllocationException
	        Console.Error.WriteLine(": could not allocate object: " + ex.reason)
	        Return 1
	    Catch ex As Icegrid.ObjectNotRegisteredException
	        Console.Error.WriteLine(": object not registered with registry")
	        Return 1
	    End Try

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
                    If line.Equals("t") Then
                        hello.sayHello()
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

            keepAlive.terminate()
            keepAliveThread.Join()
            keepAlive = Nothing

	    session.releaseObject(hello.ice_getIdentity())

            Return 0
        End Function
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Client = New Client
        Dim status As Integer = app.Main(args, "config.client")
        System.Environment.Exit(status)
    End Sub
End Module
