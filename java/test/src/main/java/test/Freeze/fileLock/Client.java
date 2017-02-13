// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.fileLock;

import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Client extends test.Util.Application
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    @Override
    public int run(String[] args)
    {
        String envName = "db";
        if(args.length > 0)
        {
            envName = args[0];
            envName += "/";
            envName += "db";
        }

        Freeze.Connection connection = Freeze.Util.createConnection(communicator(), envName);
        System.out.println("File lock acquired.");
        System.out.println("Enter some input and press enter, to release the lock and terminate the program.");
        //
        // Block the test waiting for IO, so the file lock is preserved.
        //
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        try
        {
            br.readLine();
        }
        catch(java.io.IOException ex)
        {
            System.out.println("exception:\n" + ex.toString());
            test(false);
        }

        if(connection != null)
        {
            connection.close();
        }
        System.out.println("File lock released.");
        return 0;
    }
    
    public static void main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);
        System.gc();
        System.exit(status);
    }
}
