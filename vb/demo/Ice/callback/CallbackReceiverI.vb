' **********************************************************************
'
' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Demo

Public NotInheritable Class CallbackReceiverI
    Inherits _CallbackReceiverDisp

    Public Overloads Overrides Sub callback(ByVal current As Ice.Current)
	System.Console.Out.WriteLine("received callback")
    End Sub

End Class
