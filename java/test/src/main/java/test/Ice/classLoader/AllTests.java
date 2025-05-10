// Copyright (c) ZeroC, Inc.

package test.Ice.classLoader;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.ObjectPrx;

import test.Ice.classLoader.Test.ConcreteClass;
import test.Ice.classLoader.Test.E;
import test.Ice.classLoader.Test.InitialPrx;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

public class AllTests {
    private static class MyClassLoader extends ClassLoader {
        MyClassLoader(ClassLoader parent) {
            super(parent);
        }

        @Override
        protected Class<?> loadClass(String name, boolean resolve) throws ClassNotFoundException {
            _names.add(name);
            return super.loadClass(name, resolve);
        }

        void reset() {
            _names.clear();
        }

        boolean check(String name) {
            return _names.contains(name);
        }

        private List<String> _names = new LinkedList<>();
    }

    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static void allTests(TestHelper helper, boolean collocated) {
        var moduleToPackage = Collections.singletonMap("::Test", "test.Ice.classLoader.Test");
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        //
        // Verify that the class loader is used for Ice plug-ins.
        //
        {
            out.print("testing plug-in... ");
            out.flush();
            InitializationData initData = new InitializationData();
            initData.properties = communicator.getProperties()._clone();
            initData.properties.setProperty(
                "Ice.Plugin.Test", "test.Ice.classLoader.PluginFactoryI");
            MyClassLoader classLoader = new MyClassLoader(helper.getClassLoader());
            initData.classLoader = classLoader;
            initData.sliceLoader = new ModuleToPackageSliceLoader(moduleToPackage, classLoader);
            try (Communicator ic = helper.initialize(initData)) {
                test(classLoader.check("test.Ice.classLoader.PluginFactoryI"));
                out.println("ok");
            }
        }

        //
        // Marshaling tests.
        //
        {
            InitializationData initData = new InitializationData();
            initData.properties = communicator.getProperties()._clone();
            MyClassLoader classLoader = new MyClassLoader(helper.getClassLoader());
            initData.classLoader = classLoader;
            initData.sliceLoader = new ModuleToPackageSliceLoader(moduleToPackage, classLoader);
            try (Communicator ic = helper.initialize(initData)) {
                String ref = "initial:" + helper.getTestEndpoint(0);
                ObjectPrx base = ic.stringToProxy(ref);
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
                    test(classLoader.check("test.Ice.classLoader.Test.ConcreteClass"));
                    classLoader.reset();
                    out.println("ok");
                }

                //
                // Verify that the class loader is used for user exceptions.
                //
                out.print("testing user exception... ");
                out.flush();
                try {
                    initial.throwException();
                    test(false);
                } catch (E ex) {}
                test(classLoader.check("test.Ice.classLoader.Test.E"));
                out.println("ok");

                initial.shutdown();
                ic.destroy();
            }
        }
    }

    private AllTests() {}
}
