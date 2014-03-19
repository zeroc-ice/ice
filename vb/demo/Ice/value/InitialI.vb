' **********************************************************************
'
' Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
'
' This copy of Ice is licensed to you under the terms described in the
' ICE_LICENSE file included in this distribution.
'
' **********************************************************************

Imports Demo

Public Class InitialI
    Inherits InitialDisp_

    Public Sub New(ByVal adapter As Ice.ObjectAdapter)
        _simple = New Simple
        _simple.message = "a message 4 u"

        _printer = New PrinterI
        _printer.message = "Ice rulez!"
        _printerProxy = PrinterPrxHelper.uncheckedCast(adapter.addWithUUID(_printer))

        _derivedPrinter = New DerivedPrinterI
        _derivedPrinter.message = _printer.message
        _derivedPrinter.derivedMessage = "a derived message 4 u"
        adapter.addWithUUID(_derivedPrinter)
    End Sub

    Public Overloads Overrides Function getSimple(ByVal current As Ice.Current) As Simple
        Return _simple
    End Function

    Public Overloads Overrides Sub getPrinter(ByRef impl As Printer, ByRef proxy As PrinterPrx, ByVal current As Ice.Current)
        impl = _printer
        proxy = _printerProxy
    End Sub

    Public Overloads Overrides Function getDerivedPrinter(ByVal current As Ice.Current) As Printer
        Return _derivedPrinter
    End Function

    Public Overloads Overrides Function updatePrinterMessage(ByVal printer As Printer, ByVal current As Ice.Current) As Printer
	printer.message = "a modified message 4 u"
        Return printer
    End Function

    Public Overloads Overrides Sub throwDerivedPrinter(ByVal current As Ice.Current)
        Dim ex As DerivedPrinterException = New DerivedPrinterException
        ex.derived = _derivedPrinter
        Throw ex
    End Sub

    Public Overloads Overrides Sub shutdown(ByVal current As Ice.Current)
        current.adapter.getCommunicator().shutdown()
    End Sub

    Private _simple As Simple
    Private _printer As Printer
    Private _printerProxy As PrinterPrx
    Private _derivedPrinter As DerivedPrinter

End Class
