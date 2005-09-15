'  **********************************************************************
' 
'  Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
' 
'  This copy of Ice is licensed to you under the terms described in the
'  ICE_LICENSE file included in this distribution.
' 
'  **********************************************************************

Imports System

Module Glacier2callbackSS

    NotInheritable Class DummyPermissionVerifierI
        Inherits Glacier2._PermissionsVerifierDisp

        Public Overloads Overrides Function checkPermissions(ByVal userId As String, ByVal password As String, ByRef reason As String, ByVal current As Ice.Current) As Boolean
            reason = Nothing
            Console.WriteLine("verified user `" & userId & "' with password `" & password & "'")
            Return True
        End Function

    End Class

    Class Server
        Inherits Ice.Application

        Public Overloads Overrides Function run(ByVal args() As String) As Integer

            Dim adapter As Ice.ObjectAdapter = communicator().createObjectAdapter("SessionServer")
            adapter.add(New DummyPermissionVerifierI, Ice.Util.stringToIdentity("verifier"))
            adapter.add(New SessionManagerI, Ice.Util.stringToIdentity("sessionmanager"))
            adapter.activate()
            communicator().waitForShutdown()
            Return 0
        End Function

    End Class

    Sub Main(ByVal args() As String)
        Dim app As Server = New Server
        Dim status As Integer = app.main(args, "config.sessionserver")
        Environment.Exit(status)
    End Sub

End Module
