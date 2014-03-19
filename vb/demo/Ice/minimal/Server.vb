' **********************************************************************
'
' Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Module MinimalS

    Public Sub Main(ByVal args() As String)
        Try
            Dim communicator As Ice.Communicator = Ice.Util.initialize(args)
            Dim adapter As Ice.ObjectAdapter = communicator.createObjectAdapterWithEndpoints("Hello", "tcp -h localhost -p 10000")
            adapter.add(New HelloI, communicator.stringToIdentity("hello"))
            adapter.activate()
            communicator.waitForShutdown()
            communicator.destroy()
        Catch ex As System.Exception
            System.Console.Error.WriteLine(ex)
            System.Environment.Exit(-1)
        End Try
    End Sub

End Module
