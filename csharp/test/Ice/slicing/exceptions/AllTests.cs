//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using Test;
using Ice;
public class AllTests
{
    private class Relay : IRelay
    {
        public void knownPreservedAsBase(Ice.Current current) =>
            throw new KnownPreservedDerived("base", "preserved", "derived");

        public void knownPreservedAsKnownPreserved(Ice.Current current) =>
            throw new KnownPreservedDerived("base", "preserved", "derived");

        public void unknownPreservedAsBase(Ice.Current current)
        {
            var p = new PreservedClass("bc", "pc");
            throw new Preserved2("base", "preserved", "derived", p, p);
        }

        public void unknownPreservedAsKnownPreserved(Ice.Current current)
        {
            var p = new PreservedClass("bc", "pc");
            throw new Preserved2("base", "preserved", "derived", p, p);
        }

        public void clientPrivateException(Ice.Current current) => throw new ClientPrivateException("ClientPrivate");
    }

    public static ITestIntfPrx allTests(TestHelper helper)
    {
        Communicator? communicator = helper.Communicator();
        TestHelper.Assert(communicator != null);
        var output = helper.GetWriter();
        output.Write("testing stringToProxy... ");
        output.Flush();
        ITestIntfPrx testPrx = ITestIntfPrx.Parse($"Test:{helper.GetTestEndpoint(0)} -t 2000", communicator);
        output.WriteLine("ok");

        output.Write("base... ");
        output.Flush();
        {
            try
            {
                testPrx.baseAsBase();
                TestHelper.Assert(false);
            }
            catch (Base b)
            {
                TestHelper.Assert(b.b.Equals("Base.b"));
                TestHelper.Assert(b.GetType().FullName!.Equals("Test.Base"));
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("base (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.baseAsBaseAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    throw ae.InnerException;
                }
                catch (Base b)
                {
                    TestHelper.Assert(b.b.Equals("Base.b"));
                    TestHelper.Assert(b.GetType().Name.Equals("Base"));
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown derived... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownDerivedAsBase();
                TestHelper.Assert(false);
            }
            catch (Base b)
            {
                TestHelper.Assert(b.b.Equals("UnknownDerived.b"));
                TestHelper.Assert(b.GetType().FullName!.Equals("Test.Base"));
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown derived (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownDerivedAsBaseAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    throw ae.InnerException;
                }
                catch (Base b)
                {
                    TestHelper.Assert(b.b.Equals("UnknownDerived.b"));
                    TestHelper.Assert(b.GetType().Name.Equals("Base"));
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known derived as base... ");
        output.Flush();
        {
            try
            {
                testPrx.knownDerivedAsBase();
                TestHelper.Assert(false);
            }
            catch (KnownDerived k)
            {
                TestHelper.Assert(k.b.Equals("KnownDerived.b"));
                TestHelper.Assert(k.kd.Equals("KnownDerived.kd"));
                TestHelper.Assert(k.GetType().FullName!.Equals("Test.KnownDerived"));
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known derived as base (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.knownDerivedAsBaseAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    throw ae.InnerException;
                }
                catch (KnownDerived k)
                {
                    TestHelper.Assert(k.b.Equals("KnownDerived.b"));
                    TestHelper.Assert(k.kd.Equals("KnownDerived.kd"));
                    TestHelper.Assert(k.GetType().Name.Equals("KnownDerived"));
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known derived as derived... ");
        output.Flush();
        {
            try
            {
                testPrx.knownDerivedAsKnownDerived();
                TestHelper.Assert(false);
            }
            catch (KnownDerived k)
            {
                TestHelper.Assert(k.b.Equals("KnownDerived.b"));
                TestHelper.Assert(k.kd.Equals("KnownDerived.kd"));
                TestHelper.Assert(k.GetType().FullName!.Equals("Test.KnownDerived"));
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known derived as derived (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.knownDerivedAsKnownDerivedAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    throw ae.InnerException;
                }
                catch (KnownDerived k)
                {
                    TestHelper.Assert(k.b.Equals("KnownDerived.b"));
                    TestHelper.Assert(k.kd.Equals("KnownDerived.kd"));
                    TestHelper.Assert(k.GetType().Name.Equals("KnownDerived"));
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown intermediate as base... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownIntermediateAsBase();
                TestHelper.Assert(false);
            }
            catch (Base b)
            {
                TestHelper.Assert(b.b.Equals("UnknownIntermediate.b"));
                TestHelper.Assert(b.GetType().FullName!.Equals("Test.Base"));
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown intermediate as base (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownIntermediateAsBaseAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    throw ae.InnerException;
                }
                catch (Base b)
                {
                    TestHelper.Assert(b.b.Equals("UnknownIntermediate.b"));
                    TestHelper.Assert(b.GetType().Name.Equals("Base"));
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of known intermediate as base... ");
        output.Flush();
        {
            try
            {
                testPrx.knownIntermediateAsBase();
                TestHelper.Assert(false);
            }
            catch (KnownIntermediate ki)
            {
                TestHelper.Assert(ki.b.Equals("KnownIntermediate.b"));
                TestHelper.Assert(ki.ki.Equals("KnownIntermediate.ki"));
                TestHelper.Assert(ki.GetType().FullName!.Equals("Test.KnownIntermediate"));
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of known intermediate as base (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.knownIntermediateAsBaseAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    throw ae.InnerException;
                }
                catch (KnownIntermediate ki)
                {
                    TestHelper.Assert(ki.b.Equals("KnownIntermediate.b"));
                    TestHelper.Assert(ki.ki.Equals("KnownIntermediate.ki"));
                    TestHelper.Assert(ki.GetType().Name.Equals("KnownIntermediate"));
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of known most derived as base... ");
        output.Flush();
        {
            try
            {
                testPrx.knownMostDerivedAsBase();
                TestHelper.Assert(false);
            }
            catch (KnownMostDerived kmd)
            {
                TestHelper.Assert(kmd.b.Equals("KnownMostDerived.b"));
                TestHelper.Assert(kmd.ki.Equals("KnownMostDerived.ki"));
                TestHelper.Assert(kmd.kmd.Equals("KnownMostDerived.kmd"));
                TestHelper.Assert(kmd.GetType().FullName!.Equals("Test.KnownMostDerived"));
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of known most derived as base (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.knownMostDerivedAsBaseAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    throw ae.InnerException;
                }
                catch (KnownMostDerived kmd)
                {
                    TestHelper.Assert(kmd.b.Equals("KnownMostDerived.b"));
                    TestHelper.Assert(kmd.ki.Equals("KnownMostDerived.ki"));
                    TestHelper.Assert(kmd.kmd.Equals("KnownMostDerived.kmd"));
                    TestHelper.Assert(kmd.GetType().Name.Equals("KnownMostDerived"));
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known intermediate as intermediate... ");
        output.Flush();
        {
            try
            {
                testPrx.knownIntermediateAsKnownIntermediate();
                TestHelper.Assert(false);
            }
            catch (KnownIntermediate ki)
            {
                TestHelper.Assert(ki.b.Equals("KnownIntermediate.b"));
                TestHelper.Assert(ki.ki.Equals("KnownIntermediate.ki"));
                TestHelper.Assert(ki.GetType().FullName!.Equals("Test.KnownIntermediate"));
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known intermediate as intermediate (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.knownIntermediateAsKnownIntermediateAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    throw ae.InnerException;
                }
                catch (KnownIntermediate ki)
                {
                    TestHelper.Assert(ki.b.Equals("KnownIntermediate.b"));
                    TestHelper.Assert(ki.ki.Equals("KnownIntermediate.ki"));
                    TestHelper.Assert(ki.GetType().Name.Equals("KnownIntermediate"));
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known most derived as intermediate... ");
        output.Flush();
        {
            try
            {
                testPrx.knownMostDerivedAsKnownIntermediate();
                TestHelper.Assert(false);
            }
            catch (KnownMostDerived kmd)
            {
                TestHelper.Assert(kmd.b.Equals("KnownMostDerived.b"));
                TestHelper.Assert(kmd.ki.Equals("KnownMostDerived.ki"));
                TestHelper.Assert(kmd.kmd.Equals("KnownMostDerived.kmd"));
                TestHelper.Assert(kmd.GetType().FullName!.Equals("Test.KnownMostDerived"));
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known most derived as intermediate (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.knownMostDerivedAsKnownIntermediateAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    throw ae.InnerException;
                }
                catch (KnownMostDerived kmd)
                {
                    TestHelper.Assert(kmd.b.Equals("KnownMostDerived.b"));
                    TestHelper.Assert(kmd.ki.Equals("KnownMostDerived.ki"));
                    TestHelper.Assert(kmd.kmd.Equals("KnownMostDerived.kmd"));
                    TestHelper.Assert(kmd.GetType().Name.Equals("KnownMostDerived"));
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known most derived as most derived... ");
        output.Flush();
        {
            try
            {
                testPrx.knownMostDerivedAsKnownMostDerived();
                TestHelper.Assert(false);
            }
            catch (KnownMostDerived kmd)
            {
                TestHelper.Assert(kmd.b.Equals("KnownMostDerived.b"));
                TestHelper.Assert(kmd.ki.Equals("KnownMostDerived.ki"));
                TestHelper.Assert(kmd.kmd.Equals("KnownMostDerived.kmd"));
                TestHelper.Assert(kmd.GetType().FullName!.Equals("Test.KnownMostDerived"));
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known most derived as most derived (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.knownMostDerivedAsKnownMostDerivedAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    throw ae.InnerException;
                }
                catch (KnownMostDerived kmd)
                {
                    TestHelper.Assert(kmd.b.Equals("KnownMostDerived.b"));
                    TestHelper.Assert(kmd.ki.Equals("KnownMostDerived.ki"));
                    TestHelper.Assert(kmd.kmd.Equals("KnownMostDerived.kmd"));
                    TestHelper.Assert(kmd.GetType().Name.Equals("KnownMostDerived"));
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown most derived, known intermediate as base... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownMostDerived1AsBase();
                TestHelper.Assert(false);
            }
            catch (KnownIntermediate ki)
            {
                TestHelper.Assert(ki.b.Equals("UnknownMostDerived1.b"));
                TestHelper.Assert(ki.ki.Equals("UnknownMostDerived1.ki"));
                TestHelper.Assert(ki.GetType().FullName!.Equals("Test.KnownIntermediate"));
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown most derived, known intermediate as base (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownMostDerived1AsBaseAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    throw ae.InnerException;
                }
                catch (KnownIntermediate ki)
                {
                    TestHelper.Assert(ki.b.Equals("UnknownMostDerived1.b"));
                    TestHelper.Assert(ki.ki.Equals("UnknownMostDerived1.ki"));
                    TestHelper.Assert(ki.GetType().Name.Equals("KnownIntermediate"));
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown most derived, known intermediate as intermediate... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownMostDerived1AsKnownIntermediate();
                TestHelper.Assert(false);
            }
            catch (KnownIntermediate ki)
            {
                TestHelper.Assert(ki.b.Equals("UnknownMostDerived1.b"));
                TestHelper.Assert(ki.ki.Equals("UnknownMostDerived1.ki"));
                TestHelper.Assert(ki.GetType().FullName!.Equals("Test.KnownIntermediate"));
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown most derived, known intermediate as intermediate (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownMostDerived1AsKnownIntermediateAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    throw ae.InnerException;
                }
                catch (KnownIntermediate ki)
                {
                    TestHelper.Assert(ki.b.Equals("UnknownMostDerived1.b"));
                    TestHelper.Assert(ki.ki.Equals("UnknownMostDerived1.ki"));
                    TestHelper.Assert(ki.GetType().Name.Equals("KnownIntermediate"));
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown most derived, unknown intermediate thrown as base... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownMostDerived2AsBase();
                TestHelper.Assert(false);
            }
            catch (Base b)
            {
                TestHelper.Assert(b.b.Equals("UnknownMostDerived2.b"));
                TestHelper.Assert(b.GetType().FullName!.Equals("Test.Base"));
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown most derived, unknown intermediate thrown as base (AMI)... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownMostDerived2AsBaseAsync().Wait();
                TestHelper.Assert(false);
            }
            catch (AggregateException ae)
            {
                try
                {
                    TestHelper.Assert(ae.InnerException != null);
                    throw ae.InnerException;
                }
                catch (Base b)
                {
                    TestHelper.Assert(b.b.Equals("UnknownMostDerived2.b"));
                    TestHelper.Assert(b.GetType().Name.Equals("Base"));
                }
                catch (Exception)
                {
                    TestHelper.Assert(false);
                }
            }
        }
        output.WriteLine("ok");

        output.Write("unknown most derived in compact format... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownMostDerived2AsBaseCompact();
                TestHelper.Assert(false);
            }
            catch (Base)
            {
                // Exceptions are always marshaled in sliced format; format:compact applies only to in-parameters and
                // return values.
            }
            catch (OperationNotExistException)
            {
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("completely unknown server-private exception... ");
        output.Flush();
        {
            try
            {
                testPrx.serverPrivateException();
                TestHelper.Assert(false);
            }
            catch (RemoteException ex)
            {
                SlicedData slicedData = ex.GetSlicedData()!.Value;
                TestHelper.Assert(slicedData.Slices.Count == 1);
                TestHelper.Assert(slicedData.Slices[0].TypeId! == "::Test::ServerPrivateException");
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
        }
        output.WriteLine("ok");

        output.Write("preserved exceptions... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownPreservedAsBase();
                TestHelper.Assert(false);
            }
            catch (Base ex)
            {
                IReadOnlyList<SliceInfo> slices = ex.GetSlicedData()!.Value.Slices;
                TestHelper.Assert(slices.Count == 2);
                TestHelper.Assert(slices[1].TypeId!.Equals("::Test::SPreserved1"));
                TestHelper.Assert(slices[0].TypeId!.Equals("::Test::SPreserved2"));
            }

            try
            {
                testPrx.unknownPreservedAsKnownPreserved();
                TestHelper.Assert(false);
            }
            catch (KnownPreserved ex)
            {
                TestHelper.Assert(ex.kp.Equals("preserved"));
                IReadOnlyList<SliceInfo> slices = ex.GetSlicedData()!.Value.Slices;
                TestHelper.Assert(slices.Count == 2);
                TestHelper.Assert(slices[1].TypeId!.Equals("::Test::SPreserved1"));
                TestHelper.Assert(slices[0].TypeId!.Equals("::Test::SPreserved2"));
            }

            ObjectAdapter adapter = communicator.CreateObjectAdapter();
            IRelayPrx relay = adapter.AddWithUUID(new Relay(), IRelayPrx.Factory);
            adapter.Activate();
            testPrx.GetConnection().Adapter = adapter;

            try
            {
                testPrx.relayKnownPreservedAsBase(relay);
                TestHelper.Assert(false);
            }
            catch (KnownPreservedDerived ex)
            {
                TestHelper.Assert(ex.b.Equals("base"));
                TestHelper.Assert(ex.kp.Equals("preserved"));
                TestHelper.Assert(ex.kpd.Equals("derived"));
            }
            catch (Ice.OperationNotExistException)
            {
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                testPrx.relayKnownPreservedAsKnownPreserved(relay);
                TestHelper.Assert(false);
            }
            catch (KnownPreservedDerived ex)
            {
                TestHelper.Assert(ex.b.Equals("base"));
                TestHelper.Assert(ex.kp.Equals("preserved"));
                TestHelper.Assert(ex.kpd.Equals("derived"));
            }
            catch (Ice.OperationNotExistException)
            {
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                testPrx.relayUnknownPreservedAsBase(relay);
                TestHelper.Assert(false);
            }
            catch (Preserved2 ex)
            {
                TestHelper.Assert(ex.b.Equals("base"));
                TestHelper.Assert(ex.kp.Equals("preserved"));
                TestHelper.Assert(ex.kpd.Equals("derived"));
                TestHelper.Assert(ex.p1!.GetType().GetIceTypeId()!.Equals(typeof(PreservedClass).GetIceTypeId()));
                var pc = ex.p1 as PreservedClass;
                TestHelper.Assert(pc!.bc.Equals("bc"));
                TestHelper.Assert(pc!.pc.Equals("pc"));
                TestHelper.Assert(ex.p2 == ex.p1);
            }
            catch (OperationNotExistException)
            {
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                testPrx.relayUnknownPreservedAsKnownPreserved(relay);
                TestHelper.Assert(false);
            }
            catch (Preserved2 ex)
            {
                TestHelper.Assert(ex.b.Equals("base"));
                TestHelper.Assert(ex.kp.Equals("preserved"));
                TestHelper.Assert(ex.kpd.Equals("derived"));
                TestHelper.Assert(ex.p1!.GetType().GetIceTypeId()!.Equals(typeof(PreservedClass).GetIceTypeId()));
                var pc = ex.p1 as PreservedClass;
                TestHelper.Assert(pc!.bc.Equals("bc"));
                TestHelper.Assert(pc!.pc.Equals("pc"));
                TestHelper.Assert(ex.p2 == ex.p1);
            }
            catch (OperationNotExistException)
            {
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }

            try
            {
                testPrx.relayClientPrivateException(relay);
                TestHelper.Assert(false);
            }
            catch (ClientPrivateException ex)
            {
                TestHelper.Assert(ex.cpe == "ClientPrivate");
            }
            catch (System.Exception)
            {
                TestHelper.Assert(false);
            }

            adapter.Destroy();
        }
        output.WriteLine("ok");

        return testPrx;
    }
}
