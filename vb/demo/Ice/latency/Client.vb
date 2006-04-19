' **********************************************************************
'
' Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports Demo

Module LatencyC

    Class Client
        Inherits Ice.Application

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim properties As Ice.Properties = communicator().getProperties()
            Dim refProperty As String = "Latency.Ping"
            Dim ref As String = properties.getProperty(refProperty)
            If ref.Length = 0 Then
                Console.Error.WriteLine("property `" & refProperty & "' not set")
                Return 1
            End If

            Dim ping As PingPrx = PingPrxHelper.checkedCast(communicator().stringToProxy(ref))
            If ping Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                Return 1
            End If

            '  Initial ping to setup the connection.
            ping.ice_ping()

            Dim tv1 As Long = (System.DateTime.Now.Ticks - 621355968000000000) / 10000
            Dim repetitions As Integer = 100000
            Console.Out.WriteLine("pinging server " & repetitions & " times (this may take a while)")
            For i As Integer = 0 To repetitions - 1
                ping.ice_ping()
            Next

            Dim tv2 As Long = (System.DateTime.Now.Ticks - 621355968000000000) / 10000
            Dim total As Double = tv2 - tv1
            Dim perPing As Double = total / repetitions

            Console.Out.WriteLine("time for " & repetitions & " pings: " & total & "ms")
            Console.Out.WriteLine("time per ping: " & perPing & "ms")

            Return 0
        End Function
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Client = New Client
        Dim status As Integer = app.main(args, "config.client")
        System.Environment.Exit(status)
    End Sub

End Module
