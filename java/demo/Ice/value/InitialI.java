// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

class InitialI extends Initial
{
    InitialI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;

        _simple = new Simple();
        _simple.message = "a message 4 u";

        _printer = new PrinterI();
        _printer.message = "Ice rulez!";
        _printerProxy = PrinterPrxHelper.uncheckedCast(adapter.addWithUUID(_printer));

        _derivedPrinter = new DerivedPrinterI();
        _derivedPrinter.message = _printer.message;
        _derivedPrinter.derivedMessage = "a derived message 4 u";
        adapter.addWithUUID(_derivedPrinter);
    }

    public Simple
    getSimple(Ice.Current current)
    {
        return _simple;
    }

    public void
    getPrinter(PrinterHolder impl, PrinterPrxHolder proxy, Ice.Current current)
    {
        impl.value = _printer;
        proxy.value = _printerProxy;
    }

    public Printer
    getDerivedPrinter(Ice.Current current)
    {
        return _derivedPrinter;
    }

    public void
    throwDerivedPrinter(Ice.Current current)
        throws DerivedPrinterException
    {
        DerivedPrinterException ex = new DerivedPrinterException();
        ex.derived = _derivedPrinter;
        throw ex;
    }

    private Ice.ObjectAdapter _adapter;
    private Simple _simple;
    private Printer _printer;
    private PrinterPrx _printerProxy;
    private DerivedPrinter _derivedPrinter;
}
