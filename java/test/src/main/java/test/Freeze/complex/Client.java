// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.complex;
import test.Freeze.complex.Complex.*;

import Freeze.*;

public class Client extends test.Util.Application
{
    final static String progName = "test.Freeze.complex.Client";

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private int
    validate(String dbName)
        throws DatabaseException
    {
        ComplexDict m = new ComplexDict(_connection, dbName, true);

        try
        {
            Parser parser = new Parser();

            System.out.print("testing database expressions... ");
            java.util.Iterator<java.util.Map.Entry<Key, Node>> p = m.entrySet().iterator();
            while(p.hasNext())
            {
                java.util.Map.Entry<Key, Node> e = p.next();

                Key key = e.getKey();

                //
                // Verify the stored record is correct.
                //
                test(key.result == e.getValue().calc());

                //
                // Verify that the expression & result again.
                //
                Node root = parser.parse(key.expression);
                test(root.calc(null) == key.result);
            }
            System.out.println("ok");
        }
        catch(ParseError e)
        {
            e.printStackTrace();
            test(false);
        }

        m.close();

        return 0;
    }

    private int
    populate(String dbName)
        throws DatabaseException
    {
        String[] expressions =
        {
            "2",
            "10",
            "2+(5*3)",
            "5*(2+3)",
            "10+(10+(20+(8*(2*(3*2+4+5+6)))))"
        };

        ComplexDict m = new ComplexDict(_connection, dbName, true);

        try
        {
            Parser parser = new Parser();

            System.out.print("populating the database... ");
            for(String expr : expressions)
            {
                Node root = parser.parse(expr);
                assert(root != null);
                Key k = new Key();
                k.expression = expr;
                k.result = root.calc(null);
                m.put(k, root);
            }
            System.out.println("ok");
        }
        catch(ParseError e)
        {
            e.printStackTrace();
            test(false);
        }

        m.close();
        return 0;
    }

    static void
    usage(String name)
    {
        System.out.println("Usage: " + name + " [options] validate|populate");
        System.out.println("Options:");
        System.out.println("--dbdir           Location of the database directory.");
    }

    private int
    run(String[] args, String dbName)
        throws DatabaseException
    {
        //
        // Register a factory for the node types.
        //
        Ice.ObjectFactory factory = new ObjectFactoryI();
        communicator().addObjectFactory(factory, "::Complex::NumberNode");
        communicator().addObjectFactory(factory, "::Complex::AddNode");
        communicator().addObjectFactory(factory, "::Complex::MultiplyNode");

        if(args.length != 0 && args[0].equals("populate"))
        {
            return populate(dbName);
        }
        if(args.length != 0 && args[0].equals("validate"))
        {
            return validate(dbName);
        }
        usage(progName);

        return 0;
    }

    private void
    close()
    {
        _connection.close();
    }

    @Override
    public int run(String[] args)
    {
        int status;
        Ice.Communicator communicator = null;
        String envName = "db";

        //
        // Scan for --dbdir command line argument.
        //
        int i = 0;
        while(i < args.length)
        {
            if(args[i].equals("--dbdir"))
            {
                if(i +1 >= args.length)
                {
                    usage(progName);
                    System.exit(1);
                }

                envName = args[i+1];
                envName += "/";
                envName += "db";

                //
                // Consume arguments
                //
                String[] arr = new String[args.length - 2];
                System.arraycopy(args, 0, arr, 0, i);
                if(i < args.length - 2)
                {
                    System.arraycopy(args, i + 2, arr, i, args.length - i - 2);
                }
                args = arr;
            }
            else
            {
                ++i;
            }
        }

        _connection = Freeze.Util.createConnection(communicator(), envName);
        try
        {
            status = run(args, "test");
        }
        finally
        {
            close();
        }
        return status;
    }
    
    public static void main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);
        System.gc();
        System.exit(status);
    }

    private Ice.Communicator _communicator;
    private Freeze.Connection _connection;
}
