// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.logger;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        System.out.print("testing Ice.LogFile... ");
        if(new File("log.txt").exists())
        {
            new File("log.txt").delete();
        }

        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.LogFile", "log.txt");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getLogger().trace("info", "my logger");
        }
        test(new File("log.txt").exists());

        try
        {
            test(new String(Files.readAllBytes(Paths.get("log.txt"))).contains("my logger"));
        }
        catch(IOException ex)
        {
            test(false);
        }
        new File("log.txt").delete();
        System.out.println("ok");
    }
}
