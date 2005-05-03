' **********************************************************************
'
' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Module sessionC

    Public Sub Main(ByVal args() as String)
        Dim app as SessionClient = new SessionClient
        Dim status as Integer = app.main(args, "config")
        System.Environment.Exit(status)
    End Sub

End Module
