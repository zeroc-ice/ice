' **********************************************************************
'
' Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Demo

Public NotInheritable Class ThroughputI
    Inherits _ThroughputDisp

    Public Sub New()
	_seq = New Byte(seqSize.value) {}
    End Sub

    Public Overloads Overrides Function echoByteSeq(ByVal seq As Byte(), ByVal current As Ice.Current) As Byte()
	Return seq
    End Function

    Public Overloads Overrides Function recvByteSeq(ByVal current As Ice.Current) As Byte()
	Return _seq
    End Function

    Public Overloads Overrides Sub sendByteSeq(ByVal seq() As Byte, ByVal current As Ice.Current)
    End Sub

    Private _seq() As Byte

End Class
