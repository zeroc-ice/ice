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

Module ClockC

    Class Publisher
        Inherits Ice.Application

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim properties As Ice.Properties = communicator().getProperties()
            Dim proxyProperty As String = "IceStorm.TopicManager.Proxy"
            Dim proxy As String = properties.getProperty(proxyProperty)
            If proxy.Length = 0 Then
                Console.Error.WriteLine("property `" & proxyProperty & "' not set")
                Return 1
            End If

            Dim basePrx As Ice.ObjectPrx = communicator().stringToProxy(proxy)
            Dim manager As IceStorm.TopicManagerPrx = IceStorm.TopicManagerPrxHelper.checkedCast(basePrx)
            If manager Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                Return 1
            End If

            Dim topic As IceStorm.TopicPrx
	    Try
	        topic = manager.retrieve("time")
            Catch ex As IceStorm.NoSuchTopic
                Console.Error.WriteLine(ex)
		Return 1
            End Try

	    Dim obj As Ice.ObjectPrx = topic.getPublisher()
	    If Not obj.ice_isDatagram() Then
	        obj = obj.ice_oneway()
	    End If
	    Dim clock As ClockPrx = ClockPrxHelper.uncheckedCast(obj)

	    Console.Out.WriteLine("publishing 10 tick events")
	    For i As Integer = 1 To 10
	        clock.tick()
	    Next

            Return 0
        End Function
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Publisher = New Publisher
        Dim status As Integer = app.Main(args, "config.pub")
        System.Environment.Exit(status)
    End Sub
End Module
