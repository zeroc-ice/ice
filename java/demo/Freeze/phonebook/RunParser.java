// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class RunParser
{
    static void
    usage(String appName)
    {
        System.err.println("Usage: " + appName + " [options] [file...]\n");
        System.err.print(
            "Options:\n" +
            "-h, --help           Show this message.\n");
        //"-v, --version        Display the Ice version.\n"
    }

    static int
    runParser(String appName, String[] args, Ice.Communicator communicator)
    {
        String file = null;
        int idx = 0;

        while(idx < args.length)
        {
            if(args[idx].equals("-h") | args[idx].equals("--help"))
            {
                usage(appName);
                return 0;
            }
/*
  else if(args[idx].equals("-v") || args[idx].equals("--version"))
  {
  cout + ICE_STRING_VERSION + endl;
  return 0;
  }
*/
            else if(args[idx].charAt(0) == '-')
            {
                System.err.println(appName + ": unknown option `" + args[idx] + "'");
                usage(appName);
                return 1;
            }
            else
            {
                if(file == null)
                {
                    file = args[idx];
                }
                else
                {
                    System.err.println(appName + ": only one file is supported.");
                    usage(appName);
                    return 1;
                }
                ++idx;
            }
        }

        Ice.ObjectPrx base = communicator.propertyToProxy("PhoneBook.Proxy");
        PhoneBookPrx phoneBook = PhoneBookPrxHelper.checkedCast(base);
        if(phoneBook == null)
        {
            System.err.println(appName + ": invalid object reference");
            return 1;
        }

        Parser parser = new Parser(communicator, phoneBook);
        int status;

        if(file == null)
        {
            status = parser.parse();
        }
        else
        {
            try
            {
                status = parser.parse(new java.io.BufferedReader(new java.io.FileReader(file)));
            }
            catch(java.io.IOException ex)
            {
                status = 1;
                ex.printStackTrace();
            }
        }

        return status;
    }
}
