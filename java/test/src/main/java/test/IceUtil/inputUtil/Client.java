// Copyright (c) ZeroC, Inc.

package test.IceUtil.inputUtil;

import com.zeroc.Ice.Options;
import com.zeroc.Ice.ParseException;
import com.zeroc.Ice.StringUtil;

import test.TestHelper;

public class Client extends TestHelper {
    public void run(String[] argvs) {
        System.out.print("testing string to command line arguments... ");
        System.out.flush();
        String[] args;

        try {
            test(Options.split("").length == 0);

            args = Options.split("\"\"");
            test(args.length == 1 && args[0].isEmpty());
            args = Options.split("''");
            test(args.length == 1 && args[0].isEmpty());
            args = Options.split("$''");
            test(args.length == 1 && args[0].isEmpty());

            args = Options.split("-a -b -c");
            test(
                args.length == 3
                    && "-a".equals(args[0])
                    && "-b".equals(args[1])
                    && "-c".equals(args[2]));
            args = Options.split("\"-a\" '-b' $'-c'");
            test(
                args.length == 3
                    && "-a".equals(args[0])
                    && "-b".equals(args[1])
                    && "-c".equals(args[2]));
            args = Options.split("  '-b' \"-a\" $'-c' ");
            test(
                args.length == 3
                    && "-b".equals(args[0])
                    && "-a".equals(args[1])
                    && "-c".equals(args[2]));
            args = Options.split(" $'-c' '-b' \"-a\"  ");
            test(
                args.length == 3
                    && "-c".equals(args[0])
                    && "-b".equals(args[1])
                    && "-a".equals(args[2]));

            // Testing single quote
            args = Options.split("-Dir='C:\\\\test\\\\file'"); // -Dir='C:\\test\\file'
            test(
                args.length == 1
                    && "-Dir=C:\\\\test\\\\file".equals(args[0])); // -Dir=C:\\test\\file
            args = Options.split("-Dir='C:\\test\\file'"); // -Dir='C:\test\file'
            test(args.length == 1 && "-Dir=C:\\test\\file".equals(args[0])); // -Dir=C:\test\file
            args =
                Options.split(
                    "-Dir='C:\\test\\filewith\"quote'"); // -Dir='C:\test\filewith"quote'
            test(
                args.length == 1
                    && "-Dir=C:\\test\\filewith\"quote".equals(args[0])); // -Dir=C:\test\filewith"quote

            // Testing double quote
            args = Options.split("-Dir=\"C:\\\\test\\\\file\""); // -Dir="C:\\test\\file"
            test(args.length == 1 && "-Dir=C:\\test\\file".equals(args[0])); // -Dir=C:\test\file
            args = Options.split("-Dir=\"C:\\test\\file\""); // -Dir="C:\test\file"
            test(args.length == 1 && "-Dir=C:\\test\\file".equals(args[0])); // -Dir=C:\test\file
            args =
                Options.split(
                    "-Dir=\"C:\\test\\filewith\\\"quote\""); // -Dir="C:\test\filewith\"quote"
            test(
                args.length == 1
                    && "-Dir=C:\\test\\filewith\"quote".equals(args[0])); // -Dir=C:\test\filewith"quote

            // Testing ANSI quote
            args = Options.split("-Dir=$'C:\\\\test\\\\file'"); // -Dir=$'C:\\test\\file'
            test(args.length == 1 && "-Dir=C:\\test\\file".equals(args[0])); // -Dir=C:\test\file
            args = Options.split("-Dir=$'C:\\oest\\oile'"); // -Dir='C:\oest\oile'
            test(args.length == 1 && "-Dir=C:\\oest\\oile".equals(args[0])); // -Dir=C:\oest\oile
            args =
                Options.split(
                    "-Dir=$'C:\\oest\\oilewith\"quote'"); // -Dir=$'C:\oest\oilewith"quote'
            test(
                args.length == 1
                    && "-Dir=C:\\oest\\oilewith\"quote".equals(args[0])); // -Dir=C:\oest\oilewith"quote

            //CHECKSTYLE:OFF: IllegalTokenText
            args =
                Options.split(
                    "-Dir=$'\\103\\072\\134\\164\\145\\163\\164\\134\\146\\151\\154\\145'");
            test(args.length == 1 && "-Dir=C:\\test\\file".equals(args[0])); // -Dir=C:\test\file
            args =
                Options.split(
                    "-Dir=$'\\x43\\x3A\\x5C\\x74\\x65\\x73\\x74\\x5C\\x66\\x69\\x6C\\x65'");
            test(args.length == 1 && "-Dir=C:\\test\\file".equals(args[0])); // -Dir=C:\test\file
            args = Options.split("-Dir=$'\\cM\\c_'"); // Control characters
            test(args.length == 1 && "-Dir=\015\037".equals(args[0]));
            args = Options.split("-Dir=$'C:\\\\\\146\\x66\\cMi'"); // -Dir=$'C:\\\146\x66i\cMi'
            test(args.length == 1 && "-Dir=C:\\ff\015i".equals(args[0]));
            args = Options.split("-Dir=$'C:\\\\\\cM\\x66\\146i'"); // -Dir=$'C:\\\cM\x66\146i'
            test(args.length == 1 && "-Dir=C:\\\015ffi".equals(args[0]));
            //CHECKSTYLE:ON: IllegalTokenText
        } catch (ParseException ex) {
            test(false);
        }

        String[] parseExceptionCommands = new String[6];
        parseExceptionCommands[0] = "\"";
        parseExceptionCommands[1] = "'";
        parseExceptionCommands[2] = "\\$'";
        parseExceptionCommands[3] = "-Dir=\"test";
        parseExceptionCommands[4] = "-Dir='test";
        parseExceptionCommands[5] = "-Dir=$'test";
        for (int i = 0; i < 6; i++) {
            try {
                Options.split(parseExceptionCommands[i]);
                test(false);
            } catch (ParseException ex) {}
        }

        System.out.println("ok");

        System.out.print("checking string splitting... ");
        System.out.flush();
        {
            String[] arr;

            arr = StringUtil.splitString("", "");
            test(arr.length == 0);
            arr = StringUtil.splitString("", ":");
            test(arr.length == 0);
            arr = StringUtil.splitString("a", "");
            test(arr.length == 1 && "a".equals(arr[0]));
            arr = StringUtil.splitString("a", ":");
            test(arr.length == 1 && "a".equals(arr[0]));
            arr = StringUtil.splitString("ab", "");
            test(arr.length == 1 && "ab".equals(arr[0]));
            arr = StringUtil.splitString("ab:", ":");
            test(arr.length == 1 && "ab".equals(arr[0]));
            arr = StringUtil.splitString(":ab", ":");
            test(arr.length == 1 && "ab".equals(arr[0]));
            arr = StringUtil.splitString("a:b", ":");
            test(arr.length == 2 && "a".equals(arr[0]) && "b".equals(arr[1]));
            arr = StringUtil.splitString(":a:b:", ":");
            test(arr.length == 2 && "a".equals(arr[0]) && "b".equals(arr[1]));

            arr = StringUtil.splitString("\"a\"", ":");
            test(arr.length == 1 && "a".equals(arr[0]));
            arr = StringUtil.splitString("\"a\":b", ":");
            test(arr.length == 2 && "a".equals(arr[0]) && "b".equals(arr[1]));
            arr = StringUtil.splitString("\"a\":\"b\"", ":");
            test(arr.length == 2 && "a".equals(arr[0]) && "b".equals(arr[1]));
            arr = StringUtil.splitString("\"a:b\"", ":");
            test(arr.length == 1 && "a:b".equals(arr[0]));
            arr = StringUtil.splitString("a=\"a:b\"", ":");
            test(arr.length == 1 && "a=a:b".equals(arr[0]));

            arr = StringUtil.splitString("'a'", ":");
            test(arr.length == 1 && "a".equals(arr[0]));
            arr = StringUtil.splitString("'\"a'", ":");
            test(arr.length == 1 && "\"a".equals(arr[0]));
            arr = StringUtil.splitString("\"'a\"", ":");
            test(arr.length == 1 && "'a".equals(arr[0]));

            arr = StringUtil.splitString("a\\'b", ":");
            test(arr.length == 1 && "a'b".equals(arr[0]));
            arr = StringUtil.splitString("'a:b\\'c'", ":");
            test(arr.length == 1 && "a:b'c".equals(arr[0]));
            arr = StringUtil.splitString("a\\\"b", ":");
            test(arr.length == 1 && "a\"b".equals(arr[0]));
            arr = StringUtil.splitString("\"a:b\\\"c\"", ":");
            test(arr.length == 1 && "a:b\"c".equals(arr[0]));
            arr = StringUtil.splitString("'a:b\"c'", ":");
            test(arr.length == 1 && "a:b\"c".equals(arr[0]));
            arr = StringUtil.splitString("\"a:b'c\"", ":");
            test(arr.length == 1 && "a:b'c".equals(arr[0]));

            test(StringUtil.splitString("a\"b", ":") == null);
        }
        System.out.println("ok");
    }
}
