// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class InitialI extends _InitialDisp
{
    InitialI(Ice.ObjectAdapter adapter)
    {
        _simple.message = "a message 4 u";

        _printer.message = "Ice rulez!";
        _printerProxy = PrinterPrxHelper.uncheckedCast(adapter.addWithUUID(_printer));

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

    public Printer
    updatePrinterMessage(Printer printer, Ice.Current current)
    {
    	printer.message = "a modified message 4 u";
        return printer;
    }

    public void
    throwDerivedPrinter(Ice.Current current)
        throws DerivedPrinterException
    {
        DerivedPrinterException ex = new DerivedPrinterException();
        ex.derived = _derivedPrinter;
        throw ex;
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private Simple _simple = new Simple();
    private Printer _printer = new PrinterI();
    private PrinterPrx _printerProxy;
    private DerivedPrinter _derivedPrinter = new DerivedPrinterI();
}
