' **********************************************************************
'
' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports IcePackSimpleDemo

Public Module IcePackSimpleClient

    Private Sub menu()

        Console.WriteLine("usage:")
        Console.WriteLine("t: send greeting as twoway")
        Console.WriteLine("o: send greeting as oneway")
        Console.WriteLine("O: send greeting as batch oneway")
        Console.WriteLine("d: send greeting as datagram")
        Console.WriteLine("D: send greeting as batch datagram")
        Console.WriteLine("f: flush all batch requests")
        Console.WriteLine("T: set a timeout")
        Console.WriteLine("s: shutdown server")
        Console.WriteLine("x: exit")
        Console.WriteLine("?: help")
    End Sub

    Private Function run(ByVal args As String(), ByVal communicator As Ice.Communicator) As Integer
        Dim properties As Ice.Properties = communicator.getProperties()

        Dim query As IcePack.QueryPrx = IcePack.QueryPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Query"))

        Dim base As Ice.ObjectPrx = Nothing
        Try
            base = query.findObjectByType("::IcePackSimpleDemo::Hello")
        Catch ex As IcePack.ObjectNotExistException
        End Try
        Dim twoway As HelloPrx = HelloPrxHelper.checkedCast(base.ice_twoway().ice_timeout(-1).ice_secure(False))
        If twoway Is Nothing Then
            Console.WriteLine("invalid proxy")
            Return 1
        End If
        Dim oneway As HelloPrx = HelloPrxHelper.uncheckedCast(twoway.ice_oneway())
        Dim batchoneway As HelloPrx = HelloPrxHelper.uncheckedCast(twoway.ice_batchOneway())
        Dim datagram As HelloPrx = HelloPrxHelper.uncheckedCast(twoway.ice_datagram())
        Dim batchDatagram As HelloPrx = HelloPrxHelper.uncheckedCast(twoway.ice_batchDatagram())

        Dim timeout As Integer = -1

        menu()

        Dim line As String = Nothing
        Do
            Try
                Console.Write("==> ")
                Console.Out.Flush()
                line = Console.In.ReadLine()
                If line Is Nothing Then
                    Exit Try
                End If
                If line.Equals("t") Then
                    twoway.sayHello()
                ElseIf line.Equals("o") Then
                    oneway.sayHello()
                ElseIf line.Equals("O") Then
                    batchoneway.sayHello()
                ElseIf line.Equals("d") Then
                    datagram.sayHello()
                ElseIf line.Equals("D") Then
                    batchDatagram.sayHello()
                ElseIf line.Equals("f") Then
                    communicator.flushBatchRequests()
                ElseIf line.Equals("T") Then
                    If timeout = -1 Then
                        timeout = 2000
                    Else
                        timeout = -1
                    End If

                    twoway = HelloPrxHelper.uncheckedCast(twoway.ice_timeout(timeout))
                    oneway = HelloPrxHelper.uncheckedCast(oneway.ice_timeout(timeout))
                    batchoneway = HelloPrxHelper.uncheckedCast(batchoneway.ice_timeout(timeout))

                    If timeout = -1 Then
                        Console.WriteLine("timeout is now switched off")
                    Else
                        Console.WriteLine("timeout is now set to 2000ms")
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
            Catch ex As Ice.LocalException
                Console.WriteLine(ex)
            End Try
        Loop While Not line.Equals("x")

        Return 0
    End Function

    Public Sub Main(ByVal args As String())
        Dim status As Integer = 0
        Dim communicator As Ice.Communicator = Nothing

        Try
            Dim properties As Ice.Properties = Ice.Util.createProperties()
            properties.load("config")
            communicator = Ice.Util.initializeWithProperties(args, properties)
            status = run(args, communicator)
        Catch ex As Ice.LocalException
            Console.WriteLine(ex)
            status = 1
        End Try

        If Not communicator Is Nothing Then
            Try
                communicator.destroy()
            Catch ex As Ice.LocalException
                Console.WriteLine(ex)
                status = 1
            End Try
        End If

        Environment.Exit(status)
    End Sub

End Module
