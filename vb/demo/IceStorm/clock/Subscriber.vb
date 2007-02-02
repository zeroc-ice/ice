' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System
Imports System.Collections
Imports Demo

Public Class ClockI
    Inherits ClockDisp_

    Public Overloads Overrides Sub tick(ByVal time as String, ByVal current As Ice.Current)
        System.Console.Out.WriteLine(time)
    End Sub
End Class

Module ClockS


    Class Subscriber
        Inherits Ice.Application

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim basePrx As Ice.ObjectPrx = communicator().propertyToProxy("IceStorm.TopicManager.Proxy")
            Dim manager As IceStorm.TopicManagerPrx = IceStorm.TopicManagerPrxHelper.checkedCast(basePrx)
            If manager Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                Return 1
            End If

	    Dim topicName as String = "time"
	    Dim datagram as Boolean = false
	    Dim twoway as Boolean = false
	    Dim ordered as Boolean = false
	    Dim batch as Boolean = false
	    Dim optsSet as Integer = 0
	    For i As Integer = 0 To args.Length -1
		If args(i).Equals("--datagram") Then
		    datagram = true
		    optsSet = optsSet + 1
		Elseif args(i).Equals("--twoway") Then
		    twoway = true
		    optsSet = optsSet + 1
		Elseif args(i).Equals("--ordered") Then
		    ordered = true
		    optsSet = optsSet + 1
		Elseif args(i).Equals("--oneway") Then
		    optsSet = optsSet + 1
		Elseif args(i).Equals("--batch") Then
		    batch = true
		Elseif args(i).StartsWith("--") Then
		    usage()
		    Return 1
		Else
		    topicName = args(i)
		    Exit For
		End if
    	    Next

	    If batch and (twoway or ordered) Then
		Console.WriteLine(appName() + ": batch can only be set with oneway or datagram")
		Return 1
	    End if

	    If optsSet > 1 Then
		usage()
		Return 1
	    End If

            Dim topic As IceStorm.TopicPrx 
            Try
                topic = manager.retrieve(topicName)
            Catch ex As IceStorm.NoSuchTopic
                Try
                    topic = manager.create(topicName)
                Catch e As IceStorm.TopicExists
                    Console.Error.WriteLine("temporary error. try again.")
                    Return 1
                End Try
            End Try
            
            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Clock.Subscriber")

            Dim subscriber As Ice.ObjectPrx = adapter.addWithUUID(New ClockI)

            Dim qos As IceStorm.Qos = New IceStorm.Qos

	    '
	    ' Set up the proxy.
	    '
	    If datagram Then
		subscriber = subscriber.ice_datagram()
	    Elseif twoway Then
		' Do nothing to the subscriber proxy. Its already twoway.
	    Elseif ordered Then
		' Do nothing to the subscriber proxy. Its already twoway.
		qos.Add("reliability", "ordered")
	    Else ' if oneway
		subscriber = subscriber.ice_oneway()
    	    End If
	    If batch Then
		if datagram Then
		    subscriber = subscriber.ice_batchDatagram()
		Else
		    subscriber = subscriber.ice_batchOneway()
    	    	End If
    	    End If

	    topic.subscribeAndGetPublisher(qos, subscriber)
            adapter.activate()

            shutdownOnInterrupt()
            communicator().waitForShutdown()

            topic.unsubscribe(subscriber)

            Return 0
        End Function

	Public Sub usage
	    Console.WriteLine("Usage: " + appName() + " [--batch] [--datagram|--twoway|--ordered|--oneway] [topic]")
	End Sub

    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Subscriber = New Subscriber
        Dim status As Integer = app.Main(args, "config.sub")
        System.Environment.Exit(status)
    End Sub
End Module
