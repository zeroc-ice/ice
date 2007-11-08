' **********************************************************************
'
' Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports BidirDemo

Public NotInheritable Class CallbackReceiverI
    Inherits CallbackReceiverDisp_

    Public Overloads Overrides Sub callback(ByVal num As Integer, ByVal current As Ice.Current)
        System.Console.Out.WriteLine("received callback #" & num)
    End Sub

End Class
