// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


class InitialI extends Initial
{
    InitialI(Ice.ObjectAdapter adapter)
    {
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

    public Ice.Object
    getPrinterAsObject(Ice.Current current)
    {
        return _printer;
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

    private Simple _simple;
    private Printer _printer;
    private PrinterPrx _printerProxy;
    private DerivedPrinter _derivedPrinter;
}
