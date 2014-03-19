' **********************************************************************
'
' Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Module HelloIceBoxS

    Public Class HelloServiceI
        Implements IceBox.Service

        Public Overridable Sub start(ByVal name As String, ByVal communicator As Ice.Communicator, ByVal args() As String) Implements IceBox.Service.start
            _adapter = communicator.createObjectAdapter(name)
            _adapter.add(New HelloI(), communicator.stringToIdentity("hello"))
            _adapter.activate()
        End Sub

        Public Overridable Sub [stop]() Implements IceBox.Service.stop
            _adapter.destroy()
        End Sub

        Private _adapter As Ice.ObjectAdapter

    End Class
End Module
