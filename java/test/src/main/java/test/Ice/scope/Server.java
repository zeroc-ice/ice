// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.scope;

public class Server extends test.TestHelper
{
    class I1 implements test.Ice.scope.Test.I
    {
        public test.Ice.scope.Test.I.OpSResult
        opS(test.Ice.scope.Test.S s1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.I.OpSResult result = new test.Ice.scope.Test.I.OpSResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.I.OpSSeqResult
        opSSeq(test.Ice.scope.Test.S[] s1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.I.OpSSeqResult result = new test.Ice.scope.Test.I.OpSSeqResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.I.OpSMapResult
        opSMap(java.util.Map<String, test.Ice.scope.Test.S> s1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.I.OpSMapResult result = new test.Ice.scope.Test.I.OpSMapResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.I.OpCResult
        opC(test.Ice.scope.Test.C c1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.I.OpCResult result = new test.Ice.scope.Test.I.OpCResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.I.OpCSeqResult
        opCSeq(test.Ice.scope.Test.C[] c1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.I.OpCSeqResult result = new test.Ice.scope.Test.I.OpCSeqResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.I.OpCMapResult
        opCMap(java.util.Map<String, test.Ice.scope.Test.C> c1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.I.OpCMapResult result = new test.Ice.scope.Test.I.OpCMapResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public void shutdown(com.zeroc.Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }

    class I2 implements test.Ice.scope.Test.Inner.I
    {
        public test.Ice.scope.Test.Inner.I.OpSResult
        opS(test.Ice.scope.Test.Inner.Inner2.S s1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.Inner.I.OpSResult result = new test.Ice.scope.Test.Inner.I.OpSResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.I.OpSSeqResult
        opSSeq(test.Ice.scope.Test.Inner.Inner2.S[] s1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.Inner.I.OpSSeqResult result = new test.Ice.scope.Test.Inner.I.OpSSeqResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.I.OpSMapResult
        opSMap(java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> s1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.Inner.I.OpSMapResult result = new test.Ice.scope.Test.Inner.I.OpSMapResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.I.OpCResult
        opC(test.Ice.scope.Test.Inner.Inner2.C c1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.Inner.I.OpCResult result = new test.Ice.scope.Test.Inner.I.OpCResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.Inner.I.OpCSeqResult
        opCSeq(test.Ice.scope.Test.Inner.Inner2.C[] c1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.Inner.I.OpCSeqResult result = new test.Ice.scope.Test.Inner.I.OpCSeqResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.Inner.I.OpCMapResult
        opCMap(java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> c1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.Inner.I.OpCMapResult result = new test.Ice.scope.Test.Inner.I.OpCMapResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public void shutdown(com.zeroc.Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }

    class I3 implements test.Ice.scope.Test.Inner.Inner2.I
    {
        public test.Ice.scope.Test.Inner.Inner2.I.OpSResult
        opS(test.Ice.scope.Test.Inner.Inner2.S s1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.Inner.Inner2.I.OpSResult result = new test.Ice.scope.Test.Inner.Inner2.I.OpSResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.I.OpSSeqResult
        opSSeq(test.Ice.scope.Test.Inner.Inner2.S[] s1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.Inner.Inner2.I.OpSSeqResult result =
                new test.Ice.scope.Test.Inner.Inner2.I.OpSSeqResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.I.OpSMapResult
        opSMap(java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> s1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.Inner.Inner2.I.OpSMapResult result =
                new test.Ice.scope.Test.Inner.Inner2.I.OpSMapResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.I.OpCResult
        opC(test.Ice.scope.Test.Inner.Inner2.C c1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.Inner.Inner2.I.OpCResult result = new test.Ice.scope.Test.Inner.Inner2.I.OpCResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.I.OpCSeqResult
        opCSeq(test.Ice.scope.Test.Inner.Inner2.C[] c1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.Inner.Inner2.I.OpCSeqResult result =
                new test.Ice.scope.Test.Inner.Inner2.I.OpCSeqResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Test.Inner.Inner2.I.OpCMapResult
        opCMap(java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> c1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Test.Inner.Inner2.I.OpCMapResult result
                = new test.Ice.scope.Test.Inner.Inner2.I.OpCMapResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public void shutdown(com.zeroc.Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }

    class I4 implements test.Ice.scope.Inner.Test.Inner2.I
    {
        public test.Ice.scope.Inner.Test.Inner2.I.OpSResult
        opS(test.Ice.scope.Test.S s1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Inner.Test.Inner2.I.OpSResult result = new test.Ice.scope.Inner.Test.Inner2.I.OpSResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.I.OpSSeqResult
        opSSeq(test.Ice.scope.Test.S[] s1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Inner.Test.Inner2.I.OpSSeqResult result =
                new test.Ice.scope.Inner.Test.Inner2.I.OpSSeqResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.I.OpSMapResult
        opSMap(java.util.Map<String, test.Ice.scope.Test.S> s1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Inner.Test.Inner2.I.OpSMapResult result =
                new test.Ice.scope.Inner.Test.Inner2.I.OpSMapResult();
            result.returnValue = s1;
            result.s2 = s1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.I.OpCResult
        opC(test.Ice.scope.Test.C c1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Inner.Test.Inner2.I.OpCResult result = new test.Ice.scope.Inner.Test.Inner2.I.OpCResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.I.OpCSeqResult
        opCSeq(test.Ice.scope.Test.C[] c1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Inner.Test.Inner2.I.OpCSeqResult result =
                new test.Ice.scope.Inner.Test.Inner2.I.OpCSeqResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public test.Ice.scope.Inner.Test.Inner2.I.OpCMapResult
        opCMap(java.util.Map<String, test.Ice.scope.Test.C> c1, com.zeroc.Ice.Current current)
        {
            test.Ice.scope.Inner.Test.Inner2.I.OpCMapResult result =
                new test.Ice.scope.Inner.Test.Inner2.I.OpCMapResult();
            result.returnValue = c1;
            result.c2 = c1;
            return result;
        }

        public void shutdown(com.zeroc.Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }

    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.scope");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
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
