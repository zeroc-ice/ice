' **********************************************************************
'
' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports IcePackHelloDemo

Public Module IcePackHelloClient

    Private Sub menu()
        Console.WriteLine("usage:")
        Console.WriteLine("c: create a hello object")
        Console.WriteLine("d: destroy the current hello object")
        Console.WriteLine("s: set the current hello object")
        Console.WriteLine("r: set the current hello object to a random hello object")
        Console.WriteLine("S: show the name of the current hello object")
        Console.WriteLine("t: send greeting")
        Console.WriteLine("x: exit")
        Console.WriteLine("?: help")
    End Sub

    Private Function run(ByVal args As String(), ByVal communicator As Ice.Communicator) As Integer
        Dim properties As Ice.Properties = communicator.getProperties()

        Dim query As IcePack.QueryPrx = IcePack.QueryPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Query"))

        '
        ' Get an object implementing the HelloFactory interface.
        '
        Dim factory As HelloFactoryPrx = HelloFactoryPrxHelper.checkedCast(query.findObjectByType("::IcePackHelloDemo::HelloFactory"))

        '
        ' By default we create a Hello object named 'Foo'.
        '
        Dim hello As HelloPrx
        Try
            hello = factory.find("Foo")
        Catch ex As NameNotExistException
            hello = factory.create("Foo")
        End Try

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
                    hello.sayHello()
                ElseIf (line.Equals("c")) Then
                    Console.Write("name: ")
                    Console.Out.Flush()
                    Dim name As String = Console.In.ReadLine()
                    If Not name Is Nothing AndAlso name.Length > 0 Then
                        Try
                            hello = factory.find(name)
                            Console.WriteLine("Hello object named '" & name & "' already exists")
                        Catch ex As NameNotExistException
                            factory = HelloFactoryPrxHelper.checkedCast(query.findObjectByType("::IcePackHelloDemo::HelloFactory"))
                            hello = factory.create(name)
                        End Try
                    End If
                ElseIf line.Equals("d") Then
                    If Ice.Util.identityToString(hello.ice_getIdentity()).Equals("Foo") Then
                        Console.WriteLine("Can't delete the default Hello object named 'Foo'")
                    Else
                        hello.destroy()
                        Try
                            hello = factory.find("Foo")
                        Catch ex As NameNotExistException
                            hello = factory.create("Foo")
                        End Try
                    End If
                ElseIf line.Equals("s") Then
                    Console.Write("name: ")
                    Console.Out.Flush()
                    Dim name As String = Console.In.ReadLine()
                    If Not name Is Nothing AndAlso name.Length > 0 Then
                        Try
                            hello = HelloPrxHelper.checkedCast(factory.find(name))
                        Catch ex As NameNotExistException
                            Console.WriteLine("This name doesn't exist")
                        End Try
                    End If
                ElseIf line.Equals("r") Then
                    hello = HelloPrxHelper.checkedCast(query.findObjectByType("::IcePackHelloDemo::Hello"))

                ElseIf line.Equals("S") Then
                    Console.WriteLine(Ice.Util.identityToString(hello.ice_getIdentity()))
                ElseIf line.Equals("x") Then
                    ' Nothing to do
                ElseIf line.Equals("?") Then
                    menu()
                Else
                    Console.WriteLine("unknown command `" & line & "'")
                    menu()
                End If
            Catch ex As Ice.LocalException
                Console.Error.WriteLine(ex)
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
        Catch ex As Exception
            Console.Error.WriteLine(ex)
            status = 1
        End Try

        If Not communicator Is Nothing Then
            Try
                communicator.destroy()
            Catch ex As Ice.LocalException
                Console.Error.WriteLine(ex)
                status = 1
            End Try
        End If

        Environment.Exit(status)
    End Sub

End Module
