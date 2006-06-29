' **********************************************************************
'
' Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Module HelloS

    Class Server
        Inherits Ice.Application

        Public Overloads Overrides Function run(ByVal args() As String) As Integer
            Dim endpointProperty As String = ""
            Try
                communicator().getPluginManager().getPlugin("IceSSL")
                endpointProperty = "EndpointsWithSSL"
            Catch ex As Ice.NotRegisteredException
                endpointProperty = "Endpoints"
                Console.WriteLine("SSL support not enabled (requires .NET 2.0 or later)")
            End Try
            Dim props As Ice.Properties = communicator().getProperties()
            Dim endpoints As String = props.getProperty(endpointProperty)
            If endpoints.Length = 0 Then
                Console.Error.WriteLine("property `" & endpointProperty & "' not set")
                Return 1
            End If

            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapterWithEndpoints("Hello", endpoints)
            adapter.add(New HelloI, communicator().stringToIdentity("hello"))
            adapter.activate()
            communicator().waitForShutdown()
            Return 0
        End Function
    End Class

    Public Sub Main(ByVal args() As String)
        Dim app As Server = New Server
        Dim status As Integer = app.Main(args, "config.server")
        System.Environment.Exit(status)
    End Sub
End Module
