// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.classLoader;

import java.io.PrintWriter;

import test.Ice.classLoader.Test.AbstractClass;
import test.Ice.classLoader.Test.ConcreteClass;
import test.Ice.classLoader.Test.E;
import test.Ice.classLoader.Test.InitialPrx;
import test.Ice.classLoader.Test.InitialPrxHelper;

public class AllTests
{
    private static class MyObjectFactory implements Ice.ObjectFactory
    {
        public Ice.Object create(String type)
        {
            if(type.equals("::Test::AbstractClass"))
            {
                return new AbstractClassI();
            }

            assert (false); // Should never be reached
            return null;
        }

        public void destroy()
        {
            // Nothing to do
        }
    }

    private static class MyClassLoader extends ClassLoader
    {
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

        private java.util.List<String> _names = new java.util.LinkedList<String>();
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void
    allTests(boolean collocated, PrintWriter out, Ice.InitializationData initData)
    {
        //
        // Verify that the class loader is used for Slice packages.
        //
        {
            out.print("testing package... ");
            out.flush();
            Ice.InitializationData init = (Ice.InitializationData)initData.clone();
            MyClassLoader classLoader = new MyClassLoader();
            init.classLoader = classLoader;
            Ice.Communicator communicator = Ice.Util.initialize(init);
            test(classLoader.check("test.Ice.classLoader.Test._Marker"));
            communicator.destroy();
            out.println("ok");
        }

        //
        // Verify that the class loader is used for Ice plug-ins.
        //
        {
            out.print("testing plug-in... ");
            out.flush();
            Ice.InitializationData init = (Ice.InitializationData)initData.clone();
            init.properties = (Ice.Properties)initData.properties._clone();
            init.properties.setProperty("Ice.Plugin.Test", "test.Ice.classLoader.PluginFactoryI");
            MyClassLoader classLoader = new MyClassLoader();
            init.classLoader = classLoader;
            Ice.Communicator communicator = Ice.Util.initialize(init);
            test(classLoader.check("test.Ice.classLoader.PluginFactoryI"));
            communicator.destroy();
            out.println("ok");
        }

        //
        // Verify that the class loader is used for IceSSL certificate verifiers and password callbacks.
        //
        if(initData.properties.getProperty("Ice.Default.Protocol").equals("ssl"))
        {
            out.print("testing IceSSL certificate verifier and password callback... ");
            out.flush();
            Ice.InitializationData init = (Ice.InitializationData)initData.clone();
            init.properties = (Ice.Properties)initData.properties._clone();
            init.properties.setProperty("IceSSL.CertVerifier", "test.Ice.classLoader.CertificateVerifierI");
            init.properties.setProperty("IceSSL.PasswordCallback", "test.Ice.classLoader.PasswordCallbackI");
            MyClassLoader classLoader = new MyClassLoader();
            init.classLoader = classLoader;
            Ice.Communicator communicator = Ice.Util.initialize(init);
            test(classLoader.check("test.Ice.classLoader.CertificateVerifierI"));
            test(classLoader.check("test.Ice.classLoader.PasswordCallbackI"));
            communicator.destroy();
            out.println("ok");
        }

        //
        // Marshaling tests.
        //
        {
            Ice.InitializationData init = (Ice.InitializationData)initData.clone();
            MyClassLoader classLoader = new MyClassLoader();
            init.classLoader = classLoader;
            Ice.Communicator communicator = Ice.Util.initialize(init);

            String ref = "initial:default -p 12010";
            Ice.ObjectPrx base = communicator.stringToProxy(ref);
            test(base != null);

            InitialPrx initial = InitialPrxHelper.checkedCast(base);
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
            // Verify that the class loader is invoked when a factory is not installed, and is
            // not invoked when a factory is installed.
            //
            {
                out.print("testing abstract class... ");
                out.flush();

                try
                {
                    initial.getAbstractClass();
                }
                catch(Ice.NoObjectFactoryException ex)
                {
                    // Expected.
                }
                test(classLoader.check("Test.AbstractClass"));
                test(classLoader.check("test.Ice.classLoader.Test.AbstractClass"));
                classLoader.reset();

                communicator.addObjectFactory(new MyObjectFactory(), "::Test::AbstractClass");
                AbstractClass ac = initial.getAbstractClass();
                test(ac != null);
                test(!classLoader.check("Test.AbstractClass"));
                test(!classLoader.check("test.Ice.classLoader.Test.AbstractClass"));
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
            communicator.destroy();
        }
    }
}
