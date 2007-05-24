// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;

public class Client
{
    private static void test(bool b)
    {
        if (!b)
        {
            throw new Exception();
        }
    }

    public static void Main(string[] argvs)
    {
        Debug.Listeners.Add(new ConsoleTraceListener());

        System.Console.Out.Write("testing string to command line arguments... ");
        System.Console.Out.Flush();
        string[] args;

        try
        {
            test(IceUtil.Options.split("").Length == 0);
            
            args = IceUtil.Options.split("\"\"");
            test(args.Length == 1 && args[0].Equals(""));
            args = IceUtil.Options.split("''");
            test(args.Length == 1 && args[0].Equals(""));
            args = IceUtil.Options.split("$''");
            test(args.Length == 1 && args[0].Equals(""));

            args = IceUtil.Options.split("-a -b -c");
            test(args.Length == 3 && args[0].Equals("-a") && args[1].Equals("-b") && args[2].Equals("-c"));
            args = IceUtil.Options.split("\"-a\" '-b' $'-c'");
            test(args.Length == 3 && args[0].Equals("-a") && args[1].Equals("-b") && args[2].Equals("-c"));
            args = IceUtil.Options.split("  '-b' \"-a\" $'-c' ");
            test(args.Length == 3 && args[0].Equals("-b") && args[1].Equals("-a") && args[2].Equals("-c"));
            args = IceUtil.Options.split(" $'-c' '-b' \"-a\"  ");
            test(args.Length == 3 && args[0].Equals("-c") && args[1].Equals("-b") && args[2].Equals("-a"));

            // Testing single quote
            args = IceUtil.Options.split("-Dir='C:\\\\test\\\\file'"); // -Dir='C:\\test\\file'
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\\\test\\\\file")); // -Dir=C:\\test\\file
            args = IceUtil.Options.split("-Dir='C:\\test\\file'"); // -Dir='C:\test\file'
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtil.Options.split("-Dir='C:\\test\\filewith\"quote'"); // -Dir='C:\test\filewith"quote'
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\filewith\"quote")); // -Dir=C:\test\filewith"quote

            // Testing double quote
            args = IceUtil.Options.split("-Dir=\"C:\\\\test\\\\file\""); // -Dir="C:\\test\\file"
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
                 args = IceUtil.Options.split("-Dir=\"C:\\test\\file\""); // -Dir="C:\test\file"
                 test(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtil.Options.split("-Dir=\"C:\\test\\filewith\\\"quote\""); // -Dir="C:\test\filewith\"quote"
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\filewith\"quote")); // -Dir=C:\test\filewith"quote

            // Testing ANSI quote
            args = IceUtil.Options.split("-Dir=$'C:\\\\test\\\\file'"); // -Dir=$'C:\\test\\file'
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtil.Options.split("-Dir=$'C:\\oest\\oile'"); // -Dir='C:\oest\oile'
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\oest\\oile")); // -Dir=C:\oest\oile
            args = IceUtil.Options.split("-Dir=$'C:\\oest\\oilewith\"quote'"); // -Dir=$'C:\oest\oilewith"quote'
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\oest\\oilewith\"quote")); // -Dir=C:\oest\oilewith"quote
            args = IceUtil.Options.split("-Dir=$'\\103\\072\\134\\164\\145\\163\\164\\134\\146\\151\\154\\145'");
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtil.Options.split("-Dir=$'\\x43\\x3A\\x5C\\x74\\x65\\x73\\x74\\x5C\\x66\\x69\\x6C\\x65'");
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtil.Options.split("-Dir=$'\\cM\\c_'"); // Control characters
            test(args.Length == 1 && args[0].Equals("-Dir=\x0D\x1F"));
            args = IceUtil.Options.split("-Dir=$'C:\\\\\\146\\x66\\cMi'"); // -Dir=$'C:\\\146\x66i\cMi'
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\ff\x0Di"));
            args = IceUtil.Options.split("-Dir=$'C:\\\\\\cM\\x66\\146i'"); // -Dir=$'C:\\\cM\x66\146i'
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\\x000Dffi"));
        }
        catch(IceUtil.Options.BadQuote)
        {
            test(false);
        }

        string[] badQuoteCommands = new string[6];
        badQuoteCommands[0] = "\"";
        badQuoteCommands[1] = "'";
        badQuoteCommands[2] = "\\$'";
        badQuoteCommands[3] = "-Dir=\"test";
        badQuoteCommands[4] = "-Dir='test";
        badQuoteCommands[5] = "-Dir=$'test";
        for(int i = 0; i < 6; ++i)
        {
            try
            {
                IceUtil.Options.split(badQuoteCommands[i]);
                test(false);
            }
            catch(IceUtil.Options.BadQuote)
            {
            }
        }

        System.Console.Out.WriteLine("ok");
    }
}
