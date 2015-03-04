// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;
import Freeze.*;

class ReadNew extends Ice.Application
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
            boolean createDb = true;
            NewContacts contacts = new NewContacts(connection, "contacts", createDb, less, indexComparators);

            System.out.println("All contacts (default order)");
            for(java.util.Map.Entry<String, NewContactData> entry : contacts.entrySet())
            {
                NewContactData data = entry.getValue();
                System.out.println(entry.getKey() + ":\t\t" + data.phoneNumber + " " + data.emailAddress);
            }

            System.out.println("\nAll contacts (ordered by phone number)");
            java.util.SortedMap<String, java.util.Set<java.util.Map.Entry<String, NewContactData>>> phoneNumberMap =
                contacts.mapForPhoneNumber();
            for(java.util.Set<java.util.Map.Entry<String, NewContactData>> entries : phoneNumberMap.values())
            {
                for(java.util.Map.Entry<String, NewContactData> entry : entries)
                {
                    NewContactData data = entry.getValue();
                    System.out.println(entry.getKey() + ":\t\t" + data.phoneNumber + " " + data.emailAddress);
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
        ReadNew app = new ReadNew();
        int status = app.main("demo.Freeze.transform.ReadNew", args);
        System.exit(status);
    }
}
