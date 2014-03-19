' **********************************************************************
'
' Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Module SimpleIceGridS

    Class Server
        Inherits Ice.Application

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            If args.Length > 0 Then
                Console.Error.WriteLine(appName() & ": too many arguments")
                Return 1
            End If

            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("Hello")
            Dim properties As Ice.Properties = communicator().getProperties()
            Dim id As Ice.Identity = communicator().stringToIdentity(properties.getProperty("Identity"))
            adapter.add(New HelloI(properties.getProperty("Ice.ProgramName")), id)
            adapter.activate()
            communicator.waitForShutdown()
            Return 0
        End Function
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Server = New Server
        Dim status As Integer = app.Main(args)
        System.Environment.Exit(status)
    End Sub
End Module
