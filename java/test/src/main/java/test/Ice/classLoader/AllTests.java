// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.classLoader;

import java.io.PrintWriter;

import test.Ice.classLoader.Test.ConcreteClass;
import test.Ice.classLoader.Test.E;
import test.Ice.classLoader.Test.InitialPrx;

public class AllTests
{
    private static class MyClassLoader extends ClassLoader
    {
        MyClassLoader(ClassLoader parent)
        {
            super(parent);
        }

        @Override
        protected Class<?> loadClass(String name, boolean resolve)
            throws ClassNotFoundException
        {
            _names.add(name);
            return super.loadClass(name, resolve);
        }

        void reset()
        {
            _names.clear();
        }

        boolean check(String name)
        {
            return _names.contains(name);
        }

        private java.util.List<String> _names = new java.util.LinkedList<>();
    }

    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void allTests(test.TestHelper helper, boolean collocated)
    {
        com.zeroc.Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        //
        // Verify that the class loader is used for Slice packages.
        //
        {
            out.print("testing package... ");
            out.flush();
            com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
            initData.properties = communicator.getProperties()._clone();
            MyClassLoader classLoader = new MyClassLoader(helper.getClassLoader());
            initData.classLoader = classLoader;
            try(com.zeroc.Ice.Communicator ic = helper.initialize(initData))
            {
                test(classLoader.check("test.Ice.classLoader.Test._Marker"));
                out.println("ok");
            }
        }

        //
        // Verify that the class loader is used for Ice plug-ins.
        //
        {
            out.print("testing plug-in... ");
            out.flush();
            com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("Ice.Plugin.Test", "test.Ice.classLoader.PluginFactoryI");
            MyClassLoader classLoader = new MyClassLoader(helper.getClassLoader());
            initData.classLoader = classLoader;
            try(com.zeroc.Ice.Communicator ic = helper.initialize(initData))
            {
                test(classLoader.check("test.Ice.classLoader.PluginFactoryI"));
                out.println("ok");
            }
        }

        //
        // Verify that the class loader is used for IceSSL certificate verifiers and password callbacks.
        //
        if(communicator.getProperties().getProperty("Ice.Default.Protocol").equals("ssl"))
        {
            out.print("testing IceSSL certificate verifier and password callback... ");
            out.flush();
            com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty("IceSSL.CertVerifier", "test.Ice.classLoader.CertificateVerifierI");
            initData.properties.setProperty("IceSSL.PasswordCallback", "test.Ice.classLoader.PasswordCallbackI");
            MyClassLoader classLoader = new MyClassLoader(helper.getClassLoader());
            initData.classLoader = classLoader;
            try(com.zeroc.Ice.Communicator ic = helper.initialize(initData))
            {
                test(classLoader.check("test.Ice.classLoader.CertificateVerifierI"));
                test(classLoader.check("test.Ice.classLoader.PasswordCallbackI"));
                out.println("ok");
            }
        }

        //
        // Marshaling tests.
        //
        {
            com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
            initData.properties = communicator.getProperties()._clone();
            MyClassLoader classLoader = new MyClassLoader(helper.getClassLoader());
            initData.classLoader = classLoader;
            try(com.zeroc.Ice.Communicator ic = helper.initialize(initData))
            {
                String ref = "initial:" + helper.getTestEndpoint(0);
                com.zeroc.Ice.ObjectPrx base = ic.stringToProxy(ref);
                test(base != null);

                InitialPrx initial = InitialPrx.checkedCast(base);
                test(initial != null);

                //
                // Verify that the class loader is used for concrete classes.
                //
                {
                    out.print("testing concrete class... ");
                    out.flush();
                    ConcreteClass cc = initial.getConcreteClass();
                    test(cc != null);
                    test(classLoader.check("Test.ConcreteClass"));
                    test(classLoader.check("test.Ice.classLoader.Test.ConcreteClass"));
                    classLoader.reset();
                    out.println("ok");
                }

                //
                // Verify that the class loader is used for user exceptions.
                //
                out.print("testing user exception... ");
                out.flush();
                try
                {
                    initial.throwException();
                    test(false);
                }
                catch(E ex)
                {
                }
                test(classLoader.check("Test.E"));
                test(classLoader.check("test.Ice.classLoader.Test.E"));
                out.println("ok");

                initial.shutdown();
                ic.destroy();
            }
        }
    }
}
