'  **********************************************************************
' 
'  Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
' 
'  This copy of Ice is licensed to you under the terms described in the
'  ICE_LICENSE file included in this distribution.
' 
'  **********************************************************************

Imports glacier2demo.Demo
Imports System

Public NotInheritable Class CallbackReceiverI
    Inherits _CallbackReceiverDisp

    Public Overloads Overrides Sub callback(ByVal current As Ice.Current)
        Console.WriteLine("received callback")
    End Sub

End Class
