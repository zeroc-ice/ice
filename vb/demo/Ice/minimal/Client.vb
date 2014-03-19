' **********************************************************************
'
' Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Demo

Module MinimalC

    Public Sub Main()
        Try
            Dim communicator As Ice.Communicator = Ice.Util.initialize()
            Dim hello As HelloPrx = HelloPrxHelper.checkedCast(communicator.stringToProxy("hello:tcp -h localhost -p 10000"))
            hello.sayHello()
            communicator.destroy()
        Catch ex As System.Exception
            System.Console.Error.WriteLine(ex)
            System.Environment.Exit(1)
        End Try
    End Sub

End Module
