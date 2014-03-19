' **********************************************************************
'
' Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
            If args.Length > 0 Then
                Console.Error.WriteLine(appName() & ": too many arguments")
                Return 1
            End If

            Dim ping As PingPrx = PingPrxHelper.checkedCast(communicator().propertyToProxy("Latency.Ping"))
            If ping Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                Return 1
            End If

            '
            ' A method needs to be invoked thousands of times before the JIT compiler
            ' will convert it to native code. To ensure an accurate throughput measurement,
            ' we need to "warm up" the JIT compiler.
            '
            Dim repetitions As Integer = 20000
            Console.Out.Write("warming up the JIT compiler...")
            Console.Out.Flush()
            For i As Integer = 0 To repetitions - 1
                ping.ice_ping()
            Next
            Console.Out.WriteLine("ok")
            ping.ice_ping()

            Dim tv1 As Long = (System.DateTime.Now.Ticks - 621355968000000000) / 10000
            repetitions = 100000
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
