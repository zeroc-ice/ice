// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

class NestedClient extends Ice.Application
{
    public int
    run(String[] args)
    {
        Ice.Properties properties = communicator().getProperties();
        final String refProperty = "Nested.NestedServer";
        String ref = properties.getProperty(refProperty);
        if(ref.length() == 0)
        {
            System.err.println("property `" + refProperty + "' not set");
            return 1;
        }

        Ice.ObjectPrx base = communicator().stringToProxy(ref);
        NestedPrx nested = NestedPrxHelper.checkedCast(base);
        if(nested == null)
        {
            System.err.println("invalid object reference");
            return 1;
        }

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("NestedClientAdapter");
        NestedPrx self = NestedPrxHelper.uncheckedCast(adapter.createProxy(Ice.Util.stringToIdentity("nestedClient")));
        adapter.add(new NestedI(self), Ice.Util.stringToIdentity("nestedClient"));
        adapter.activate();

        System.out.println("Note: The maximum nesting level is (sz - 1) * 2, with sz");
        System.out.println("being the number of threads in the server thread pool. If");
        System.out.println("you specify a value higher than that, the application will");
        System.out.println("block or timeout.");
        System.out.println();

        java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        String s = null;
        do
        {
            try
            {
                System.out.print("enter nesting level or 'x' for exit: ");
                System.out.flush();
                s = in.readLine();
                if(s == null)
                {
                    break;
                }
                int level = Integer.parseInt(s);
                if(level > 0)
                {
                    nested.nested(level, self);
                }
            }
            catch(NumberFormatException ex)
            {
            }
            catch(java.io.IOException ex)
            {
                ex.printStackTrace();
            }
            catch(Ice.LocalException ex)
            {
                ex.printStackTrace();
            }
        }
        while(!s.equals("x"));

        return 0;
    }
}
