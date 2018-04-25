// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;
using System.IO;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    public static int Main(string[] args)
    {
        Console.Out.Write("testing Ice.LogFile... ");
        Console.Out.Flush();
        if(File.Exists("log.txt"))
        {
            File.Delete("log.txt");
        }
        var initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        initData.properties.setProperty("Ice.LogFile", "log.txt");
        using(var communicator = Ice.Util.initialize(initData))
        {
            communicator.getLogger().trace("info", "my logger");
        }
        test(File.Exists("log.txt"));
        test(File.ReadAllText("log.txt").Contains("my logger"));
        File.Delete("log.txt");
        Console.Out.WriteLine("ok");
        return 0;
    }
}
