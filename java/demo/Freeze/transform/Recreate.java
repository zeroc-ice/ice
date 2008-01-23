// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;
import Freeze.*;

class Recreate extends Ice.Application
{
    public int
    run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        Connection connection = Util.createConnection(communicator(), "dbnew");

        final java.util.Comparator less =
        new java.util.Comparator()
        {
            public int compare(Object o1, Object o2)
            {
                if(o1 == o2)
                {
                    return 0;
                }
                else if(o1 == null)
                {
                    return -((Comparable)o2).compareTo(o1);
                }
                else
                {
                    return ((Comparable)o1).compareTo(o2);
                }
            }
        };

        java.util.Map indexComparators = new java.util.HashMap();
        indexComparators.put("phoneNumber", less);

        try
        {
            NewContacts.recreate(connection, "contacts", less, indexComparators);
        }
        finally
        {
            connection.close();
        }
        
        System.out.println("Recreated contacts database successfully!");
    
        return 0;
    }

    static public void
    main(String[] args)
    {
        Recreate app = new Recreate();
        app.main("test.Freeze.transform.Recreate", args);
    }
}
