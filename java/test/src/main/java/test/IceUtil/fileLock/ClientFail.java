// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceUtil.fileLock;

import java.io.File;
import java.io.RandomAccessFile;
import java.io.EOFException;
import java.io.UTFDataFormatException;
import java.io.IOException;


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

    public static void
    main(String[] argvs)
    {
        
        try
        {
            new IceUtilInternal.FileLock("file.lock");
            test(false);
        }
        catch(IceUtil.FileLockException ex)
        {
            System.out.println("File lock not acquired.");
            //
            // Try to read the pid of the lock owner from the lock file, in Windows 
            // we don't write pid to lock files.
            //
            if(!System.getProperty("os.name").startsWith("Windows"))
            {
                String pid = "";
                try
                {
                    RandomAccessFile file = new RandomAccessFile(new File("file.lock"), "r");
                    try
                    {
                        pid = file.readUTF();
                    }
                    finally
                    {
                        file.close();
                    }
                }
                catch(EOFException eofEx)
                {
                    System.out.println("exception:\n" + eofEx.toString());
                    test(false);
                }
                catch(UTFDataFormatException utfEx)
                {
                    System.out.println("exception:\n" + utfEx.toString());
                    test(false);
                }
                catch(IOException ioEx)
                {
                    System.out.println("exception:\n" + ioEx.toString());
                    test(false);
                }

                if(pid.length() == 0)
                {
                    test(false);
                }
                System.out.println("Lock owned by: " + pid);
            }
        }
    }
}
