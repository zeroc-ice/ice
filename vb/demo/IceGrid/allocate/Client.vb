' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Allocate.Demo
Imports System
Imports System.Threading

Module AllocateC

    Class Client
        Inherits Ice.Application

        Class SessionKeepAliveThread

            Public Sub New(ByVal session As IceGrid.SessionPrx, ByVal timeout As Long)
                _session = session
                _timeout = timeout
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
            Console.WriteLine("s: shutdown server")
            Console.WriteLine("x: exit")
            Console.WriteLine("?: help")
        End Sub

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim status As Integer = 0
            Dim registry As IceGrid.RegistryPrx
            registry = IceGrid.RegistryPrxHelper.checkedCast(communicator().stringToProxy("DemoIceGrid/Registry"))
            If registry Is Nothing Then
                Console.Error.WriteLine("could not contact registry")
            End If

            Dim session As IceGrid.SessionPrx = Nothing
            While True
                Console.Out.WriteLine("This demo accepts any user-id / password combination.")

                Console.Out.Write("user id: ")
                Console.Out.Flush()
                Dim id As String = Console.In.ReadLine()

                Console.Out.Write("password: ")
                Console.Out.Flush()
                Dim pw As String = Console.In.ReadLine()

                Try
                    session = registry.createSession(id, pw)
                    Exit While
                Catch ex As IceGrid.PermissionDeniedException
                    Console.Error.WriteLine("permission denied:\n" + ex.reason)
                End Try
            End While

            Dim keepAlive As SessionKeepAliveThread = New SessionKeepAliveThread(session, registry.getSessionTimeout() / 2)
            Dim keepAliveThread As Thread = New Thread(New ThreadStart(AddressOf keepAlive.run))
            keepAliveThread.Start()

            Try
                Dim hello As HelloPrx = Nothing
                Try
                    hello = HelloPrxHelper.checkedCast(session.allocateObjectById(communicator().stringToIdentity("hello")))
                Catch ex As IceGrid.ObjectNotRegisteredException
                    hello = HelloPrxHelper.checkedCast(session.allocateObjectByType("::Demo::Hello"))
                End Try

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
                            hello.sayHello()
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
            Catch ex As IceGrid.AllocationException
                Console.Error.WriteLine("could not allocate object: " + ex.reason)
                status = 1
            Catch ex As Exception
                Console.Error.WriteLine("unexpected exception: " + ex.ToString())
                status = 1
            End Try

            '
            ' Destroy the keepAlive thread and the sesion object otherwise
            ' the session will be kept allocated until the timeout occurs.
            ' Destroying the session will release all allocated objects.
            '
            keepAlive.terminate()
            keepAliveThread.Join()
            session.destroy()

            Return status
        End Function
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Client = New Client
        Dim status As Integer = app.Main(args, "config.client")
        System.Environment.Exit(status)
    End Sub
End Module
