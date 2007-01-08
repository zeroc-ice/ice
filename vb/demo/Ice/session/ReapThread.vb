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

Public Class ReapThread

    Public Class SessionProxyPair
        Public Sub New(ByVal p As SessionPrx, ByVal s As SessionI)
            proxy = p
            session = s
        End Sub

        Public proxy As SessionPrx
        Public session As SessionI
    End Class

    Public Sub New()
        _timeout = System.TimeSpan.FromSeconds(10)
        _terminated = False
        _sessions = New ArrayList
    End Sub

    Public Sub run()
        SyncLock Me
            While Not _terminated
                System.Threading.Monitor.Wait(Me, _timeout)
                If Not _terminated Then
                    Dim tmp As ArrayList = New ArrayList
                    For Each p As SessionProxyPair In _sessions
                        Try
                            '
                            ' Session destruction may take time in a
                            ' real-world example. Therefore the current time
                            ' is computed for each iteration.
                            '
                            If System.TimeSpan.Compare(System.DateTime.Now.Subtract(p.session.timestamp()), _timeout) > 0 Then
                                Dim name As String = p.proxy.getName()
                                p.proxy.destroy()
                                Console.Out.Write("The session " + name)
                                Console.Out.WriteLine(" has timed out.")
                            Else
                                tmp.Add(p)
                            End If
                        Catch e As Ice.ObjectNotExistException
                            ' Ignore.
                        End Try
                        _sessions = tmp
                    Next
                End If
            End While
        End SyncLock
    End Sub

    Public Sub terminate()
        SyncLock Me
            _terminated = True
            System.Threading.Monitor.Pulse(Me)

            _sessions.Clear()
        End SyncLock
    End Sub

    Public Sub add(ByVal proxy As SessionPrx, ByVal session As SessionI)
        SyncLock Me
            _sessions.Add(New SessionProxyPair(proxy, session))
        End SyncLock
    End Sub

    Private _terminated As Boolean
    Private _timeout As System.TimeSpan
    Private _sessions As ArrayList
End Class
