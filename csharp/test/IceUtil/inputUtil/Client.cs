// Copyright (c) ZeroC, Inc.

namespace IceUtil.inputUtil;

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        Console.Out.Write("testing string to command line arguments... ");
        Console.Out.Flush();
        string[] arguments;

        try
        {
            test(Ice.UtilInternal.Options.split("").Length == 0);

            arguments = Ice.UtilInternal.Options.split("\"\"");
            test(arguments.Length == 1 && arguments[0].Length == 0);
            arguments = Ice.UtilInternal.Options.split("''");
            test(arguments.Length == 1 && arguments[0].Length == 0);
            arguments = Ice.UtilInternal.Options.split("$''");
            test(arguments.Length == 1 && arguments[0].Length == 0);

            arguments = Ice.UtilInternal.Options.split("-a -b -c");
            test(arguments.Length == 3 && arguments[0] == "-a" && arguments[1] == "-b" && arguments[2] == "-c");
            arguments = Ice.UtilInternal.Options.split("\"-a\" '-b' $'-c'");
            test(arguments.Length == 3 && arguments[0] == "-a" && arguments[1] == "-b" && arguments[2] == "-c");
            arguments = Ice.UtilInternal.Options.split("  '-b' \"-a\" $'-c' ");
            test(arguments.Length == 3 && arguments[0] == "-b" && arguments[1] == "-a" && arguments[2] == "-c");
            arguments = Ice.UtilInternal.Options.split(" $'-c' '-b' \"-a\"  ");
            test(arguments.Length == 3 && arguments[0] == "-c" && arguments[1] == "-b" && arguments[2] == "-a");

            // Testing single quote
            arguments = Ice.UtilInternal.Options.split("-Dir='C:\\\\test\\\\file'"); // -Dir='C:\\test\\file'
            test(arguments.Length == 1 && arguments[0] == "-Dir=C:\\\\test\\\\file"); // -Dir=C:\\test\\file
            arguments = Ice.UtilInternal.Options.split("-Dir='C:\\test\\file'"); // -Dir='C:\test\file'
            test(arguments.Length == 1 && arguments[0] == "-Dir=C:\\test\\file"); // -Dir=C:\test\file
            arguments = Ice.UtilInternal.Options.split("-Dir='C:\\test\\filewith\"quote'"); // -Dir='C:\test\filewith"quote'
            test(arguments.Length == 1 && arguments[0] == "-Dir=C:\\test\\filewith\"quote"); // -Dir=C:\test\filewith"quote

            // Testing double quote
            arguments = Ice.UtilInternal.Options.split("-Dir=\"C:\\\\test\\\\file\""); // -Dir="C:\\test\\file"
            test(arguments.Length == 1 && arguments[0] == "-Dir=C:\\test\\file"); // -Dir=C:\test\file
            arguments = Ice.UtilInternal.Options.split("-Dir=\"C:\\test\\file\""); // -Dir="C:\test\file"
            test(arguments.Length == 1 && arguments[0] == "-Dir=C:\\test\\file"); // -Dir=C:\test\file
            arguments = Ice.UtilInternal.Options.split("-Dir=\"C:\\test\\filewith\\\"quote\""); // -Dir="C:\test\filewith\"quote"
            test(arguments.Length == 1 && arguments[0] == "-Dir=C:\\test\\filewith\"quote"); // -Dir=C:\test\filewith"quote

            // Testing ANSI quote
            arguments = Ice.UtilInternal.Options.split("-Dir=$'C:\\\\test\\\\file'"); // -Dir=$'C:\\test\\file'
            test(arguments.Length == 1 && arguments[0] == "-Dir=C:\\test\\file"); // -Dir=C:\test\file
            arguments = Ice.UtilInternal.Options.split("-Dir=$'C:\\oest\\oile'"); // -Dir='C:\oest\oile'
            test(arguments.Length == 1 && arguments[0] == "-Dir=C:\\oest\\oile"); // -Dir=C:\oest\oile
            arguments = Ice.UtilInternal.Options.split("-Dir=$'C:\\oest\\oilewith\"quote'"); // -Dir=$'C:\oest\oilewith"quote'
            test(arguments.Length == 1 && arguments[0] == "-Dir=C:\\oest\\oilewith\"quote"); // -Dir=C:\oest\oilewith"quote
            arguments = Ice.UtilInternal.Options.split(
                "-Dir=$'\\103\\072\\134\\164\\145\\163\\164\\134\\146\\151\\154\\145'");
            test(arguments.Length == 1 && arguments[0] == "-Dir=C:\\test\\file"); // -Dir=C:\test\file
            arguments = Ice.UtilInternal.Options.split(
                "-Dir=$'\\x43\\x3A\\x5C\\x74\\x65\\x73\\x74\\x5C\\x66\\x69\\x6C\\x65'");
            test(arguments.Length == 1 && arguments[0] == "-Dir=C:\\test\\file"); // -Dir=C:\test\file
            arguments = Ice.UtilInternal.Options.split("-Dir=$'\\cM\\c_'"); // Control characters
            test(arguments.Length == 1 && arguments[0] == "-Dir=\x0D\x1F");
            arguments = Ice.UtilInternal.Options.split("-Dir=$'C:\\\\\\146\\x66\\cMi'"); // -Dir=$'C:\\\146\x66i\cMi'
            test(arguments.Length == 1 && arguments[0] == "-Dir=C:\\ff\x0Di");
            arguments = Ice.UtilInternal.Options.split("-Dir=$'C:\\\\\\cM\\x66\\146i'"); // -Dir=$'C:\\\cM\x66\146i'
            test(arguments.Length == 1 && arguments[0] == "-Dir=C:\\\x000Dffi");
        }
        catch (Ice.ParseException)
        {
            test(false);
        }

        string[] badQuoteCommands =
        [
            "\"",
            "'",
            "\\$'",
            "-Dir=\"test",
            "-Dir='test",
            "-Dir=$'test",
        ];

        for (int i = 0; i < 6; ++i)
        {
            try
            {
                Ice.UtilInternal.Options.split(badQuoteCommands[i]);
                test(false);
            }
            catch (Ice.ParseException)
            {
            }
        }

        Console.Out.WriteLine("ok");

        Console.Out.Write("checking string splitting... ");
        Console.Out.Flush();
        {
            string[] arr;

            arr = Ice.UtilInternal.StringUtil.splitString("", "");
            test(arr.Length == 0);
            arr = Ice.UtilInternal.StringUtil.splitString("", ":");
            test(arr.Length == 0);
            arr = Ice.UtilInternal.StringUtil.splitString("a", "");
            test(arr.Length == 1 && arr[0] == "a");
            arr = Ice.UtilInternal.StringUtil.splitString("a", ":");
            test(arr.Length == 1 && arr[0] == "a");
            arr = Ice.UtilInternal.StringUtil.splitString("ab", "");
            test(arr.Length == 1 && arr[0] == "ab");
            arr = Ice.UtilInternal.StringUtil.splitString("ab:", ":");
            test(arr.Length == 1 && arr[0] == "ab");
            arr = Ice.UtilInternal.StringUtil.splitString(":ab", ":");
            test(arr.Length == 1 && arr[0] == "ab");
            arr = Ice.UtilInternal.StringUtil.splitString("a:b", ":");
            test(arr.Length == 2 && arr[0] == "a" && arr[1] == "b");
            arr = Ice.UtilInternal.StringUtil.splitString(":a:b:", ":");
            test(arr.Length == 2 && arr[0] == "a" && arr[1] == "b");

            arr = Ice.UtilInternal.StringUtil.splitString("\"a\"", ":");
            test(arr.Length == 1 && arr[0] == "a");
            arr = Ice.UtilInternal.StringUtil.splitString("\"a\":b", ":");
            test(arr.Length == 2 && arr[0] == "a" && arr[1] == "b");
            arr = Ice.UtilInternal.StringUtil.splitString("\"a\":\"b\"", ":");
            test(arr.Length == 2 && arr[0] == "a" && arr[1] == "b");
            arr = Ice.UtilInternal.StringUtil.splitString("\"a:b\"", ":");
            test(arr.Length == 1 && arr[0] == "a:b");
            arr = Ice.UtilInternal.StringUtil.splitString("a=\"a:b\"", ":");
            test(arr.Length == 1 && arr[0] == "a=a:b");

            arr = Ice.UtilInternal.StringUtil.splitString("'a'", ":");
            test(arr.Length == 1 && arr[0] == "a");
            arr = Ice.UtilInternal.StringUtil.splitString("'\"a'", ":");
            test(arr.Length == 1 && arr[0] == "\"a");
            arr = Ice.UtilInternal.StringUtil.splitString("\"'a\"", ":");
            test(arr.Length == 1 && arr[0] == "'a");

            arr = Ice.UtilInternal.StringUtil.splitString("a\\'b", ":");
            test(arr.Length == 1 && arr[0] == "a'b");
            arr = Ice.UtilInternal.StringUtil.splitString("'a:b\\'c'", ":");
            test(arr.Length == 1 && arr[0] == "a:b'c");
            arr = Ice.UtilInternal.StringUtil.splitString("a\\\"b", ":");
            test(arr.Length == 1 && arr[0] == "a\"b");
            arr = Ice.UtilInternal.StringUtil.splitString("\"a:b\\\"c\"", ":");
            test(arr.Length == 1 && arr[0] == "a:b\"c");
            arr = Ice.UtilInternal.StringUtil.splitString("'a:b\"c'", ":");
            test(arr.Length == 1 && arr[0] == "a:b\"c");
            arr = Ice.UtilInternal.StringUtil.splitString("\"a:b'c\"", ":");
            test(arr.Length == 1 && arr[0] == "a:b'c");

            test(Ice.UtilInternal.StringUtil.splitString("a\"b", ":") == null);
        }
        Console.Out.WriteLine("ok");
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
