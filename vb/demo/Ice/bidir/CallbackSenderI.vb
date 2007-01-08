' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports BidirDemo
Imports System.Collections

Class CallbackSenderI
    Inherits CallbackSenderDisp_

    Public Sub New()
        _destroy = False
        _num = 0
        _clients = New ArrayList
    End Sub

    Public Sub destroy()
        SyncLock Me
            System.Console.Out.WriteLine("destroying callback sender")
            _destroy = True

            System.Threading.Monitor.Pulse(Me)
        End SyncLock
    End Sub

    Public Overloads Overrides Sub addClient(ByVal ident As Ice.Identity, ByVal current As Ice.Current)
        SyncLock Me
            System.Console.Out.WriteLine("adding client `" & current.adapter.getCommunicator().identityToString(ident) & "'")

            Dim base As Ice.ObjectPrx = current.con.createProxy(ident)
            Dim client As CallbackReceiverPrx = CallbackReceiverPrxHelper.uncheckedCast(base)
            _clients.Add(client)
        End SyncLock
    End Sub

    Public Sub Run()
        SyncLock Me
            While Not _destroy
                System.Threading.Monitor.Wait(Me, 2000)

                If Not _destroy AndAlso _clients.Count <> 0 Then

                    _num += 1

                    Dim toRemove As ArrayList = New ArrayList
                    For Each c As CallbackReceiverPrx In _clients
                        Try
                            c.callback(_num)
                        Catch ex As Ice.LocalException
                            toRemove.Add(c)
                        End Try
                    Next
                    For Each c As CallbackReceiverPrx In toRemove
                        _clients.Remove(c)
                    Next
                End If
            End While
        End SyncLock
    End Sub

    Private _destroy As Boolean
    Private _num As Integer
    Private _clients As ArrayList

End Class
