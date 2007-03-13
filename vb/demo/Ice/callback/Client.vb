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

Module CallbackC
    Class Client
        Inherits Ice.Application

        Private Sub menu()
            Console.Out.WriteLine("usage:")
            Console.Out.WriteLine("t: send callback as twoway")
            Console.Out.WriteLine("o: send callback as oneway")
            Console.Out.WriteLine("O: send callback as batch oneway")
            Console.Out.WriteLine("d: send callback as datagram")
            Console.Out.WriteLine("D: send callback as batch datagram")
            Console.Out.WriteLine("f: flush all batch requests")
            If _haveSSL Then
                Console.Out.WriteLine("S: switch secure mode on/off")
            End If
            Console.Out.WriteLine("s: shutdown server")
            Console.Out.WriteLine("x: exit")
            Console.Out.WriteLine("?: help")
        End Sub

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Try
                communicator().getPluginManager().getPlugin("IceSSL")
                _haveSSL = True
            Catch ex As Ice.NotRegisteredException
            End Try

            Dim twoway As CallbackSenderPrx = CallbackSenderPrxHelper.checkedCast(communicator().propertyToProxy("Callback.CallbackServer").ice_twoway().ice_timeout(-1).ice_secure(False))
            If twoway Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                Return 1
            End If
            Dim oneway As CallbackSenderPrx = CallbackSenderPrxHelper.uncheckedCast(twoway.ice_oneway())
            Dim batchOneway As CallbackSenderPrx = CallbackSenderPrxHelper.uncheckedCast(twoway.ice_batchOneway())
            Dim datagram As CallbackSenderPrx = CallbackSenderPrxHelper.uncheckedCast(twoway.ice_datagram())
            Dim batchDatagram As CallbackSenderPrx = CallbackSenderPrxHelper.uncheckedCast(twoway.ice_batchDatagram())

            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Callback.Client")
            adapter.add(New CallbackReceiverI, communicator().stringToIdentity("callbackReceiver"))
            adapter.activate()

            Dim twowayR As CallbackReceiverPrx = CallbackReceiverPrxHelper.uncheckedCast( _
                adapter.createProxy(communicator().stringToIdentity("callbackReceiver")))
            Dim onewayR As CallbackReceiverPrx = CallbackReceiverPrxHelper.uncheckedCast( _
                twowayR.ice_oneway())
            Dim datagramR As CallbackReceiverPrx = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_datagram())

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
                        twoway.initiateCallback(twowayR)
                    ElseIf line.Equals("o") Then
                        oneway.initiateCallback(onewayR)
                    ElseIf line.Equals("O") Then
                        batchOneway.initiateCallback(onewayR)
                    ElseIf line.Equals("d") Then
                        If secure Then
                            Console.WriteLine("secure datagrams are not supported")
                        Else
                            datagram.initiateCallback(datagramR)
                        End If
                    ElseIf line.Equals("D") Then
                        If secure Then
                            Console.WriteLine("secure datagrams are not supported")
                        Else
                            batchDatagram.initiateCallback(datagramR)
                        End If
                    ElseIf line.Equals("f") Then
                        communicator().flushBatchRequests()
                    ElseIf _haveSSL And line.Equals("S") Then
                        secure = Not secure

                        twoway = CallbackSenderPrxHelper.uncheckedCast(twoway.ice_secure(secure))
                        oneway = CallbackSenderPrxHelper.uncheckedCast(oneway.ice_secure(secure))
                        batchOneway = CallbackSenderPrxHelper.uncheckedCast(batchOneway.ice_secure(secure))
                        datagram = CallbackSenderPrxHelper.uncheckedCast(datagram.ice_secure(secure))
                        batchDatagram = CallbackSenderPrxHelper.uncheckedCast(batchDatagram.ice_secure(secure))

                        twowayR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_secure(secure))
                        onewayR = CallbackReceiverPrxHelper.uncheckedCast(onewayR.ice_secure(secure))
                        datagramR = CallbackReceiverPrxHelper.uncheckedCast(datagramR.ice_secure(secure))

                        If secure Then
                            Console.Out.WriteLine("secure mode is now on")
                        Else
                            Console.Out.WriteLine("secure mode is now off")
                        End If
                    ElseIf line.Equals("s") Then
                        twoway.shutdown()
                    ElseIf line.Equals("x") Then
                        '  Nothing to do
                    ElseIf line.Equals("?") Then
                        menu()
                    Else
                        Console.Out.WriteLine("unknown command `" & line & "'")
                        menu()
                    End If
                Catch ex As System.Exception
                    Console.Error.WriteLine(ex)
                End Try
            Loop While Not line.Equals("x")

            Return 0
        End Function

        Private Shared _haveSSL As Boolean = False

    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Client = New Client
        Dim status As Integer = app.main(args, "config.client")
        System.Environment.Exit(status)
    End Sub

End Module
