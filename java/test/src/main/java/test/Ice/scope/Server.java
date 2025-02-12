// Copyright (c) ZeroC, Inc.

package test.Ice.scope;

public class Server extends test.TestHelper {
    class I1 implements test.Ice.scope.Test.I {
        public test.Ice.scope.Test.I.OpMyStructResult opMyStruct(
                test.Ice.scope.Test.MyStruct s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.I.OpMyStructResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.I.OpMyStructSeqResult opMyStructSeq(
                test.Ice.scope.Test.MyStruct[] s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.I.OpMyStructSeqResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.I.OpMyStructMapResult opMyStructMap(
                java.util.Map<String, test.Ice.scope.Test.MyStruct> s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.I.OpMyStructMapResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.I.OpMyClassResult opMyClass(
                test.Ice.scope.Test.MyClass c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.I.OpMyClassResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.I.OpMyClassSeqResult opMyClassSeq(
                test.Ice.scope.Test.MyClass[] c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.I.OpMyClassSeqResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.I.OpMyClassMapResult opMyClassMap(
                java.util.Map<String, test.Ice.scope.Test.MyClass> c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.I.OpMyClassMapResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.E1 opE1(
                test.Ice.scope.Test.E1 e1, com.zeroc.Ice.Current current) {
            return e1;
        }

        public test.Ice.scope.Test.S1 opS1(
                test.Ice.scope.Test.S1 s1, com.zeroc.Ice.Current current) {
            return s1;
        }

        public test.Ice.scope.Test.C1 opC1(
                test.Ice.scope.Test.C1 e1, com.zeroc.Ice.Current current) {
            return e1;
        }

        public void shutdown(com.zeroc.Ice.Current current) {
            current.adapter.getCommunicator().shutdown();
        }
    }

    class I2 implements test.Ice.scope.Test.Inner.I {
        public test.Ice.scope.Test.Inner.I.OpMyStructResult opMyStruct(
                test.Ice.scope.Test.Inner.Inner2.MyStruct s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.I.OpMyStructResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.I.OpMyStructSeqResult opMyStructSeq(
                test.Ice.scope.Test.Inner.Inner2.MyStruct[] s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.I.OpMyStructSeqResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.I.OpMyStructMapResult opMyStructMap(
                java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.MyStruct> s1,
                com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.I.OpMyStructMapResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.I.OpMyClassResult opMyClass(
                test.Ice.scope.Test.Inner.Inner2.MyClass c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.I.OpMyClassResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.Inner.I.OpMyClassSeqResult opMyClassSeq(
                test.Ice.scope.Test.Inner.Inner2.MyClass[] c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.I.OpMyClassSeqResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.Inner.I.OpMyClassMapResult opMyClassMap(
                java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.MyClass> c1,
                com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.I.OpMyClassMapResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public void shutdown(com.zeroc.Ice.Current current) {
            current.adapter.getCommunicator().shutdown();
        }
    }

    class I3 implements test.Ice.scope.Test.Inner.Inner2.I {
        public test.Ice.scope.Test.Inner.Inner2.I.OpMyStructResult opMyStruct(
                test.Ice.scope.Test.Inner.Inner2.MyStruct s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.Inner2.I.OpMyStructResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.I.OpMyStructSeqResult opMyStructSeq(
                test.Ice.scope.Test.Inner.Inner2.MyStruct[] s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.Inner2.I.OpMyStructSeqResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.I.OpMyStructMapResult opMyStructMap(
                java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.MyStruct> s1,
                com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.Inner2.I.OpMyStructMapResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.I.OpMyClassResult opMyClass(
                test.Ice.scope.Test.Inner.Inner2.MyClass c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.Inner2.I.OpMyClassResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.I.OpMyClassSeqResult opMyClassSeq(
                test.Ice.scope.Test.Inner.Inner2.MyClass[] c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.Inner2.I.OpMyClassSeqResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.I.OpMyClassMapResult opMyClassMap(
                java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.MyClass> c1,
                com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Test.Inner.Inner2.I.OpMyClassMapResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public void shutdown(com.zeroc.Ice.Current current) {
            current.adapter.getCommunicator().shutdown();
        }
    }

    class I4 implements test.Ice.scope.Inner.Test.Inner2.I {
        public test.Ice.scope.Inner.Test.Inner2.I.OpMyStructResult opMyStruct(
                test.Ice.scope.Test.MyStruct s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Inner.Test.Inner2.I.OpMyStructResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.I.OpMyStructSeqResult opMyStructSeq(
                test.Ice.scope.Test.MyStruct[] s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Inner.Test.Inner2.I.OpMyStructSeqResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.I.OpMyStructMapResult opMyStructMap(
                java.util.Map<String, test.Ice.scope.Test.MyStruct> s1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Inner.Test.Inner2.I.OpMyStructMapResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.I.OpMyClassResult opMyClass(
                test.Ice.scope.Test.MyClass c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Inner.Test.Inner2.I.OpMyClassResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.I.OpMyClassSeqResult opMyClassSeq(
                test.Ice.scope.Test.MyClass[] c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Inner.Test.Inner2.I.OpMyClassSeqResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.I.OpMyClassMapResult opMyClassMap(
                java.util.Map<String, test.Ice.scope.Test.MyClass> c1, com.zeroc.Ice.Current current) {
            var result = new test.Ice.scope.Inner.Test.Inner2.I.OpMyClassMapResult();
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
            adapter.add(new I1(), com.zeroc.Ice.Util.stringToIdentity("i1"));
            adapter.add(new I2(), com.zeroc.Ice.Util.stringToIdentity("i2"));
            adapter.add(new I3(), com.zeroc.Ice.Util.stringToIdentity("i3"));
            adapter.add(new I4(), com.zeroc.Ice.Util.stringToIdentity("i4"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
