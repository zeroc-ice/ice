' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports sessionDemo.Demo
Imports System
Imports System.Threading
Imports System.Collections

Module SessionC
    Class Client
        Inherits Ice.Application

        Class SessionRefreshThread

            Public Sub New(ByVal logger As Ice.Logger, ByVal timeout As Integer, ByVal session As SessionPrx)
                _logger = logger
                _session = session
                _timeout = timeout
                _terminated = False
            End Sub

            Public Sub run()
                SyncLock Me
                    While Not _terminated
                        System.Threading.Monitor.Wait(Me, _timeout)
                        If Not _terminated Then
                            Try
                                _session.refresh()
                            Catch ex As Ice.Exception
                                _logger.warning("SessionRefreshThread: " + ex.ToString())
                                _terminated = True
                            End Try
                        End If
                    End While
                End SyncLock
            End Sub

            Public Sub terminate()
                SyncLock Me
                    _terminated = True
                    System.Threading.Monitor.Pulse(Me)
                End SyncLock
            End Sub

            Private _logger As Ice.Logger
            Private _session As SessionPrx
            Private _timeout As Integer
            Private _terminated As Boolean
        End Class

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim name As String = Nothing
            Do
                Console.Out.Write("Please enter your name ==> ")
                Console.Out.Flush()

                name = Console.In.ReadLine()
                If name Is Nothing Then
                    Return 1
                End If
                name = name.Trim()
            Loop While name.Length = 0

            Dim properties As Ice.Properties = communicator().getProperties()
            Dim proxyProperty As String = "SessionFactory.Proxy"
            Dim proxy As String = properties.getProperty(proxyProperty)
            If proxy.Length = 0 Then
                Console.Error.WriteLine("property `" + proxyProperty + "' not set")
                Return 1
            End If

            Dim basePrx As Ice.ObjectPrx = communicator().propertyToProxy("SessionFactory.Proxy")
            Dim factory As SessionFactoryPrx = SessionFactoryPrxHelper.checkedCast(basePrx)
            If factory Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                Return 1
            End If

            Dim session As SessionPrx = factory.create(name)

            Dim refresh As SessionRefreshThread = New SessionRefreshThread(communicator().getLogger(), 5000, session)
            Dim refreshThread As Thread = New Thread(New ThreadStart(AddressOf refresh.run))
            refreshThread.Start()

            Dim hellos As ArrayList = New ArrayList

            menu()

            Try
                Dim destroy As Boolean = True
                Dim shutdown As Boolean = False
                While True
                    Console.Out.Write("==> ")
                    Console.Out.Flush()
                    Dim line As String = Console.In.ReadLine()
                    If line Is Nothing Then
                        Exit While
                    End If
                    If line.Length > 0 AndAlso Char.IsDigit(line.Chars(0)) Then
                        Dim index As Integer = Int32.Parse(line)
                        If index < hellos.Count Then
                            Dim hello As HelloPrx = hellos.Item(index)
                            hello.sayHello()
                        Else
                            Console.Out.WriteLine("Index is too high. " & hellos.Count & " hello objects exist so far.")
                            Console.Out.WriteLine("Use `c' to create a new hello object.")
                        End If
                    ElseIf line.Equals("c") Then
                        hellos.Add(session.createHello())
                        Console.Out.WriteLine("Created hello object " & (hellos.Count - 1))
                    ElseIf line.Equals("s") Then
                        destroy = False
                        shutdown = True
                        Exit While
                    ElseIf line.Equals("x") Then
                        Exit While
                    ElseIf line.Equals("t") Then
                        destroy = False
                        Exit While
                    ElseIf line.Equals("?") Then
                        menu()
                    Else
                        Console.Out.WriteLine("Unknown command `" + line + "'.")
                        menu()
                    End If
                End While

                '
                ' The refresher thread must be terminated before destroy Is
                ' called, otherwIse it might get ObjectNotExIstException. refresh
                ' Is set to 0 so that If session.destroy() raIses an exception
                ' the thread will not be re-terminated and re-joined.
                '
                refresh.terminate()
                refreshThread.Join()
                refresh = Nothing

                If destroy Then
                    session.destroy()
                End If
                If shutdown Then
                    factory.shutdown()
                End If
            Catch e As System.Exception
                '
                ' The refresher thread must be terminated in the event of a
                ' failure.
                '
                If Not refresh Is Nothing Then
                    refresh.terminate()
                    refreshThread.Join()
                    refresh = Nothing
                End If
                Throw e
            End Try

            Return 0
        End Function

        Private Sub menu()
            Console.Out.WriteLine("usage:")
            Console.Out.WriteLine("c:     create a new per-client hello object")
            Console.Out.WriteLine("0-9:   send a greeting to a hello object")
            Console.Out.WriteLine("s:     shutdown the server and exit")
            Console.Out.WriteLine("x:     exit")
            Console.Out.WriteLine("t:     exit without destroying the session")
            Console.Out.WriteLine("?:     help")
        End Sub

    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Client = New Client
        Dim status As Integer = app.main(args, "config.client")
        System.Environment.Exit(status)
    End Sub

End Module
