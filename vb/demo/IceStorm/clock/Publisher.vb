' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

            Dim basePrx As Ice.ObjectPrx = communicator().propertyToProxy("IceStorm.TopicManager.Proxy")
            Dim manager As IceStorm.TopicManagerPrx = IceStorm.TopicManagerPrxHelper.checkedCast(basePrx)
            If manager Is Nothing Then
                Console.Error.WriteLine("invalid proxy")
                Return 1
            End If

	    Dim topicName as String = "time"
	    Dim datagram as Boolean = false
	    Dim twoway as Boolean = false
	    Dim optsSet as Integer = 0
	    For i As Integer = 0 To args.Length -1
		If args(i).Equals("--datagram") Then
		    datagram = true
		    optsSet = optsSet + 1
		Elseif args(i).Equals("--twoway") Then
		    twoway = true
		    optsSet = optsSet + 1
		Elseif args(i).Equals("--oneway") Then
		    optsSet = optsSet + 1
		Elseif args(i).StartsWith("--") Then
		    usage()
		    Return 1
		Else
		    topicName = args(i)
		    Exit For
		End if
    	    Next

	    If optsSet > 1 Then
		usage()
		Return 1
	    End If

	    '
	    ' Retrieve the topic.
	    '
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

	    '
	    ' Get the topic's publisher object, and create a Clock proxy with
	    ' the mode specified as an argument of this application.
	    '
	    Dim publisher as Ice.ObjectPrx = topic.getPublisher()
	    If datagram Then
		publisher = publisher.ice_datagram()
	    Elseif twoway Then
		' Do nothing.
	    Else  ' if oneway
		publisher = publisher.ice_oneway()
	    End If
            Dim clock As ClockPrx = ClockPrxHelper.uncheckedCast(publisher)

            Console.Out.WriteLine("publishing tick events. Press ^C to terminate the application.")
            Try
                While 1
                    clock.tick(DateTime.Now.ToString("MM/dd/yyyy HH:mm:ss"))

                    System.Threading.Thread.Sleep(1000)
                End While
            Catch ex As Ice.CommunicatorDestroyedException
            End Try

            Return 0
        End Function

	Public Sub usage
	    Console.WriteLine("Usage: " + appName() + " [--datagram|--twoway|--oneway] [topic]")
	End Sub
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Publisher = New Publisher
        Dim status As Integer = app.Main(args, "config.pub")
        System.Environment.Exit(status)
    End Sub
End Module
