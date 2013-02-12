// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.fileLock;

import java.io.BufferedReader;
import java.io.InputStreamReader;

import Freeze.*;

public class ClientFail
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

            Freeze.Connection connection = null;
            try
            {
                connection = Freeze.Util.createConnection(communicator, envName);
                test(false);
            }
            catch(IceUtil.FileLockException ex)
            {
                System.out.println("File lock not acquired.");
                test(true);
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
    }
}
