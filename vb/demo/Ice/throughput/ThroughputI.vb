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
	_byteSeq = New Byte(ByteSeqSize.value) {}
        _stringSeq = New String(StringSeqSize.value) {}
        For i As Integer = 0 To StringSeqSize.value - 1
            _stringSeq(i) = "hello"
        Next
        _structSeq = New StringDouble(StringDoubleSeqSize.value) {}
        For i As Integer = 0 To StringDoubleSeqSize.value - 1
            _structSeq(i).s = "hello"
            _structSeq(i).d = 3.14
        Next
        _fixedSeq = New Fixed(FixedSeqSize.value) {}
        For i As Integer = 0 To FixedSeqSize.value - 1
            _fixedSeq(i).i = 0
            _fixedSeq(i).j = 0
            _fixedSeq(i).d = 0
        Next
    End Sub

    Public Overloads Overrides Sub sendByteSeq(ByVal seq() As Byte, ByVal current As Ice.Current)
    End Sub

    Public Overloads Overrides Function recvByteSeq(ByVal current As Ice.Current) As Byte()
        Return _byteSeq
    End Function

    Public Overloads Overrides Function echoByteSeq(ByVal seq() As Byte, ByVal current As Ice.Current) As Byte()
	Return seq
    End Function

    Public Overloads Overrides Sub sendStringSeq(ByVal seq() As String, ByVal current As Ice.Current)
    End Sub

    Public Overloads Overrides Function recvStringSeq(ByVal current As Ice.Current) As String()
        Return _stringSeq
    End Function

    Public Overloads Overrides Function echoStringSeq(ByVal seq() As String, ByVal current As Ice.Current) As String()
        Return seq
    End Function

    Public Overloads Overrides Sub sendStructSeq(ByVal seq() As StringDouble, ByVal current As Ice.Current)
    End Sub

    Public Overloads Overrides Function recvStructSeq(ByVal current As Ice.Current) As StringDouble()
        Return _structSeq
    End Function

    Public Overloads Overrides Function echoStructSeq(ByVal seq As StringDouble(), ByVal current As Ice.Current) As StringDouble()
        Return seq
    End Function

    Public Overloads Overrides Sub sendFixedSeq(ByVal seq() As Fixed, ByVal current As Ice.Current)
    End Sub

    Public Overloads Overrides Function recvFixedSeq(ByVal current As Ice.Current) As Fixed()
        Return _fixedSeq
    End Function

    Public Overloads Overrides Function echoFixedSeq(ByVal seq As Fixed(), ByVal current As Ice.Current) As Fixed()
        Return seq
    End Function

    Public Overloads Overrides Sub shutdown(ByVal current As Ice.Current)
        current.adapter.getCommunicator().shutdown()
    End Sub

    Private _byteSeq() As Byte
    Private _stringSeq() As String
    Private _structSeq() As StringDouble
    Private _fixedSeq() As Fixed

End Class
