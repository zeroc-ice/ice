' **********************************************************************
'
' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System.Collections

Module ClockS

    Class Subscriber
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

	    Dim topics As ArrayList = New ArrayList
	    If args.Length > 0 Then
	        For i As Integer = 0 To args.Length - 1
		    topics.Add(args(i))
		Next
	    Else
	        topics.Add("time")
	    End If

	    Dim qos As IceStorm.Qos = New IceStorm.Qos
	    qos.Add("reliability", "batch")

	    Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Clock.Subscriber")
	    Dim clock As Ice.Object = New ClockI

	    Dim subscribers As System.Collections.Hashtable = New System.Collections.Hashtable

	    For i As Integer = 0 To topics.Count - 1
	        Dim obj As Ice.ObjectPrx = adapter.addWithUUID(clock)
		Try
		    Dim topic As IceStorm.TopicPrx = manager.retrieve(topics(i))
		    topic.subscribe(qos, obj)
		Catch ex As IceStorm.NoSuchTopic
		    Console.Error.WriteLine(ex)
		End Try

		subscribers.Add(topics(i), obj)
	    Next

	    If subscribers.Count = topics.Count Then
	        adapter.activate()
		shutdownOnInterrupt()
		communicator().waitForShutdown()
	    End If

	    For Each entry As DictionaryEntry in subscribers
	        Try
		    Dim topic As IceStorm.TopicPrx = manager.retrieve(entry.Key)
		    topic.unsubscribe(entry.Value)
		Catch ex As IceStorm.NoSuchTopic
		    Console.Error.WriteLine(ex)
		End Try
	    Next entry

            Return 0
        End Function
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Subscriber = New Subscriber
        Dim status As Integer = app.Main(args, "config")
        System.Environment.Exit(status)
    End Sub
End Module
