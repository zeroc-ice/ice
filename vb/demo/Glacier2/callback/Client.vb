' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports glacier2demo.Demo
Imports System
Imports System.Collections

Module Glacier2callbackC
    Class Client
        Inherits Ice.Application

        Private Sub menu()
            Console.Out.WriteLine("usage:")
            Console.Out.WriteLine("t: send callback as twoway")
            Console.Out.WriteLine("o: send callback as oneway")
            Console.Out.WriteLine("O: send callback as batch oneway")
            Console.Out.WriteLine("f: flush all batch requests")
            Console.Out.WriteLine("v: set/reset override context field")
            Console.Out.WriteLine("F: set/reset fake category")
            Console.Out.WriteLine("s: shutdown server")
            Console.Out.WriteLine("x: exit")
            Console.Out.WriteLine("?: help")
        End Sub

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim defaultRouter As Ice.RouterPrx = communicator().getDefaultRouter()
            If defaultRouter Is Nothing Then
                Console.Error.WriteLine("no default router set")
                Return 1
            End If

            Dim router As Glacier2.RouterPrx = Glacier2.RouterPrxHelper.checkedCast(defaultRouter)
            If router Is Nothing Then
                Console.Error.WriteLine("configured router is not a Glacier2 router")
                Return 1
            End If

            While True
                Console.WriteLine("This demo accepts any user-id / password combination.")

                Dim id As String
                Console.Write("user id: ")
                Console.Out.Flush()
                id = Console.In.ReadLine()

                Dim pw As String
                Console.Write("password: ")
                Console.Out.Flush()
                pw = Console.In.ReadLine()

                Try
                    router.createSession(id, pw)
                    Exit While
                Catch ex As Glacier2.PermissionDeniedException
                    Console.Write("permission denied:\n" & ex.reason)
                Catch ex As Glacier2.CannotCreateSessionException
                    Console.Write("cannot create session:\n" & ex.reason)
                End Try

            End While

            Dim category As String = router.getCategoryForClient()
            Dim callbackReceiverIdent As Ice.Identity = New Ice.Identity
            callbackReceiverIdent.name = "callbackReceiver"
            callbackReceiverIdent.category = category
            Dim callbackReceiverFakeIdent As Ice.Identity = New Ice.Identity
            callbackReceiverFakeIdent.name = "callbackReceiver"
            callbackReceiverFakeIdent.category = "fake"

            Dim base As Ice.ObjectPrx = communicator().propertyToProxy("Callback.Proxy")
            Dim twoway As CallbackPrx = CallbackPrxHelper.checkedCast(base)
            Dim oneway As CallbackPrx = CallbackPrxHelper.uncheckedCast(twoway.ice_oneway())
            Dim batchOneway As CallbackPrx = CallbackPrxHelper.uncheckedCast(twoway.ice_batchOneway())

            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Callback.Client")
            adapter.add(New CallbackReceiverI, callbackReceiverIdent)
            adapter.add(New CallbackReceiverI, callbackReceiverFakeIdent)
            adapter.activate()

            Dim twowayR As CallbackReceiverPrx = CallbackReceiverPrxHelper.uncheckedCast(adapter.createProxy(callbackReceiverIdent))
            Dim onewayR As CallbackReceiverPrx = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_oneway())

            menu()

            Dim line As String = Nothing
            Dim override As String = Nothing
            Dim fake As Boolean = False
            Do
                Try
                    Console.Write("==> ")
                    Console.Out.Flush()
                    line = Console.In.ReadLine()
                    If line Is Nothing Then
                        Exit Do
                    End If
                    If line.Equals("t") Then
                        Dim context As Ice.Context = New Ice.Context
                        context("_fwd") = "t"
                        If Not override Is Nothing Then
                            context("_ovrd") = override
                        End If
                        twoway.initiateCallback(twowayR, context)
                    ElseIf line.Equals("o") Then
                        Dim context As Ice.Context = New Ice.Context
                        context("_fwd") = "o"
                        If override Is Nothing Then
                            context("_ovrd") = override
                        End If
                        oneway.initiateCallback(onewayR, context)
                    ElseIf line.Equals("O") Then
                        Dim context As Ice.Context = New Ice.Context
                        context("_fwd") = "O"
                        If Not override Is Nothing Then
                            context("_ovrd") = override
                        End If
                        batchOneway.initiateCallback(onewayR, context)
                    ElseIf line.Equals("f") Then
                        communicator().flushBatchRequests()
                    ElseIf line.Equals("v") Then
                        If override Is Nothing Then
                            override = "some_value"
                            Console.WriteLine("override context field is now `" & override & "'")
                        Else
                            override = Nothing
                            Console.WriteLine("override context field is empty")
                        End If
                    ElseIf line.Equals("F") Then
                        fake = Not fake

                        If fake Then
                            twowayR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_identity(callbackReceiverFakeIdent))
                            onewayR = CallbackReceiverPrxHelper.uncheckedCast(onewayR.ice_identity(callbackReceiverFakeIdent))
                        Else
                            twowayR = CallbackReceiverPrxHelper.uncheckedCast(twowayR.ice_identity(callbackReceiverIdent))
                            onewayR = CallbackReceiverPrxHelper.uncheckedCast(onewayR.ice_identity(callbackReceiverIdent))
                        End If

                        Console.WriteLine("callback receiver identity: " & communicator().identityToString(twowayR.ice_getIdentity()))
                    ElseIf line.Equals("s") Then
                        twoway.shutdown()
                    ElseIf line.Equals("x") Then
                        '  Nothing to do
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

    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Client = New Client
        Dim status As Integer = app.main(args, "config.client")
        System.Environment.Exit(status)
    End Sub

End Module
