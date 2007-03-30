// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;

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

    public int
    run(String[] args)
    {
        //
        // Since this is an interactive demo we want to clear the
        // Application installed interrupt callback and install our
        // own shutdown hook.
        //
        setInterruptHook(new ShutdownHook());

        //
        // Create a proxy for the root directory.
        //
        DirectoryPrx rootDir = DirectoryPrxHelper.checkedCast(communicator().propertyToProxy("RootDir.Proxy"));
        if(rootDir == null)
        {
            System.err.println("Client: invalid proxy");
            return 1;
        }

        try
        {
            //
            // Create a file called "README" in the root directory.
            //
            try
            {
                FilePrx readme = rootDir.createFile("README");
                String[] text = new String[1];
                text[0] = "This file system contains a collection of poetry.";
                readme.write(text);
                System.out.println("Created README.");
            }
            catch(NameInUse ex)
            {
                //
                // Ignore - file already exists.
                //
            }

            //
            // Create a directory called "Coleridge" in the root directory.
            //
            DirectoryPrx coleridge = null;
            try
            {
                coleridge = rootDir.createDirectory("Coleridge");
                System.out.println("Created Coleridge.");
            }
            catch(NameInUse ex)
            {
                NodeDesc desc = rootDir.resolve("Coleridge");
                coleridge = DirectoryPrxHelper.checkedCast(desc.proxy);
                assert(coleridge != null);
            }

            //
            // Create a file called "Kubla_Khan" in the Coleridge directory.
            //
            try
            {
                FilePrx file = coleridge.createFile("Kubla_Khan");
                String[] text = new String[5];
                text[0] = "In Xanadu did Kubla Khan";
                text[1] = "A stately pleasure-dome decree:";
                text[2] = "Where Alph, the sacred river, ran";
                text[3] = "Through caverns measureless to man";
                text[4] = "Down to a sunless sea.";
                file.write(text);
                System.out.println("Created Coleridge/Kubla_Khan.");
            }
            catch(NameInUse ex)
            {
                //
                // Ignore - file already exists.
                //
            }

            System.out.println("Contents of filesystem:");
            java.util.Map contents = rootDir.list(ListMode.RecursiveList);
            java.util.Iterator p = contents.keySet().iterator();
            while(p.hasNext())
            {
                System.out.println("  " + (String)p.next());
            }

            NodeDesc desc = rootDir.resolve("Coleridge/Kubla_Khan");
            FilePrx file = FilePrxHelper.checkedCast(desc.proxy);
            assert(file != null);
            String[] text = file.read();
            System.out.println("Contents of file Coleridge/Kubla_Khan:");
            for(int i = 0; i < text.length; ++i)
            {
                System.out.println("  " + text[i]);
            }

            //
            // Destroy the filesystem.
            //
            contents = rootDir.list(ListMode.NormalList);
            p = contents.entrySet().iterator();
            while(p.hasNext())
            {
                java.util.Map.Entry e = (java.util.Map.Entry)p.next();
                NodeDesc d = (NodeDesc)e.getValue();
                System.out.println("Destroying " + (String)e.getKey() + "...");
                d.proxy.destroy();
            }
        }
        catch(Ice.UserException ex)
        {
            ex.printStackTrace();
        }

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
