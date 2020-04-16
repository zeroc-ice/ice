//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

public class Client : Test.TestHelper
{
    public override void Run(string[] argvs)
    {
        Console.Out.Write("testing string to command line arguments... ");
        Console.Out.Flush();
        string[] args;

        try
        {
            Assert(IceUtilInternal.Options.Split("").Length == 0);

            args = IceUtilInternal.Options.Split("\"\"");
            Assert(args.Length == 1 && args[0].Equals(""));
            args = IceUtilInternal.Options.Split("''");
            Assert(args.Length == 1 && args[0].Equals(""));
            args = IceUtilInternal.Options.Split("$''");
            Assert(args.Length == 1 && args[0].Equals(""));

            args = IceUtilInternal.Options.Split("-a -b -c");
            Assert(args.Length == 3 && args[0].Equals("-a") && args[1].Equals("-b") && args[2].Equals("-c"));
            args = IceUtilInternal.Options.Split("\"-a\" '-b' $'-c'");
            Assert(args.Length == 3 && args[0].Equals("-a") && args[1].Equals("-b") && args[2].Equals("-c"));
            args = IceUtilInternal.Options.Split("  '-b' \"-a\" $'-c' ");
            Assert(args.Length == 3 && args[0].Equals("-b") && args[1].Equals("-a") && args[2].Equals("-c"));
            args = IceUtilInternal.Options.Split(" $'-c' '-b' \"-a\"  ");
            Assert(args.Length == 3 && args[0].Equals("-c") && args[1].Equals("-b") && args[2].Equals("-a"));

            // Testing single quote
            args = IceUtilInternal.Options.Split("-Dir='C:\\\\test\\\\file'"); // -Dir='C:\\test\\file'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\\\test\\\\file")); // -Dir=C:\\test\\file
            args = IceUtilInternal.Options.Split("-Dir='C:\\test\\file'"); // -Dir='C:\test\file'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtilInternal.Options.Split("-Dir='C:\\test\\filewith\"quote'"); // -Dir='C:\test\filewith"quote'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\filewith\"quote")); // -Dir=C:\test\filewith"quote

            // Testing double quote
            args = IceUtilInternal.Options.Split("-Dir=\"C:\\\\test\\\\file\""); // -Dir="C:\\test\\file"
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtilInternal.Options.Split("-Dir=\"C:\\test\\file\""); // -Dir="C:\test\file"
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtilInternal.Options.Split("-Dir=\"C:\\test\\filewith\\\"quote\""); // -Dir="C:\test\filewith\"quote"
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\filewith\"quote")); // -Dir=C:\test\filewith"quote

            // Testing ANSI quote
            args = IceUtilInternal.Options.Split("-Dir=$'C:\\\\test\\\\file'"); // -Dir=$'C:\\test\\file'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtilInternal.Options.Split("-Dir=$'C:\\oest\\oile'"); // -Dir='C:\oest\oile'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\oest\\oile")); // -Dir=C:\oest\oile
            args = IceUtilInternal.Options.Split("-Dir=$'C:\\oest\\oilewith\"quote'"); // -Dir=$'C:\oest\oilewith"quote'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\oest\\oilewith\"quote")); // -Dir=C:\oest\oilewith"quote
            args = IceUtilInternal.Options.Split("-Dir=$'\\103\\072\\134\\164\\145\\163\\164\\134\\146\\151\\154\\145'");
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtilInternal.Options.Split("-Dir=$'\\x43\\x3A\\x5C\\x74\\x65\\x73\\x74\\x5C\\x66\\x69\\x6C\\x65'");
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtilInternal.Options.Split("-Dir=$'\\cM\\c_'"); // Control characters
            Assert(args.Length == 1 && args[0].Equals("-Dir=\x0D\x1F"));
            args = IceUtilInternal.Options.Split("-Dir=$'C:\\\\\\146\\x66\\cMi'"); // -Dir=$'C:\\\146\x66i\cMi'
            Assert(args.Length == 1 && args[0].Equals("-Dir=C:\\ff\x0Di"));
            args = IceUtilInternal.Options.Split("-Dir=$'C:\\\\\\cM\\x66\\146i'"); // -Dir=$'C:\\\cM\x66\146i'
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
                IceUtilInternal.Options.Split(badQuoteCommands[i]);
                Assert(false);
            }
            catch (FormatException)
            {
            }
        }

        Console.Out.WriteLine("ok");

        Console.Out.Write("checking string splitting... ");
        Console.Out.Flush();
        {
            string[]? arr;

            arr = IceUtilInternal.StringUtil.SplitString("", "");
            Assert(arr != null && arr.Length == 0);
            arr = IceUtilInternal.StringUtil.SplitString("", ":");
            Assert(arr != null && arr.Length == 0);
            arr = IceUtilInternal.StringUtil.SplitString("a", "");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("a"));
            arr = IceUtilInternal.StringUtil.SplitString("a", ":");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("a"));
            arr = IceUtilInternal.StringUtil.SplitString("ab", "");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("ab"));
            arr = IceUtilInternal.StringUtil.SplitString("ab:", ":");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("ab"));
            arr = IceUtilInternal.StringUtil.SplitString(":ab", ":");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("ab"));
            arr = IceUtilInternal.StringUtil.SplitString("a:b", ":");
            Assert(arr != null && arr.Length == 2 && arr[0].Equals("a") && arr[1].Equals("b"));
            arr = IceUtilInternal.StringUtil.SplitString(":a:b:", ":");
            Assert(arr != null && arr.Length == 2 && arr[0].Equals("a") && arr[1].Equals("b"));

            arr = IceUtilInternal.StringUtil.SplitString("\"a\"", ":");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("a"));
            arr = IceUtilInternal.StringUtil.SplitString("\"a\":b", ":");
            Assert(arr != null && arr.Length == 2 && arr[0].Equals("a") && arr[1].Equals("b"));
            arr = IceUtilInternal.StringUtil.SplitString("\"a\":\"b\"", ":");
            Assert(arr != null && arr.Length == 2 && arr[0].Equals("a") && arr[1].Equals("b"));
            arr = IceUtilInternal.StringUtil.SplitString("\"a:b\"", ":");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("a:b"));
            arr = IceUtilInternal.StringUtil.SplitString("a=\"a:b\"", ":")!;
            Assert(arr.Length == 1 && arr[0].Equals("a=a:b"));

            arr = IceUtilInternal.StringUtil.SplitString("'a'", ":");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("a"));
            arr = IceUtilInternal.StringUtil.SplitString("'\"a'", ":");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("\"a"));
            arr = IceUtilInternal.StringUtil.SplitString("\"'a\"", ":");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("'a"));

            arr = IceUtilInternal.StringUtil.SplitString("a\\'b", ":");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("a'b"));
            arr = IceUtilInternal.StringUtil.SplitString("'a:b\\'c'", ":");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("a:b'c"));
            arr = IceUtilInternal.StringUtil.SplitString("a\\\"b", ":");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("a\"b"));
            arr = IceUtilInternal.StringUtil.SplitString("\"a:b\\\"c\"", ":");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("a:b\"c"));
            arr = IceUtilInternal.StringUtil.SplitString("'a:b\"c'", ":");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("a:b\"c"));
            arr = IceUtilInternal.StringUtil.SplitString("\"a:b'c\"", ":");
            Assert(arr != null && arr.Length == 1 && arr[0].Equals("a:b'c"));

            Assert(IceUtilInternal.StringUtil.SplitString("a\"b", ":") == null);
        }
        Console.Out.WriteLine("ok");
    }

    public static int Main(string[] args) => Test.TestDriver.RunTest<Client>(args);
}
