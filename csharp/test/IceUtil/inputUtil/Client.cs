//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;

public class Client : Test.TestHelper
{
    public override void run(string[] argvs)
    {
        Console.Out.Write("testing string to command line arguments... ");
        Console.Out.Flush();
        string[] args;

        try
        {
            test(IceUtilInternal.Options.split("").Length == 0);

            args = IceUtilInternal.Options.split("\"\"");
            test(args.Length == 1 && args[0].Length == 0);
            args = IceUtilInternal.Options.split("''");
            test(args.Length == 1 && args[0].Length == 0);
            args = IceUtilInternal.Options.split("$''");
            test(args.Length == 1 && args[0].Length == 0);

            args = IceUtilInternal.Options.split("-a -b -c");
            test(args.Length == 3 && args[0] == "-a" && args[1] == "-b" && args[2] == "-c");
            args = IceUtilInternal.Options.split("\"-a\" '-b' $'-c'");
            test(args.Length == 3 && args[0] == "-a" && args[1] == "-b" && args[2] == "-c");
            args = IceUtilInternal.Options.split("  '-b' \"-a\" $'-c' ");
            test(args.Length == 3 && args[0] == "-b" && args[1] == "-a" && args[2] == "-c");
            args = IceUtilInternal.Options.split(" $'-c' '-b' \"-a\"  ");
            test(args.Length == 3 && args[0] == "-c" && args[1] == "-b" && args[2] == "-a");

            // Testing single quote
            args = IceUtilInternal.Options.split("-Dir='C:\\\\test\\\\file'"); // -Dir='C:\\test\\file'
            test(args.Length == 1 && args[0] == "-Dir=C:\\\\test\\\\file"); // -Dir=C:\\test\\file
            args = IceUtilInternal.Options.split("-Dir='C:\\test\\file'"); // -Dir='C:\test\file'
            test(args.Length == 1 && args[0] == "-Dir=C:\\test\\file"); // -Dir=C:\test\file
            args = IceUtilInternal.Options.split("-Dir='C:\\test\\filewith\"quote'"); // -Dir='C:\test\filewith"quote'
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\filewith\"quote")); // -Dir=C:\test\filewith"quote

