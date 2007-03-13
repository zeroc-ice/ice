' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports Demo

Module HelloC

    Class Client
        Inherits Ice.Application

        Private Sub menu()
            Console.WriteLine("usage:")
            Console.WriteLine("t: send greeting as twoway")
            Console.WriteLine("o: send greeting as oneway")
            Console.WriteLine("O: send greeting as batch oneway")
            Console.WriteLine("d: send greeting as datagram")
            Console.WriteLine("D: send greeting as batch datagram")
            Console.WriteLine("f: flush all batch requests")
            Console.WriteLine("T: set a timeout")
            Console.WriteLine("P: set a server delay")
            If _haveSSL Then
                Console.WriteLine("S: switch secure mode on/off")
            End If
            Console.WriteLine("s: shutdown server")
            Console.WriteLine("x: exit")
            Console.WriteLine("?: help")
        End Sub

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Try
                communicator().getPluginManager().getPlugin("IceSSL")
                _haveSSL = True
            Catch ex As Ice.NotRegisteredException
            End Try

            Dim twoway As HelloPrx = HelloPrxHelper.checkedCast(communicator().propertyToProxy("Hello.Proxy").ice_twoway().ice_timeout(-1).ice_secure(False))
            If twoway Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                Return 1
            End If
            Dim oneway As HelloPrx = HelloPrxHelper.uncheckedCast(twoway.ice_oneway())
            Dim batchOneway As HelloPrx = HelloPrxHelper.uncheckedCast(twoway.ice_batchOneway())
            Dim datagram As HelloPrx = HelloPrxHelper.uncheckedCast(twoway.ice_datagram())
            Dim batchDatagram As HelloPrx = HelloPrxHelper.uncheckedCast(twoway.ice_batchDatagram())

            Dim secure As Boolean = False
            Dim timeout As Integer = -1
            Dim delay As Integer = 0

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
                        twoway.sayHello(delay)
                    ElseIf line.Equals("o") Then
                        oneway.sayHello(delay)
                    ElseIf line.Equals("O") Then
                        batchOneway.sayHello(delay)
                    ElseIf line.Equals("d") Then
                        If secure Then
                            Console.WriteLine("secure datagrams are not supported")
                        Else
                            datagram.sayHello(delay)
                        End If
                    ElseIf line.Equals("D") Then
                        If secure Then
                            Console.WriteLine("secure datagrams are not supported")
                        Else
                            batchDatagram.sayHello(delay)
                        End If
                    ElseIf line.Equals("f") Then
                        communicator().flushBatchRequests()
                    ElseIf line.Equals("T") Then
                        If timeout = -1 Then
                            timeout = 2000
                        Else
                            timeout = -1
                        End If

                        twoway = HelloPrxHelper.uncheckedCast(twoway.ice_timeout(timeout))
                        oneway = HelloPrxHelper.uncheckedCast(oneway.ice_timeout(timeout))
                        batchOneway = HelloPrxHelper.uncheckedCast(batchOneway.ice_timeout(timeout))

                        If timeout = -1 Then
                            Console.WriteLine("timeout is now switched off")
                        Else
                            Console.WriteLine("timeout is now set to 2000ms")
                        End If
                    ElseIf line.Equals("P") Then
                        If delay = 0 Then
                            delay = 2500
                        Else
                            delay = 0 
                        End If

                        If delay = 0 Then
                            Console.WriteLine("server delay is now deactivated")
                        Else
                            Console.WriteLine("server delay is now set to 2500ms")
                        End If
                    ElseIf _haveSSL And line.Equals("S") Then
                        secure = Not secure

                        twoway = HelloPrxHelper.uncheckedCast(twoway.ice_secure(secure))
                        oneway = HelloPrxHelper.uncheckedCast(oneway.ice_secure(secure))
                        batchOneway = HelloPrxHelper.uncheckedCast(batchOneway.ice_secure(secure))
                        datagram = HelloPrxHelper.uncheckedCast(datagram.ice_secure(secure))
                        batchDatagram = HelloPrxHelper.uncheckedCast(batchDatagram.ice_secure(secure))

                        If secure Then
                            Console.WriteLine("secure mode is now on")
                        Else
                            Console.WriteLine("secure mode is now off")
                        End If
                    ElseIf line.Equals("s") Then
                        twoway.shutdown()
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

        Private _haveSSL As Boolean = False
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Client = New Client
        Dim status As Integer = app.Main(args, "config.client")
        System.Environment.Exit(status)
    End Sub
End Module
