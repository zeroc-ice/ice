//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using ZeroC.Ice;

public class Client : Test.TestHelper
{
    public override void Run(string[] argvs)
    {
        Console.Out.Write("testing string to command line arguments... ");
        Console.Out.Flush();
        string[] args;

        try
        {
            Assert(Options.Split("").Length == 0);

            args = Options.Split("\"\"");
            Assert(args.Length == 1 && args[0].Equals(""));
            args = Options.Split("''");
            Assert(args.Length == 1 && args[0].Equals(""));
            args = Options.Split("$''");
            Assert(args.Length == 1 && args[0].Equals(""));

            args = Options.Split("-a -b -c");
            Assert(args.Length == 3 && args[0].Equals("-a") && args[1].Equals("-b") && args[2].Equals("-c"));
            args = Options.Split("\"-a\" '-b' $'-c'");
            Assert(args.Length == 3 && args[0].Equals("-a") && args[1].Equals("-b") && args[2].Equals("-c"));
            args = Options.Split("  '-b' \"-a\" $'-c' ");
            Assert(args.Length == 3 && args[0].Equals("-b") && args[1].Equals("-a") && args[2].Equals("-c"));
            args = Options.Split(" $'-c' '-b' \"-a\"  ");
            Assert(args.Length == 3 && args[0].Equals("-c") && args[1].Equals("-b") && args[2].Equals("-a"));

            // Testing single quote
            args = Options.Split("-Dir='C:\\\\test\\\\file'"); // -Dir='C:\\test\\file'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\\\test\\\\file")); // -Dir=C:\\test\\file
            args = Options.Split("-Dir='C:\\test\\file'"); // -Dir='C:\test\file'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = Options.Split("-Dir='C:\\test\\filewith\"quote'"); // -Dir='C:\test\filewith"quote'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\filewith\"quote")); // -Dir=C:\test\filewith"quote

            // Testing double quote
            args = Options.Split("-Dir=\"C:\\\\test\\\\file\""); // -Dir="C:\\test\\file"
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = Options.Split("-Dir=\"C:\\test\\file\""); // -Dir="C:\test\file"
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = Options.Split("-Dir=\"C:\\test\\filewith\\\"quote\""); // -Dir="C:\test\filewith\"quote"
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\filewith\"quote")); // -Dir=C:\test\filewith"quote

            // Testing ANSI quote
            args = Options.Split("-Dir=$'C:\\\\test\\\\file'"); // -Dir=$'C:\\test\\file'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = Options.Split("-Dir=$'C:\\oest\\oile'"); // -Dir='C:\oest\oile'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\oest\\oile")); // -Dir=C:\oest\oile
            args = Options.Split("-Dir=$'C:\\oest\\oilewith\"quote'"); // -Dir=$'C:\oest\oilewith"quote'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\oest\\oilewith\"quote")); // -Dir=C:\oest\oilewith"quote
            args = Options.Split("-Dir=$'\\103\\072\\134\\164\\145\\163\\164\\134\\146\\151\\154\\145'");
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = Options.Split("-Dir=$'\\x43\\x3A\\x5C\\x74\\x65\\x73\\x74\\x5C\\x66\\x69\\x6C\\x65'");
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = Options.Split("-Dir=$'\\cM\\c_'"); // Control characters
            Assert(args.Length == 1 && args[0].Equals("-Dir=\x0D\x1F"));
            args = Options.Split("-Dir=$'C:\\\\\\146\\x66\\cMi'"); // -Dir=$'C:\\\146\x66i\cMi'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\ff\x0Di"));
            args = Options.Split("-Dir=$'C:\\\\\\cM\\x66\\146i'"); // -Dir=$'C:\\\cM\x66\146i'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\\x000Dffi"));
        }
        catch (FormatException)
        {
            Assert(false);
        }

        string[] badQuoteCommands = new string[6];
        badQuoteCommands[0] = "\"";
        badQuoteCommands[1] = "'";
        badQuoteCommands[2] = "\\$'";
        badQuoteCommands[3] = "-Dir=\"test";
        badQuoteCommands[4] = "-Dir='test";
        badQuoteCommands[5] = "-Dir=$'test";
        for (int i = 0; i < 6; ++i)
        {
            try
            {
                Options.Split(badQuoteCommands[i]);
                Assert(false);
            }
            catch (FormatException)
            {
            }
        }
        Console.Out.WriteLine("ok");
    }

    public static int Main(string[] args) => Test.TestDriver.RunTest<Client>(args);
}
