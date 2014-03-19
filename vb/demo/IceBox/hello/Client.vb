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

Module HelloIceBoxC

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
            If _haveSSL Then
                Console.WriteLine("S: switch secure mode on/off")
            End If
            Console.WriteLine("x: exit")
            Console.WriteLine("?: help")
        End Sub

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            If args.Length > 0 Then
                Console.Error.WriteLine(appName() & ": too many arguments")
                Return 1
            End If

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
                        twoway.sayHello()
                    ElseIf line.Equals("o") Then
                        oneway.sayHello()
                    ElseIf line.Equals("O") Then
                        batchOneway.sayHello()
                    ElseIf line.Equals("d") Then
                        If secure Then
                            Console.WriteLine("secure datagrams are not supported")
                        Else
                            datagram.sayHello()
                        End If
                    ElseIf line.Equals("D") Then
                        If secure Then
                            Console.WriteLine("secure datagrams are not supported")
                        Else
                            batchDatagram.sayHello()
                        End If
                    ElseIf line.Equals("f") Then
                        communicator.flushBatchRequests()
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
