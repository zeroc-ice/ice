' **********************************************************************
'
' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Module bidirC

    Public Sub Main(ByVal args() As String)
        Dim app As CallbackClient = New CallbackClient
        Dim status As Integer = app.main(args, "config")
        System.Environment.Exit(status)
    End Sub

End Module
