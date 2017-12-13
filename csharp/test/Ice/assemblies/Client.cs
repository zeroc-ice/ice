// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Linq;
using System.IO;
using System.Reflection;
using System.Collections.Generic;

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
        int status = 0;
        try
        {
            Console.Out.Write("testing preloading assemblies... ");
            Console.Out.Flush();
            User.UserInfo info = new User.UserInfo();
            Ice.InitializationData id = new Ice.InitializationData();
            id.properties = Ice.Util.createProperties();
            id.properties.setProperty("Ice.PreloadAssemblies", "0");

            string assembly =
                String.Format("{0}/core.dll",
                              Path.GetFileName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().CodeBase)));
            Ice.Communicator ic = Ice.Util.initialize(id);
            test(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault((e) =>
                    {
                        return e.CodeBase.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase);
                    }) == null);
            ic.destroy();
            id.properties.setProperty("Ice.PreloadAssemblies", "1");
            ic = Ice.Util.initialize(id);
            test(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault((e) =>
                    {
                        return e.CodeBase.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase);
                    }) != null);
            Console.Out.WriteLine("ok");
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine(ex);
            status = 1;
        }
        return status;
    }
}
