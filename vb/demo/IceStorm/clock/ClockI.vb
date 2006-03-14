' **********************************************************************
'
' Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Demo

Public Class ClockI
    Inherits ClockDisp_

    Public Overloads Overrides Sub tick(ByVal current As Ice.Current)
	System.Console.Out.WriteLine("tick")
    End Sub
End Class
