// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Demo;

public class Client : Ice.Application
{
    private static void menu()
    {
        Console.WriteLine(
            "usage:\n" +
            "1: print string\n" +
            "2: print string sequence\n" +
            "3: print dictionary\n" +
            "4: print enum\n" +
            "5: print struct\n" +
            "6: print struct sequence\n" +
            "7: print class\n" +
            "8: get values\n" +
            "9: throw exception\n" +
            "s: shutdown server\n" +
            "x: exit\n" +
            "?: help\n");
    }

    public override int
    run(string[] args)
    {
        Ice.ObjectPrx obj = communicator().propertyToProxy("Printer.Proxy");

        menu();

        string line = null;
        do
        {
            try
            {
                Console.Write("==> ");
                Console.Out.Flush();
                line = Console.In.ReadLine();
                if(line == null)
                {
                    break;
                }

                byte[] outParams;

                if(line.Equals("1"))
                {
                    //
                    // Marshal the in parameter.
                    //
                    Ice.OutputStream outStream = Ice.Util.createOutputStream(communicator());
                    outStream.writeString("The streaming API works!");

                    //
                    // Invoke operation.
                    //
                    if(!obj.ice_invoke("printString", Ice.OperationMode.Normal, outStream.finished(), out outParams))
                    {
                        Console.Error.WriteLine("Unknown user exception");
                    }

                    outStream.destroy();
                }
                else if(line.Equals("2"))
                {
                    //
                    // Marshal the in parameter.
                    //
                    Ice.OutputStream outStream = Ice.Util.createOutputStream(communicator());
                    string[] arr = { "The", "streaming", "API", "works!" };
                    Demo.StringSeqHelper.write(outStream, arr);

                    //
                    // Invoke operation.
                    //
                    if(!obj.ice_invoke("printStringSequence", Ice.OperationMode.Normal, outStream.finished(), out outParams))
                    {
                        Console.Error.WriteLine("Unknown user exception");
                    }

                    outStream.destroy();
                }
                else if(line.Equals("3"))
                {
                    //
                    // Marshal the in parameter.
                    //
                    Ice.OutputStream outStream = Ice.Util.createOutputStream(communicator());
                    StringDict dict = new StringDict();
                    dict["The"] = "streaming";
                    dict["API"] = "works!";
                    Demo.StringDictHelper.write(outStream, dict);

                    //
                    // Invoke operation.
                    //
                    if(!obj.ice_invoke("printDictionary", Ice.OperationMode.Normal, outStream.finished(), out outParams))
                    {
                        Console.Error.WriteLine("Unknown user exception");
                    }

                    outStream.destroy();
                }
                else if(line.Equals("4"))
                {
                    //
                    // Marshal the in parameter.
                    //
                    Ice.OutputStream outStream = Ice.Util.createOutputStream(communicator());
                    Demo.ColorHelper.write(outStream, Demo.Color.green);

                    //
                    // Invoke operation.
                    //
                    if(!obj.ice_invoke("printEnum", Ice.OperationMode.Normal, outStream.finished(), out outParams))
                    {
                        Console.Error.WriteLine("Unknown user exception");
                    }

                    outStream.destroy();
                }
                else if(line.Equals("5"))
                {
                    //
                    // Marshal the in parameter.
                    //
                    Ice.OutputStream outStream = Ice.Util.createOutputStream(communicator());
                    Demo.Structure s = new Demo.Structure();
                    s.name = "red";
                    s.value = Demo.Color.red;
                    s.ice_write(outStream);

                    //
                    // Invoke operation.
                    //
                    if(!obj.ice_invoke("printStruct", Ice.OperationMode.Normal, outStream.finished(), out outParams))
                    {
                        Console.Error.WriteLine("Unknown user exception");
                    }

                    outStream.destroy();
                }
                else if(line.Equals("6"))
                {
                    //
                    // Marshal the in parameter.
                    //
                    Ice.OutputStream outStream = Ice.Util.createOutputStream(communicator());
                    Demo.Structure[] arr = new Demo.Structure[3];
                    arr[0] = new Demo.Structure();
                    arr[0].name = "red";
                    arr[0].value = Demo.Color.red;
                    arr[1] = new Demo.Structure();
                    arr[1].name = "green";
                    arr[1].value = Demo.Color.green;
                    arr[2] = new Demo.Structure();
                    arr[2].name = "blue";
                    arr[2].value = Demo.Color.blue;
                    Demo.StructureSeqHelper.write(outStream, arr);

                    //
                    // Invoke operation.
                    //
                    if(!obj.ice_invoke("printStructSequence", Ice.OperationMode.Normal, outStream.finished(),
                                       out outParams))
                    {
                        Console.Error.WriteLine("Unknown user exception");
                    }

                    outStream.destroy();
                }
                else if(line.Equals("7"))
                {
                    //
                    // Marshal the in parameter.
                    //
                    Ice.OutputStream outStream = Ice.Util.createOutputStream(communicator());
                    Demo.C c = new Demo.C();
                    c.s = new Demo.Structure();
                    c.s.name = "blue";
                    c.s.value = Demo.Color.blue;
                    Demo.CHelper.write(outStream, c);
                    outStream.writePendingObjects();

                    //
                    // Invoke operation.
                    //
                    if(!obj.ice_invoke("printClass", Ice.OperationMode.Normal, outStream.finished(), out outParams))
                    {
                        Console.Error.WriteLine("Unknown user exception");
                    }

                    outStream.destroy();
                }
                else if(line.Equals("8"))
                {
                    //
                    // Invoke operation.
                    //
                    if(!obj.ice_invoke("getValues", Ice.OperationMode.Normal, null, out outParams))
                    {
                        Console.Error.WriteLine("Unknown user exception");
                        continue;
                    }

                    //
                    // Unmarshal the results.
                    //
                    Ice.InputStream inStream = Ice.Util.createInputStream(communicator(), outParams);
                    Demo.CHelper ch = new Demo.CHelper(inStream);
                    ch.read();
                    String str = inStream.readString();
                    inStream.readPendingObjects();
                    inStream.destroy();
                    Demo.C c = ch.value;
                    Console.Error.WriteLine("Got string `" + str + "' and class: s.name=" + c.s.name +
                                            ", s.value=" + c.s.value);
                }
                else if(line.Equals("9"))
                {
                    //
                    // Invoke operation.
                    //
                    if(obj.ice_invoke("throwPrintFailure", Ice.OperationMode.Normal, null, out outParams))
                    {
                        Console.Error.WriteLine("Expected exception");
                        continue;
                    }

                    Ice.InputStream inStream = Ice.Util.createInputStream(communicator(), outParams);
                    try
                    {
                        inStream.throwException();
                    }
                    catch(Demo.PrintFailure)
                    {
                        // Expected.
                    }
                    catch(Ice.UserException)
                    {
                        Console.Error.WriteLine("Unknown user exception");
                    }
                    inStream.destroy();
                }
                else if(line.Equals("s"))
                {
                    obj.ice_invoke("shutdown", Ice.OperationMode.Normal, null, out outParams);
                }
                else if(line.Equals("x"))
                {
                    // Nothing to do.
                }
                else if(line.Equals("?"))
                {
                    menu();
                }
                else
                {
                    Console.Error.WriteLine("unknown command `" + line + "'");
                    menu();
                }
            }
            catch(Ice.LocalException ex)
            {
                Console.Error.WriteLine(ex);
            }
        }
        while(!line.Equals("x"));

        return 0;
    }

    public static void Main(string[] args)
    {
        Client app = new Client();
        int status = app.main(args, "config.client");
        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
