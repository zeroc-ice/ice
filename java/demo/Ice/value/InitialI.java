// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
        _printerProxy =
            PrinterPrxHelper.uncheckedCast(adapter.addWithUUID(_printer));

        _derivedPrinter = new DerivedPrinterI();
        _derivedPrinter.message = _printer.message;
        _derivedPrinter.derivedMessage = "Coming soon: the ultimate online " +
            "game from MutableRealms!";
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
