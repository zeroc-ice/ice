' **********************************************************************
'
' Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports System.Diagnostics

Public Class ObjectFactory
    Implements Ice.ObjectFactory

    Public Overloads Function create(ByVal type As String) As Ice.Object Implements Ice.ObjectFactory.create
        If type.Equals("::Demo::Printer") Then
            Return New PrinterI
        End If

        If type.Equals("::Demo::DerivedPrinter") Then
            Return New DerivedPrinterI
        End If

        If type.Equals("::Demo::ClientPrinter") Then
            Return New ClientPrinterI
        End If

        ' Debug.Assert(False)   ' Bug in VB 7.1: Diagnostics.Debug is not found
        Return Nothing
    End Function

    Public Overloads Sub destroy() Implements Ice.ObjectFactory.destroy
        ' Nothing to do
    End Sub

End Class
