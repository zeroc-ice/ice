// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.fileLock;

import java.io.BufferedReader;
import java.io.InputStreamReader;

import Freeze.*;

public class Client
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static public void
    main(String[] args)
    {
        Ice.Communicator communicator = null;
        String envName = "db";

        try
        {
            Ice.StringSeqHolder holder = new Ice.StringSeqHolder();
            holder.value = args;
            communicator = Ice.Util.initialize(holder);
            args = holder.value;
            if(args.length > 0)
            {
                envName = args[0];
                envName += "/";
                envName += "db";
            }

            Freeze.Connection connection = Freeze.Util.createConnection(communicator, envName);
            System.out.println("File lock acquired.");
            System.out.println("Enter some input and press enter, to release the lock and terminate the program.");
            //
            // Block the test waiting for IO, so the file lock is preserved.
            //
            BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
            try
            {
                String dummy = br.readLine();
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
        }
        catch(Exception ex)
        {
            System.out.println("exception:\n" + ex.toString());
            test(false);
        }
        finally
        {
            if(communicator != null)
            {
                try
                {
                    communicator.destroy();
                }
                catch(Exception ex)
                {
                    System.err.println("exception:\n" + ex);
                }
            }
        }
        System.out.println("File lock released.");
    }
}