            // Testing double quote
            args = IceUtilInternal.Options.split("-Dir=\"C:\\\\test\\\\file\""); // -Dir="C:\\test\\file"
            test(args.Length == 1 && args[0] == "-Dir=C:\\test\\file"); // -Dir=C:\test\file
                 args = IceUtilInternal.Options.split("-Dir=\"C:\\test\\file\""); // -Dir="C:\test\file"
                 test(args.Length == 1 && args[0] == "-Dir=C:\\test\\file"); // -Dir=C:\test\file
            args = IceUtilInternal.Options.split("-Dir=\"C:\\test\\filewith\\\"quote\""); // -Dir="C:\test\filewith\"quote"
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\test\\filewith\"quote")); // -Dir=C:\test\filewith"quote

            // Testing ANSI quote
            args = IceUtilInternal.Options.split("-Dir=$'C:\\\\test\\\\file'"); // -Dir=$'C:\\test\\file'
            test(args.Length == 1 && args[0] == "-Dir=C:\\test\\file"); // -Dir=C:\test\file
            args = IceUtilInternal.Options.split("-Dir=$'C:\\oest\\oile'"); // -Dir='C:\oest\oile'
            test(args.Length == 1 && args[0] == "-Dir=C:\\oest\\oile"); // -Dir=C:\oest\oile
            args = IceUtilInternal.Options.split("-Dir=$'C:\\oest\\oilewith\"quote'"); // -Dir=$'C:\oest\oilewith"quote'
            test(args.Length == 1 && args[0].Equals("-Dir=C:\\oest\\oilewith\"quote")); // -Dir=C:\oest\oilewith"quote
            args = IceUtilInternal.Options.split("-Dir=$'\\103\\072\\134\\164\\145\\163\\164\\134\\146\\151\\154\\145'");
            test(args.Length == 1 && args[0] == "-Dir=C:\\test\\file"); // -Dir=C:\test\file
            args = IceUtilInternal.Options.split("-Dir=$'\\x43\\x3A\\x5C\\x74\\x65\\x73\\x74\\x5C\\x66\\x69\\x6C\\x65'");
            test(args.Length == 1 && args[0] == "-Dir=C:\\test\\file"); // -Dir=C:\test\file
            args = IceUtilInternal.Options.split("-Dir=$'\\cM\\c_'"); // Control characters
            test(args.Length == 1 && args[0] == "-Dir=\x0D\x1F");
            args = IceUtilInternal.Options.split("-Dir=$'C:\\\\\\146\\x66\\cMi'"); // -Dir=$'C:\\\146\x66i\cMi'
            test(args.Length == 1 && args[0] == "-Dir=C:\\ff\x0Di");
            args = IceUtilInternal.Options.split("-Dir=$'C:\\\\\\cM\\x66\\146i'"); // -Dir=$'C:\\\cM\x66\146i'
            test(args.Length == 1 && args[0] == "-Dir=C:\\\x000Dffi");
        }
        catch(IceUtilInternal.Options.BadQuote)
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
                IceUtilInternal.Options.split(badQuoteCommands[i]);
                test(false);
            }
            catch(IceUtilInternal.Options.BadQuote)
            {
            }
        }

        Console.Out.WriteLine("ok");

        Console.Out.Write("checking string splitting... ");
        Console.Out.Flush();
        {
            string[] arr;

            arr = IceUtilInternal.StringUtil.splitString("", "");
            test(arr.Length == 0);
            arr = IceUtilInternal.StringUtil.splitString("", ":");
            test(arr.Length == 0);
            arr = IceUtilInternal.StringUtil.splitString("a", "");
            test(arr.Length == 1 && arr[0] == "a");
            arr = IceUtilInternal.StringUtil.splitString("a", ":");
            test(arr.Length == 1 && arr[0] == "a");
            arr = IceUtilInternal.StringUtil.splitString("ab", "");
            test(arr.Length == 1 && arr[0] == "ab");
            arr = IceUtilInternal.StringUtil.splitString("ab:", ":");
            test(arr.Length == 1 && arr[0] == "ab");
            arr = IceUtilInternal.StringUtil.splitString(":ab", ":");
            test(arr.Length == 1 && arr[0] == "ab");
            arr = IceUtilInternal.StringUtil.splitString("a:b", ":");
            test(arr.Length == 2 && arr[0] == "a" && arr[1] == "b");
            arr = IceUtilInternal.StringUtil.splitString(":a:b:", ":");
            test(arr.Length == 2 && arr[0] == "a" && arr[1] == "b");

            arr = IceUtilInternal.StringUtil.splitString("\"a\"", ":");
            test(arr.Length == 1 && arr[0] == "a");
            arr = IceUtilInternal.StringUtil.splitString("\"a\":b", ":");
            test(arr.Length == 2 && arr[0] == "a" && arr[1] == "b");
            arr = IceUtilInternal.StringUtil.splitString("\"a\":\"b\"", ":");
            test(arr.Length == 2 && arr[0] == "a" && arr[1] == "b");
            arr = IceUtilInternal.StringUtil.splitString("\"a:b\"", ":");
            test(arr.Length == 1 && arr[0] == "a:b");
            arr = IceUtilInternal.StringUtil.splitString("a=\"a:b\"", ":");
            test(arr.Length == 1 && arr[0] == "a=a:b");

            arr = IceUtilInternal.StringUtil.splitString("'a'", ":");
            test(arr.Length == 1 && arr[0] == "a");
            arr = IceUtilInternal.StringUtil.splitString("'\"a'", ":");
            test(arr.Length == 1 && arr[0].Equals("\"a"));
            arr = IceUtilInternal.StringUtil.splitString("\"'a\"", ":");
            test(arr.Length == 1 && arr[0] == "'a");

            arr = IceUtilInternal.StringUtil.splitString("a\\'b", ":");
            test(arr.Length == 1 && arr[0] == "a'b");
            arr = IceUtilInternal.StringUtil.splitString("'a:b\\'c'", ":");
            test(arr.Length == 1 && arr[0] == "a:b'c");
            arr = IceUtilInternal.StringUtil.splitString("a\\\"b", ":");
            test(arr.Length == 1 && arr[0].Equals("a\"b"));
            arr = IceUtilInternal.StringUtil.splitString("\"a:b\\\"c\"", ":");
            test(arr.Length == 1 && arr[0].Equals("a:b\"c"));
            arr = IceUtilInternal.StringUtil.splitString("'a:b\"c'", ":");
            test(arr.Length == 1 && arr[0].Equals("a:b\"c"));
            arr = IceUtilInternal.StringUtil.splitString("\"a:b'c\"", ":");
            test(arr.Length == 1 && arr[0] == "a:b'c");

            test(IceUtilInternal.StringUtil.splitString("a\"b", ":") == null);
        }
        Console.Out.WriteLine("ok");
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
