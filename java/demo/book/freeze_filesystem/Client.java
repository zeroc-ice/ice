// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;

public class Client extends Ice.Application
{
    // Recursively print the contents of directory "dir" in tree fashion.
    // For files, show the contents of each file. The "depth"
    // parameter is the current nesting level (for indentation).

    static void
    listRecursive(DirectoryPrx dir, int depth)
    {
        char[] indentCh = new char[++depth];
        java.util.Arrays.fill(indentCh, '\t');
        String indent = new String(indentCh);

        NodeDesc[] contents = dir.list();

        for (int i = 0; i < contents.length; ++i) {
            DirectoryPrx subdir = DirectoryPrxHelper.checkedCast(contents[i].proxy);
            FilePrx file = FilePrxHelper.uncheckedCast(contents[i].proxy);
            System.out.println(indent + contents[i].name + (subdir != null ? " (directory):" : " (file):"));
            if (subdir != null) {
                listRecursive(subdir, depth);
            } else {
                String[] text = file.read();
                for (int j = 0; j < text.length; ++j)
                    System.out.println(indent + "\t" + text[j]);
            }
        }
    }

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
                NodeDesc desc = rootDir.find("Coleridge");
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
            listRecursive(rootDir, 0);

            //
            // Destroy the filesystem.
            //
            NodeDesc[] contents = rootDir.list();
            for(int i = 0; i < contents.length; ++i)
            {
                NodeDesc d = contents[i];
                System.out.println("Destroying " + d.name);
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
