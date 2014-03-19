' **********************************************************************
'
' Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports BidirDemo
Imports System.Collections

Class CallbackSenderI
    Inherits CallbackSenderDisp_

    Public Sub New(ByVal communicator As Ice.Communicator)
        _communicator = communicator
        _destroy = False
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
        Dim num As Integer = 0
        While True
            Dim clients As ArrayList
            SyncLock Me
                System.Threading.Monitor.Wait(Me, 2000)
                If _destroy Then
                    Exit While
                End If
                clients = New ArrayList(_clients)
            End SyncLock

            If clients.Count > 0 Then
                num += 1

                For Each c As CallbackReceiverPrx In clients
                    Try
                        c.callback(num)
                    Catch ex As Ice.LocalException
                        System.Console.Out.WriteLine("removing client `" & _communicator.identityToString(c.ice_getIdentity()) & "'")
                        SyncLock Me
                            _clients.Remove(c)
                        End SyncLock
                    End Try
                Next
            End If
        End While
    End Sub

    Private _communicator As Ice.Communicator
    Private _destroy As Boolean
    Private _clients As ArrayList

End Class
