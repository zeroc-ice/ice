// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
            boolean createDb = true;
            NewContacts contacts = new NewContacts(connection, "contacts", createDb, less, indexComparators);
            
            System.out.println("All contacts (default order)");
            java.util.Iterator p = contacts.entrySet().iterator();
            while(p.hasNext())
            {
                java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
                NewContactData data = (NewContactData)entry.getValue();

                System.out.println((String)entry.getKey() + ":\t\t"
                                   + data.phoneNumber + " " + data.emailAddress);
            }
            
            System.out.println("\nAll contacts (ordered by phone number)");
            java.util.SortedMap phoneNumberMap = contacts.mapForIndex("phoneNumber");
            p = phoneNumberMap.values().iterator();
            while(p.hasNext())
            {
                java.util.Set entries = (java.util.Set)p.next();
                java.util.Iterator q = entries.iterator();
                while(q.hasNext())
                {
                    java.util.Map.Entry entry = (java.util.Map.Entry)q.next();
                    NewContactData data = (NewContactData)entry.getValue();

                    System.out.println((String)entry.getKey() + ":\t\t"
                                       + data.phoneNumber + " " + data.emailAddress);
                    
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
        app.main("demo.Freeze.transform.ReadNew", args);
    }
}
