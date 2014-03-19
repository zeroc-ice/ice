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

class Read extends Ice.Application
{
    public int
    run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        Connection connection = Util.createConnection(communicator(), "db");

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
            Contacts.IndexComparators indexComparators = new Contacts.IndexComparators(less);
            Contacts contacts = new Contacts(connection, "contacts", false, less, indexComparators);

            System.out.println("All contacts (default order)");
            for(java.util.Map.Entry<String, ContactData> entry : contacts.entrySet())
            {
                System.out.println(entry.getKey() + ":\t\t" + entry.getValue().phoneNumber);
            }

            System.out.println("\nAll contacts (ordered by phone number)");
            java.util.SortedMap<String, java.util.Set<java.util.Map.Entry<String, ContactData>>> phoneNumberMap =
                contacts.mapForPhoneNumber();
            for(java.util.Set<java.util.Map.Entry<String, ContactData>> entries : phoneNumberMap.values())
            {
                for(java.util.Map.Entry<String, ContactData> entry : entries)
                {
                    System.out.println(entry.getKey() + ":\t\t" + entry.getValue().phoneNumber);
                }
            }
        }
        finally
        {
            connection.close();
        }

        return 0;
    }

    static public void
    main(String[] args)
    {
        Read app = new Read();
        int status = app.main("demo.Freeze.transform.Read", args);
        System.exit(status);
    }
}
