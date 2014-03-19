' **********************************************************************
'
' Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Demo
Imports System
Imports System.Threading
Imports System.Collections.Generic

Module Glacier2callbackC
    Class Client
        Inherits Glacier2.Application

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

        Public Overloads Overrides Function createSession() As Glacier2.SessionPrx
            Dim session As Glacier2.SessionPrx = Nothing
            While True
                Console.Out.WriteLine("This demo accepts any user-id / password combination.")

                Dim id as String
                Dim pw as String
                Try
                    Console.Out.Write("user id: ")
                    Console.Out.Flush()
                    id = Console.In.ReadLine()
                    If id Is Nothing Then
                        throw New Ice.CommunicatorDestroyedException()
                    End If
                    id = id.Trim()

                    Console.Out.Write("password: ")
                    Console.Out.Flush()
                    pw = Console.In.ReadLine()
                    If pw Is Nothing Then
                        throw New Ice.CommunicatorDestroyedException()
                    End If
                    pw = pw.Trim()
                Catch ex As System.IO.IOException
                    Console.Out.WriteLine(ex.StackTrace.ToString())
                    Continue While
                End try

                Try
                    session = router().createSession(id, pw)
                    Exit While
                Catch ex As Glacier2.PermissionDeniedException
                    Console.Out.WriteLine("permission denied:\n" + ex.reason)
                Catch ex As Glacier2.CannotCreateSessionException
                    Console.Out.WriteLine("cannot create session:\n" + ex.reason)
                End Try
            End While

            Return session
        End Function


        Public Overloads Overrides Function runWithSession(ByVal args() As String) As Integer
            If args.Length > 0 Then
                Console.Error.WriteLine(appName() & ": too many arguments")
                Return 1
            End If

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


            Dim callbackReceiverIdent As Ice.Identity
            callbackReceiverIdent = createCallbackIdentity("callbackReceiver")

            Dim callbackReceiverFakeIdent As Ice.Identity = New Ice.Identity
            callbackReceiverFakeIdent.name = "callbackReceiver"
            callbackReceiverFakeIdent.category = "fake"

            Dim base As Ice.ObjectPrx = communicator().propertyToProxy("Callback.Proxy")
            Dim twoway As CallbackPrx = CallbackPrxHelper.checkedCast(base)
            Dim oneway As CallbackPrx = CallbackPrxHelper.uncheckedCast(twoway.ice_oneway())
            Dim batchOneway As CallbackPrx = CallbackPrxHelper.uncheckedCast(twoway.ice_batchOneway())

            objectAdapter().add(New CallbackReceiverI, callbackReceiverIdent)
            objectAdapter().add(New CallbackReceiverI, callbackReceiverFakeIdent)
            objectAdapter().activate()

            Dim twowayR As CallbackReceiverPrx = CallbackReceiverPrxHelper.uncheckedCast(objectAdapter().createProxy(callbackReceiverIdent))
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
                        Dim context As Dictionary(Of String, String) = New Dictionary(Of String, String)()
                        context("_fwd") = "t"
                        If Not override Is Nothing Then
                            context("_ovrd") = override
                        End If
                        twoway.initiateCallback(twowayR, context)
                    ElseIf line.Equals("o") Then
                        Dim context As Dictionary(Of String, String) = New Dictionary(Of String, String)()
                        context("_fwd") = "o"
                        If Not override Is Nothing Then
                            context("_ovrd") = override
                        End If
                        oneway.initiateCallback(onewayR, context)
                    ElseIf line.Equals("O") Then
                        Dim context As Dictionary(Of String, String) = New Dictionary(Of String, String)()
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
