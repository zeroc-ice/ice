// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.scope;

public class Server extends test.TestHelper
{
    class I1 extends test.Ice.scope.Test._IDisp
    {
        public test.Ice.scope.Test.S
        opS(test.Ice.scope.Test.S s1, test.Ice.scope.Test.SHolder s2, Ice.Current current)
        {
            s2.value = s1;
            return s1;
        }

        public test.Ice.scope.Test.S[]
        opSSeq(test.Ice.scope.Test.S[] s1, test.Ice.scope.Test.SSeqHolder s2, Ice.Current current)
        {
            s2.value = s1;
            return s1;
        }

        public java.util.Map<String, test.Ice.scope.Test.S>
        opSMap(java.util.Map<String, test.Ice.scope.Test.S> s1,
               test.Ice.scope.Test.SMapHolder s2, Ice.Current current)
        {
            s2.value = s1;
            return s1;
        }

        public test.Ice.scope.Test.C
        opC(test.Ice.scope.Test.C c1, test.Ice.scope.Test.CHolder c2, Ice.Current current)
        {
            c2.value = c1;
            return c1;
        }

        public test.Ice.scope.Test.C[]
        opCSeq(test.Ice.scope.Test.C[] c1,
               test.Ice.scope.Test.CSeqHolder c2,
               Ice.Current current)
        {
            c2.value = c1;
            return c1;
        }

        public java.util.Map<String, test.Ice.scope.Test.C>
        opCMap(java.util.Map<String, test.Ice.scope.Test.C> c1,
               test.Ice.scope.Test.CMapHolder c2,
               Ice.Current current)
        {
            c2.value = c1;
            return c1;
        }

        public void shutdown(Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }

    class I2 extends test.Ice.scope.Test.Inner._IDisp
    {
        public test.Ice.scope.Test.Inner.Inner2.S
        opS(test.Ice.scope.Test.Inner.Inner2.S s1,
            test.Ice.scope.Test.Inner.Inner2.SHolder s2,
            Ice.Current current)
        {
            s2.value = s1;
            return s1;
        }

        public test.Ice.scope.Test.Inner.Inner2.S[]
        opSSeq(test.Ice.scope.Test.Inner.Inner2.S[] s1,
               test.Ice.scope.Test.Inner.Inner2.SSeqHolder s2,
               Ice.Current current)
        {
            s2.value = s1;
            return s1;
        }

        public java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S>
        opSMap(java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> s1,
               test.Ice.scope.Test.Inner.Inner2.SMapHolder s2,
               Ice.Current current)
        {
            s2.value = s1;
            return s1;
        }

        public test.Ice.scope.Test.Inner.Inner2.C
        opC(test.Ice.scope.Test.Inner.Inner2.C c1,
            test.Ice.scope.Test.Inner.Inner2.CHolder c2,
            Ice.Current current)
        {
            c2.value = c1;
            return c1;
        }

        public test.Ice.scope.Test.Inner.Inner2.C[]
        opCSeq(test.Ice.scope.Test.Inner.Inner2.C[] c1,
               test.Ice.scope.Test.Inner.Inner2.CSeqHolder c2,
               Ice.Current current)
        {
            c2.value = c1;
            return c1;
        }

        public java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C>
        opCMap(java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> c1,
               test.Ice.scope.Test.Inner.Inner2.CMapHolder c2,
               Ice.Current current)
        {
            c2.value = c1;
            return c1;
        }

        public void shutdown(Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }

    class I3 extends test.Ice.scope.Test.Inner.Inner2._IDisp
    {
        public test.Ice.scope.Test.Inner.Inner2.S
        opS(test.Ice.scope.Test.Inner.Inner2.S s1,
            test.Ice.scope.Test.Inner.Inner2.SHolder s2,
            Ice.Current current)
        {
            s2.value = s1;
            return s1;
        }

        public test.Ice.scope.Test.Inner.Inner2.S[]
        opSSeq(test.Ice.scope.Test.Inner.Inner2.S[] s1,
               test.Ice.scope.Test.Inner.Inner2.SSeqHolder s2,
               Ice.Current current)
        {
            s2.value = s1;
            return s1;
        }

        public java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S>
        opSMap(java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.S> s1,
               test.Ice.scope.Test.Inner.Inner2.SMapHolder s2,
               Ice.Current current)
        {
            s2.value = s1;
            return s1;
        }

        public test.Ice.scope.Test.Inner.Inner2.C
        opC(test.Ice.scope.Test.Inner.Inner2.C c1,
            test.Ice.scope.Test.Inner.Inner2.CHolder c2,
            Ice.Current current)
        {
            c2.value = c1;
            return c1;
        }

        public test.Ice.scope.Test.Inner.Inner2.C[]
        opCSeq(test.Ice.scope.Test.Inner.Inner2.C[] c1,
               test.Ice.scope.Test.Inner.Inner2.CSeqHolder c2,
               Ice.Current current)
        {
            c2.value = c1;
            return c1;
        }

        public java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C>
        opCMap(java.util.Map<String, test.Ice.scope.Test.Inner.Inner2.C> c1,
               test.Ice.scope.Test.Inner.Inner2.CMapHolder c2,
               Ice.Current current)
        {
            c2.value = c1;
            return c1;
        }

        public void shutdown(Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }

    class I4 extends test.Ice.scope.Inner.Test.Inner2._IDisp
    {
        public test.Ice.scope.Test.S
        opS(test.Ice.scope.Test.S s1, test.Ice.scope.Test.SHolder s2, Ice.Current current)
        {
            s2.value = s1;
            return s1;
        }

        public test.Ice.scope.Test.S[]
        opSSeq(test.Ice.scope.Test.S[] s1, test.Ice.scope.Test.SSeqHolder s2, Ice.Current current)
        {
            s2.value = s1;
            return s1;
        }

        public java.util.Map<String, test.Ice.scope.Test.S>
        opSMap(java.util.Map<String, test.Ice.scope.Test.S> s1, test.Ice.scope.Test.SMapHolder s2, Ice.Current current)
        {
            s2.value = s1;
            return s1;
        }

        public test.Ice.scope.Test.C
        opC(test.Ice.scope.Test.C c1, test.Ice.scope.Test.CHolder c2, Ice.Current current)
        {
            c2.value = c1;
            return c1;
        }

        public test.Ice.scope.Test.C[]
        opCSeq(test.Ice.scope.Test.C[] c1, test.Ice.scope.Test.CSeqHolder c2, Ice.Current current)
        {
            c2.value = c1;
            return c1;
        }

        public java.util.Map<String, test.Ice.scope.Test.C>
        opCMap(java.util.Map<String, test.Ice.scope.Test.C> c1, test.Ice.scope.Test.CMapHolder c2, Ice.Current current)
        {
            c2.value = c1;
            return c1;
        }

        public void shutdown(Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }

    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.scope");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new I1(), Ice.Util.stringToIdentity("i1"));
            adapter.add(new I2(), Ice.Util.stringToIdentity("i2"));
            adapter.add(new I3(), Ice.Util.stringToIdentity("i3"));
            adapter.add(new I4(), Ice.Util.stringToIdentity("i4"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
