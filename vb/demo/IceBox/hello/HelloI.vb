' **********************************************************************
'
' Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Demo

Public Class HelloI
    Inherits HelloDisp_

    Public Overloads Overrides Sub sayHello(ByVal current As Ice.Current)
        System.Console.Out.WriteLine("Hello World!")
    End Sub
End Class
