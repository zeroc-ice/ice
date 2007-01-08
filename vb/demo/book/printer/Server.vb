' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports printerDemo.Demo

Public Class PrinterI
    Inherits PrinterDisp_

    Public Overloads Overrides Sub printString(ByVal s As String, ByVal current As Ice.Current)
        Console.WriteLine(s)
    End Sub

End Class

Module Server

    Public Sub Main(ByVal args() As String)

        Dim status As Integer = 0
        Dim ic As Ice.Communicator = Nothing
        Try
            ic = Ice.Util.initialize(args)
            Dim adapter As Ice.ObjectAdapter = ic.createObjectAdapterWithEndpoints("SimplePrinterAdapter", "default -p 10000")
            Dim obj As Ice.Object = New PrinterI
            adapter.add(obj, ic.stringToIdentity("SimplePrinter"))
            adapter.activate()
            ic.waitForShutdown()
        Catch e As Exception
            Console.Error.WriteLine(e)
            status = 1
        End Try
        If Not ic Is Nothing Then
            ' Clean up
            '
            Try
                ic.destroy()
            Catch e As Exception
                Console.Error.WriteLine(e)
                status = 1
            End Try
        End If
        Environment.Exit(status)
    End Sub

End module
