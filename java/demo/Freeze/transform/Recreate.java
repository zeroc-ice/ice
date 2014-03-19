// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

        final java.util.Comparator<String> less = new java.util.Comparator<String>()
        {
            public int compare(String s1, String s2)
            {
                if(s1 == s2)
                {
                    return 0;
                }
                else if(s1 == null)
                {
                    return -s2.compareTo(s1);
                }
                else
                {
                    return s1.compareTo(s2);
                }
            }
        };

        try
        {
            NewContacts.IndexComparators indexComparators = new NewContacts.IndexComparators(less);
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
        int status = app.main("demo.Freeze.transform.Recreate", args);
        System.exit(status);
    }
}
