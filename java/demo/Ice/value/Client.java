// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client extends Ice.Application
{
    class ShutdownHook extends Thread
    {
        public void
        run()
        {
            try
            {
                communicator().destroy();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
            }
        }
    }

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

    public int
    run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        //
        // Since this is an interactive demo we want to clear the
        // Application installed interrupt callback and install our
        // own shutdown hook.
        //
        setInterruptHook(new ShutdownHook());

        InitialPrx initial = InitialPrxHelper.checkedCast(communicator().propertyToProxy("Initial.Proxy"));
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
        System.out.println("Yes, this worked. Now let's try to transfer an object for a class");
        System.out.println("with operations as type Demo.Printer, without installing a factory first.");
        System.out.println("This should give us a `no factory' exception.");
        System.out.println("[press enter]");
        readline(in);

        PrinterHolder printer = new PrinterHolder();
        PrinterPrxHolder printerProxy = new PrinterPrxHolder();
        boolean gotException = false;
        try
        {
            initial.getPrinter(printer, printerProxy);
            System.err.println("Did not get the expected NoObjectFactoryException!");
            System.exit(1);
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
        communicator().addObjectFactory(factory, Demo.Printer.ice_staticId());

        initial.getPrinter(printer, printerProxy);
        System.out.println("==> " + printer.value.message);

        System.out.println();
        System.out.println("Cool, it worked! Let's try calling the printBackwards() method");
        System.out.println("on the object we just received locally.");
        System.out.println("[press enter]");
        readline(in);

        System.out.print("==> ");
        printer.value.printBackwards();

        System.out.println();
        System.out.println("Now we call the same method, but on the remote object. Watch the");
        System.out.println("server's output.");
        System.out.println("[press enter]");
        readline(in);

        printerProxy.value.printBackwards();

        System.out.println();
        System.out.println("Next, we transfer a derived object from the server as a base");
        System.out.println("object. Since we haven't yet installed a factory for the derived");
        System.out.println("class, the derived class (Demo.DerivedPrinter) is sliced");
        System.out.println("to its base class (Demo.Printer).");
        System.out.println("[press enter]");
        readline(in);

        Printer derivedAsBase = initial.getDerivedPrinter();
        System.out.println("==> The type ID of the received object is \"" + derivedAsBase.ice_id() + "\"");
        assert(derivedAsBase.ice_id().equals(Demo.Printer.ice_staticId()));

        System.out.println();
        System.out.println("Now we install a factory for the derived class, and try again.");
        System.out.println("Because we receive the derived object as a base object,");
        System.out.println("we need to do a class cast to get from the base to the derived object.");
        System.out.println("[press enter]");
        readline(in);

        communicator().addObjectFactory(factory, Demo.DerivedPrinter.ice_staticId());

        derivedAsBase = initial.getDerivedPrinter();
        DerivedPrinter derived = (Demo.DerivedPrinter)derivedAsBase;

        System.out.println("==> class cast to derived object succeeded");
        System.out.println("==> The type ID of the received object is \"" + derived.ice_id() + "\"");

        System.out.println();
        System.out.println("Let's print the message contained in the derived object, and");
        System.out.println("call the operation printUppercase() on the derived object");
        System.out.println("locally.");
        System.out.println("[press enter]");
        readline(in);

        System.out.println("==> " + derived.derivedMessage);
        System.out.print("==> ");
        derived.printUppercase();

        System.out.println();
        System.out.println("Now let's make sure that slice is preserved with [\"preserve-slice\"]");
        System.out.println("metadata. We create a derived type on the client and pass it to the");
        System.out.println("server, which does not have a factory for the derived type. We do a");
        System.out.println("class cast to make sure we can still access the derived type when");
        System.out.println("it has been returned from the server.");
        System.out.println("[press enter]");
        readline(in);

    	ClientPrinter clientp = new ClientPrinterI();
    	clientp.message = "a message 4 u";
    	communicator().addObjectFactory(factory, Demo.ClientPrinter.ice_staticId());

    	derivedAsBase = initial.updatePrinterMessage(clientp);
    	clientp = (Demo.ClientPrinter)derivedAsBase;
        System.out.println("==> " + clientp.message);

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
        derived.printUppercase();

        System.out.println();
        System.out.println("That's it for this demo. Have fun with Ice!");

        initial.shutdown();

        return 0;
    }

    public static void
    main(String[] args)
    {
        Client app = new Client();
        int status = app.main("Client", args, "config.client");
        System.exit(status);
    }
}
