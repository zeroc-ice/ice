// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class Client
{
    private static void
    readline(java.io.BufferedReader in)
    {
        try
        {
            in.readLine();
        }
        catch(java.io.IOException ex)
        {
            ex.printStackTrace();
        }
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        Ice.Properties properties = communicator.getProperties();
        final String refProperty = "Value.Initial";
        String ref = properties.getProperty(refProperty);
        if(ref.length() == 0)
        {
            System.err.println("property `" + refProperty + "' not set");
            return 1;
        }

        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        InitialPrx initial = InitialPrxHelper.checkedCast(base);
        if(initial == null)
        {
            System.err.println("invalid object reference");
            return 1;
        }

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        System.out.println();
        System.out.println("Let's first transfer a simple object, for a class without");
        System.out.println("operations, and print its contents. No factory is required");
        System.out.println("for this.");
        System.out.println("[press enter]");
        readline(in);

        Simple simple = initial.getSimple();
        System.out.println("==> " + simple.message);

        System.out.println();
        System.out.println("Ok, this worked. Now let's try to transfer an object for a class");
        System.out.println("with operations, without installing a factory first. This should");
        System.out.println("give us a `no factory' exception.");
        System.out.println("[press enter]");
        readline(in);

        PrinterHolder printer = new PrinterHolder();
        PrinterPrxHolder printerProxy = new PrinterPrxHolder();
        try
        {
            initial.getPrinter(printer, printerProxy);
        }
        catch(Ice.NoObjectFactoryException ex)
        {
            System.out.println("==> " + ex);
        }

        System.out.println();
        System.out.println("Yep, that's what we expected. Now let's try again, but with");
        System.out.println("installing an appropriate factory first. If successful, we print");
        System.out.println("the object's content.");
        System.out.println("[press enter]");
        readline(in);

        Ice.ObjectFactory factory = new ObjectFactory();
        communicator.addObjectFactory(factory, "::Printer");

        initial.getPrinter(printer, printerProxy);
        System.out.println("==> " + printer.value.message);

        System.out.println();
        System.out.println("Cool, it worked! Let's try calling the printBackwards() method");
        System.out.println("on the object we just received locally.");
        System.out.println("[press enter]");
        readline(in);

        System.out.print("==> ");
        printer.value.printBackwards(null);

        System.out.println();
        System.out.println("Now we call the same method, but on the remote object. Watch the");
        System.out.println("server's output.");
        System.out.println("[press enter]");
        readline(in);

        printerProxy.value.printBackwards();

        System.out.println();
        System.out.println("Next, we transfer a derived object from the server as base");
        System.out.println("object. Since we didn't install a factory for the derived");
        System.out.println("class yet, we will get another `no factory' exception.");
        System.out.println("[press enter]");
        readline(in);

        Printer derivedAsBase;
        try
        {
            derivedAsBase = initial.getDerivedPrinter();
            assert(false);
        }
        catch(Ice.NoObjectFactoryException ex)
        {
            System.out.println("==> " + ex);
        }

        System.out.println();
        System.out.println("Now we install a factory for the derived class, and try again.");
        System.out.println("We won't get a `no factory' exception anymore, but since we");
        System.out.println("receive the derived object as base object, we need to do a");
        System.out.println("dynamic_cast<> to get from the base to the derived object.");
        System.out.println("[press enter]");
        readline(in);

        communicator.addObjectFactory(factory, "::DerivedPrinter");

        derivedAsBase = initial.getDerivedPrinter();
        DerivedPrinter derived = (DerivedPrinter)derivedAsBase;

        System.out.println("==> dynamic_cast<> to derived object succeded");

        System.out.println();
        System.out.println("Let's print the message contained in the derived object, and");
        System.out.println("call the operation printUppercase() on the derived object");
        System.out.println("locally.");
        System.out.println("[press enter]");
        readline(in);

        System.out.println("==> " + derived.derivedMessage);
        System.out.print("==> ");
        derived.printUppercase(null);

        System.out.println();
        System.out.println("Finally, we try the same again, but instead of returning the");
        System.out.println("derived object, we throw an exception containing the derived");
        System.out.println("object.");
        System.out.println("[press enter]");
        readline(in);

        try
        {
            initial.throwDerivedPrinter();
        }
        catch(DerivedPrinterException ex)
        {
            derived = ex.derived;
            assert(derived != null);
        }

        System.out.println("==> " + derived.derivedMessage);
        System.out.print("==> ");
        derived.printUppercase(null);

        System.out.println();
        System.out.println("That's it for this demo. Have fun with Ice!");

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            Ice.Properties properties = Ice.Util.createProperties(args);
            properties.load("config");
            communicator = Ice.Util.initializeWithProperties(args, properties);
            status = run(args, communicator);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
                status = 1;
            }
        }

        System.exit(status);
    }
}
