//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.IceUtil.inputUtil;

public class Client extends test.TestHelper
{
    public void run(String[] argvs)
    {
        System.out.print("testing string to command line arguments... ");
        System.out.flush();
        String[] args;

        try
        {
            test(IceUtilInternal.Options.split("").length == 0);

            args = IceUtilInternal.Options.split("\"\"");
            test(args.length == 1 && args[0].equals(""));
            args = IceUtilInternal.Options.split("''");
            test(args.length == 1 && args[0].equals(""));
            args = IceUtilInternal.Options.split("$''");
            test(args.length == 1 && args[0].equals(""));

            args = IceUtilInternal.Options.split("-a -b -c");
            test(args.length == 3 && args[0].equals("-a") && args[1].equals("-b") && args[2].equals("-c"));
            args = IceUtilInternal.Options.split("\"-a\" '-b' $'-c'");
            test(args.length == 3 && args[0].equals("-a") && args[1].equals("-b") && args[2].equals("-c"));
            args = IceUtilInternal.Options.split("  '-b' \"-a\" $'-c' ");
            test(args.length == 3 && args[0].equals("-b") && args[1].equals("-a") && args[2].equals("-c"));
            args = IceUtilInternal.Options.split(" $'-c' '-b' \"-a\"  ");
            test(args.length == 3 && args[0].equals("-c") && args[1].equals("-b") && args[2].equals("-a"));

            // Testing single quote
            args = IceUtilInternal.Options.split("-Dir='C:\\\\test\\\\file'"); // -Dir='C:\\test\\file'
            test(args.length == 1 && args[0].equals("-Dir=C:\\\\test\\\\file")); // -Dir=C:\\test\\file
            args = IceUtilInternal.Options.split("-Dir='C:\\test\\file'"); // -Dir='C:\test\file'
            test(args.length == 1 && args[0].equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtilInternal.Options.split("-Dir='C:\\test\\filewith\"quote'"); // -Dir='C:\test\filewith"quote'
            test(args.length == 1 && args[0].equals("-Dir=C:\\test\\filewith\"quote")); // -Dir=C:\test\filewith"quote

            // Testing double quote
            args = IceUtilInternal.Options.split("-Dir=\"C:\\\\test\\\\file\""); // -Dir="C:\\test\\file"
            test(args.length == 1 && args[0].equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
                 args = IceUtilInternal.Options.split("-Dir=\"C:\\test\\file\""); // -Dir="C:\test\file"
                 test(args.length == 1 && args[0].equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtilInternal.Options.split("-Dir=\"C:\\test\\filewith\\\"quote\""); // -Dir="C:\test\filewith\"quote"
            test(args.length == 1 && args[0].equals("-Dir=C:\\test\\filewith\"quote")); // -Dir=C:\test\filewith"quote

            // Testing ANSI quote
            args = IceUtilInternal.Options.split("-Dir=$'C:\\\\test\\\\file'"); // -Dir=$'C:\\test\\file'
            test(args.length == 1 && args[0].equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtilInternal.Options.split("-Dir=$'C:\\oest\\oile'"); // -Dir='C:\oest\oile'
            test(args.length == 1 && args[0].equals("-Dir=C:\\oest\\oile")); // -Dir=C:\oest\oile
            args = IceUtilInternal.Options.split("-Dir=$'C:\\oest\\oilewith\"quote'"); // -Dir=$'C:\oest\oilewith"quote'
            test(args.length == 1 && args[0].equals("-Dir=C:\\oest\\oilewith\"quote")); // -Dir=C:\oest\oilewith"quote
            args = IceUtilInternal.Options.split("-Dir=$'\\103\\072\\134\\164\\145\\163\\164\\134\\146\\151\\154\\145'");
            test(args.length == 1 && args[0].equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtilInternal.Options.split("-Dir=$'\\x43\\x3A\\x5C\\x74\\x65\\x73\\x74\\x5C\\x66\\x69\\x6C\\x65'");
            test(args.length == 1 && args[0].equals("-Dir=C:\\test\\file")); // -Dir=C:\test\file
            args = IceUtilInternal.Options.split("-Dir=$'\\cM\\c_'"); // Control characters
            test(args.length == 1 && args[0].equals("-Dir=\015\037"));
            args = IceUtilInternal.Options.split("-Dir=$'C:\\\\\\146\\x66\\cMi'"); // -Dir=$'C:\\\146\x66i\cMi'
            test(args.length == 1 && args[0].equals("-Dir=C:\\ff\015i"));
            args = IceUtilInternal.Options.split("-Dir=$'C:\\\\\\cM\\x66\\146i'"); // -Dir=$'C:\\\cM\x66\146i'
            test(args.length == 1 && args[0].equals("-Dir=C:\\\015ffi"));
        }
        catch(IceUtilInternal.Options.BadQuote ex)
        {
            test(false);
        }

        String[] badQuoteCommands = new String[6];
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
            catch(IceUtilInternal.Options.BadQuote ex)
            {
            }
        }

        System.out.println("ok");

        System.out.print("checking string splitting... ");
        System.out.flush();
        {
            String[] arr;

            arr = IceUtilInternal.StringUtil.splitString("", "");
            test(arr.length == 0);
            arr = IceUtilInternal.StringUtil.splitString("", ":");
            test(arr.length == 0);
            arr = IceUtilInternal.StringUtil.splitString("a", "");
            test(arr.length == 1 && arr[0].equals("a"));
            arr = IceUtilInternal.StringUtil.splitString("a", ":");
            test(arr.length == 1 && arr[0].equals("a"));
            arr = IceUtilInternal.StringUtil.splitString("ab", "");
            test(arr.length == 1 && arr[0].equals("ab"));
            arr = IceUtilInternal.StringUtil.splitString("ab:", ":");
            test(arr.length == 1 && arr[0].equals("ab"));
            arr = IceUtilInternal.StringUtil.splitString(":ab", ":");
            test(arr.length == 1 && arr[0].equals("ab"));
            arr = IceUtilInternal.StringUtil.splitString("a:b", ":");
            test(arr.length == 2 && arr[0].equals("a") && arr[1].equals("b"));
            arr = IceUtilInternal.StringUtil.splitString(":a:b:", ":");
            test(arr.length == 2 && arr[0].equals("a") && arr[1].equals("b"));

            arr = IceUtilInternal.StringUtil.splitString("\"a\"", ":");
            test(arr.length == 1 && arr[0].equals("a"));
            arr = IceUtilInternal.StringUtil.splitString("\"a\":b", ":");
            test(arr.length == 2 && arr[0].equals("a") && arr[1].equals("b"));
            arr = IceUtilInternal.StringUtil.splitString("\"a\":\"b\"", ":");
            test(arr.length == 2 && arr[0].equals("a") && arr[1].equals("b"));
            arr = IceUtilInternal.StringUtil.splitString("\"a:b\"", ":");
            test(arr.length == 1 && arr[0].equals("a:b"));
            arr = IceUtilInternal.StringUtil.splitString("a=\"a:b\"", ":");
            test(arr.length == 1 && arr[0].equals("a=a:b"));

            arr = IceUtilInternal.StringUtil.splitString("'a'", ":");
            test(arr.length == 1 && arr[0].equals("a"));
            arr = IceUtilInternal.StringUtil.splitString("'\"a'", ":");
            test(arr.length == 1 && arr[0].equals("\"a"));
            arr = IceUtilInternal.StringUtil.splitString("\"'a\"", ":");
            test(arr.length == 1 && arr[0].equals("'a"));

            arr = IceUtilInternal.StringUtil.splitString("a\\'b", ":");
            test(arr.length == 1 && arr[0].equals("a'b"));
            arr = IceUtilInternal.StringUtil.splitString("'a:b\\'c'", ":");
            test(arr.length == 1 && arr[0].equals("a:b'c"));
            arr = IceUtilInternal.StringUtil.splitString("a\\\"b", ":");
            test(arr.length == 1 && arr[0].equals("a\"b"));
            arr = IceUtilInternal.StringUtil.splitString("\"a:b\\\"c\"", ":");
            test(arr.length == 1 && arr[0].equals("a:b\"c"));
            arr = IceUtilInternal.StringUtil.splitString("'a:b\"c'", ":");
            test(arr.length == 1 && arr[0].equals("a:b\"c"));
            arr = IceUtilInternal.StringUtil.splitString("\"a:b'c\"", ":");
            test(arr.length == 1 && arr[0].equals("a:b'c"));

            test(IceUtilInternal.StringUtil.splitString("a\"b", ":") == null);
        }
        System.out.println("ok");
    }
}
