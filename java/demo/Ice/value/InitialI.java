// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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
        _simple._message = "a message 4 u";

        _printer = new PrinterI();
        _printer._message = "Ice rulez!";
        _printerProxy =
            PrinterPrxHelper.uncheckedCast(adapter.addWithUUID(_printer));

        _derivedPrinter = new DerivedPrinterI();
        _derivedPrinter._message = _printer._message;
        _derivedPrinter._derivedMessage = "Coming soon: the ultimate online " +
            "game from MutableRealms!";
        adapter.addWithUUID(_derivedPrinter);
    }

    public Simple
    simple(Ice.Current current)
    {
        return _simple;
    }

    public void
    printer(PrinterHolder impl, PrinterPrxHolder proxy, Ice.Current current)
    {
        impl.value = _printer;
        proxy.value = _printerProxy;
    }

    public Printer
    derivedPrinter(Ice.Current current)
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
