// Copyright (c) ZeroC, Inc.

package test.Ice.scope;

public class Server extends test.TestHelper {
    class MyInterface1 implements test.Ice.scope.Test.MyInterface {
        public test.Ice.scope.Test.MyInterface.OpMyStructResult opMyStruct(
                test.Ice.scope.Test.MyStruct s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.MyInterface.OpMyStructResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.MyInterface.OpMyStructSeqResult opMyStructSeq(
                test.Ice.scope.Test.MyStruct[] s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.MyInterface.OpMyStructSeqResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.MyInterface.OpMyStructMapResult opMyStructMap(
                java.util.Map<String, test.Ice.scope.Test.MyStruct> s1,
                com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.MyInterface.OpMyStructMapResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.MyInterface.OpMyClassResult opMyClass(
                test.Ice.scope.Test.MyClass c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.MyInterface.OpMyClassResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.MyInterface.OpMyClassSeqResult opMyClassSeq(
                test.Ice.scope.Test.MyClass[] c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.MyInterface.OpMyClassSeqResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.MyInterface.OpMyClassMapResult opMyClassMap(
                java.util.Map<String, test.Ice.scope.Test.MyClass> c1,
                com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.MyInterface.OpMyClassMapResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.MyEnum opMyEnum(
                test.Ice.scope.Test.MyEnum e1, com.zeroc.Ice.Current current) {
            return e1;
        }

        public test.Ice.scope.Test.MyOtherStruct opMyOtherStruct(
                test.Ice.scope.Test.MyOtherStruct s1, com.zeroc.Ice.Current current) {
            return s1;
        }

        public test.Ice.scope.Test.MyOtherClass opMyOtherClass(
                test.Ice.scope.Test.MyOtherClass e1, com.zeroc.Ice.Current current) {
            return e1;
        }

        public void shutdown(com.zeroc.Ice.Current current) {
            current.adapter.getCommunicator().shutdown();
        }
    }

    class MyInterface2 implements test.Ice.scope.Test.Inner.MyInterface {
        public test.Ice.scope.Test.Inner.MyInterface.OpMyStructResult opMyStruct(
                test.Ice.scope.Test.Inner.Inner2.MyStruct s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.MyInterface.OpMyStructResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.MyInterface.OpMyStructSeqResult opMyStructSeq(
                test.Ice.scope.Test.Inner.Inner2.MyStruct[] s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.MyInterface.OpMyStructSeqResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.MyInterface.OpMyStructMapResult opMyStructMap(
                java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.MyStruct> s1,
                com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.MyInterface.OpMyStructMapResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.MyInterface.OpMyClassResult opMyClass(
                test.Ice.scope.Test.Inner.Inner2.MyClass c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.MyInterface.OpMyClassResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.Inner.MyInterface.OpMyClassSeqResult opMyClassSeq(
                test.Ice.scope.Test.Inner.Inner2.MyClass[] c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.MyInterface.OpMyClassSeqResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.Inner.MyInterface.OpMyClassMapResult opMyClassMap(
                java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.MyClass> c1,
                com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.MyInterface.OpMyClassMapResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public void shutdown(com.zeroc.Ice.Current current) {
            current.adapter.getCommunicator().shutdown();
        }
    }

    class MyInterface3 implements test.Ice.scope.Test.Inner.Inner2.MyInterface {
        public test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyStructResult opMyStruct(
                test.Ice.scope.Test.Inner.Inner2.MyStruct s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyStructResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyStructSeqResult opMyStructSeq(
                test.Ice.scope.Test.Inner.Inner2.MyStruct[] s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyStructSeqResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyStructMapResult opMyStructMap(
                java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.MyStruct> s1,
                com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyStructMapResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyClassResult opMyClass(
                test.Ice.scope.Test.Inner.Inner2.MyClass c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyClassResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyClassSeqResult opMyClassSeq(
                test.Ice.scope.Test.Inner.Inner2.MyClass[] c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyClassSeqResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyClassMapResult opMyClassMap(
                java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.MyClass> c1,
                com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.Inner2.MyInterface.OpMyClassMapResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public void shutdown(com.zeroc.Ice.Current current) {
            current.adapter.getCommunicator().shutdown();
        }
    }

    class MyInterface4 implements test.Ice.scope.Inner.Test.Inner2.MyInterface {
        public test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyStructResult opMyStruct(
                test.Ice.scope.Test.MyStruct s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyStructResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyStructSeqResult opMyStructSeq(
                test.Ice.scope.Test.MyStruct[] s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyStructSeqResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyStructMapResult opMyStructMap(
                java.util.Map<String, test.Ice.scope.Test.MyStruct> s1,
                com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyStructMapResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyClassResult opMyClass(
                test.Ice.scope.Test.MyClass c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyClassResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyClassSeqResult opMyClassSeq(
                test.Ice.scope.Test.MyClass[] c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyClassSeqResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyClassMapResult opMyClassMap(
                java.util.Map<String, test.Ice.scope.Test.MyClass> c1,
                com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Inner.Test.Inner2.MyInterface.OpMyClassMapResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public void shutdown(com.zeroc.Ice.Current current) {
            current.adapter.getCommunicator().shutdown();
        }
    }

    public void run(String[] args) {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.scope");
        try (com.zeroc.Ice.Communicator communicator = initialize(properties)) {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new MyInterface1(), com.zeroc.Ice.Util.stringToIdentity("i1"));
            adapter.add(new MyInterface2(), com.zeroc.Ice.Util.stringToIdentity("i2"));
            adapter.add(new MyInterface3(), com.zeroc.Ice.Util.stringToIdentity("i3"));
            adapter.add(new MyInterface4(), com.zeroc.Ice.Util.stringToIdentity("i4"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
