' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Module AsyncS

    Class Server
        Inherits Ice.Application

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            callbackOnInterrupt()

            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Hello")
            _workQueue = New WorkQueue
            adapter.add(New HelloI(_workQueue), communicator().stringToIdentity("hello"))

            _workQueue.Start()
            adapter.activate()

            communicator().waitForShutdown()
            _workQueue.Join()
            Return 0
        End Function

        Public Overloads Overrides Sub interruptCallback(ByVal sig As Integer)
            _workQueue.destroy()
            communicator().shutdown()
        End Sub

        Private _workQueue As WorkQueue
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Server = New Server
        Dim status As Integer = app.Main(args, "config.server")
        System.Environment.Exit(status)
    End Sub
End Module
