// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Slicing.Exceptions
{
    public static class AllTests
    {
        private class Relay : IRelay
        {
            public void KnownPreservedAsBase(Current current, CancellationToken cancel) =>
                throw new KnownPreservedDerived("base", "preserved", "derived");

            public void KnownPreservedAsKnownPreserved(Current current, CancellationToken cancel) =>
                throw new KnownPreservedDerived("base", "preserved", "derived");

            public void UnknownPreservedAsBase(Current current, CancellationToken cancel)
            {
                var p = new PreservedClass("bc", "pc");
                throw new Preserved2("base", "preserved", "derived", p, p);
            }

            public void UnknownPreservedAsKnownPreserved(Current current, CancellationToken cancel)
            {
                var p = new PreservedClass("bc", "pc");
                throw new Preserved2("base", "preserved", "derived", p, p);
            }

            public void ClientPrivateException(Current current, CancellationToken cancel) =>
                throw new ClientPrivateException("ClientPrivate");
        }

        public static async Task RunAsync(TestHelper helper)
        {
            Communicator communicator = helper.Communicator;

            TextWriter output = helper.Output;
            output.Write("testing stringToProxy... ");
            output.Flush();
            var testPrx = ITestIntfPrx.Parse(helper.GetTestProxy("Test", 0), communicator);
            output.WriteLine("ok");

            output.Write("base... ");
            output.Flush();
            {
                try
                {
                    testPrx.BaseAsBase();
                    TestHelper.Assert(false);
                }
                catch (Base b)
                {
                    TestHelper.Assert(b.B.Equals("Base.b"));
                    TestHelper.Assert(b.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Exceptions.Base"));
                }
                catch
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
                    testPrx.BaseAsBaseAsync().Wait();
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
                        TestHelper.Assert(b.B.Equals("Base.b"));
                        TestHelper.Assert(b.GetType().Name.Equals("Base"));
                    }
                    catch
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
                    testPrx.UnknownDerivedAsBase();
                    TestHelper.Assert(false);
                }
                catch (Base b)
                {
                    TestHelper.Assert(b.B.Equals("UnknownDerived.b"));
                    TestHelper.Assert(b.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Exceptions.Base"));
                }
                catch
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
                    testPrx.UnknownDerivedAsBaseAsync().Wait();
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
                        TestHelper.Assert(b.B.Equals("UnknownDerived.b"));
                        TestHelper.Assert(b.GetType().Name.Equals("Base"));
                    }
                    catch
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
                    testPrx.KnownDerivedAsBase();
                    TestHelper.Assert(false);
                }
                catch (KnownDerived k)
                {
                    TestHelper.Assert(k.B.Equals("KnownDerived.b"));
                    TestHelper.Assert(k.Kd.Equals("KnownDerived.kd"));
                    TestHelper.Assert(k.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Exceptions.KnownDerived"));
                }
                catch
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
                    testPrx.KnownDerivedAsBaseAsync().Wait();
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
                        TestHelper.Assert(k.B.Equals("KnownDerived.b"));
                        TestHelper.Assert(k.Kd.Equals("KnownDerived.kd"));
                        TestHelper.Assert(k.GetType().Name.Equals("KnownDerived"));
                    }
                    catch
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
                    testPrx.KnownDerivedAsKnownDerived();
                    TestHelper.Assert(false);
                }
                catch (KnownDerived k)
                {
                    TestHelper.Assert(k.B.Equals("KnownDerived.b"));
                    TestHelper.Assert(k.Kd.Equals("KnownDerived.kd"));
                    TestHelper.Assert(k.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Exceptions.KnownDerived"));
                }
                catch
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
                    testPrx.KnownDerivedAsKnownDerivedAsync().Wait();
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
                        TestHelper.Assert(k.B.Equals("KnownDerived.b"));
                        TestHelper.Assert(k.Kd.Equals("KnownDerived.kd"));
                        TestHelper.Assert(k.GetType().Name.Equals("KnownDerived"));
                    }
                    catch
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
                    testPrx.UnknownIntermediateAsBase();
                    TestHelper.Assert(false);
                }
                catch (Base b)
                {
                    TestHelper.Assert(b.B.Equals("UnknownIntermediate.b"));
                    TestHelper.Assert(b.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Exceptions.Base"));
                }
                catch
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
                    testPrx.UnknownIntermediateAsBaseAsync().Wait();
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
                        TestHelper.Assert(b.B.Equals("UnknownIntermediate.b"));
                        TestHelper.Assert(b.GetType().Name.Equals("Base"));
                    }
                    catch
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
                    testPrx.KnownIntermediateAsBase();
                    TestHelper.Assert(false);
                }
                catch (KnownIntermediate ki)
                {
                    TestHelper.Assert(ki.B.Equals("KnownIntermediate.b"));
                    TestHelper.Assert(ki.Ki.Equals("KnownIntermediate.ki"));
                    TestHelper.Assert(ki.GetType().FullName!.Equals(
                        "ZeroC.Ice.Test.Slicing.Exceptions.KnownIntermediate"));
                }
                catch
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
                    testPrx.KnownIntermediateAsBaseAsync().Wait();
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
                        TestHelper.Assert(ki.B.Equals("KnownIntermediate.b"));
                        TestHelper.Assert(ki.Ki.Equals("KnownIntermediate.ki"));
                        TestHelper.Assert(ki.GetType().Name.Equals("KnownIntermediate"));
                    }
                    catch
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
                    testPrx.KnownMostDerivedAsBase();
                    TestHelper.Assert(false);
                }
                catch (KnownMostDerived kmd)
                {
                    TestHelper.Assert(kmd.B.Equals("KnownMostDerived.b"));
                    TestHelper.Assert(kmd.Ki.Equals("KnownMostDerived.ki"));
                    TestHelper.Assert(kmd.Kmd.Equals("KnownMostDerived.kmd"));
                    TestHelper.Assert(kmd.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Exceptions.KnownMostDerived"));
                }
                catch
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
                    testPrx.KnownMostDerivedAsBaseAsync().Wait();
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
                        TestHelper.Assert(kmd.B.Equals("KnownMostDerived.b"));
                        TestHelper.Assert(kmd.Ki.Equals("KnownMostDerived.ki"));
                        TestHelper.Assert(kmd.Kmd.Equals("KnownMostDerived.kmd"));
                        TestHelper.Assert(kmd.GetType().Name.Equals("KnownMostDerived"));
                    }
                    catch
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
                    testPrx.KnownIntermediateAsKnownIntermediate();
                    TestHelper.Assert(false);
                }
                catch (KnownIntermediate ki)
                {
                    TestHelper.Assert(ki.B.Equals("KnownIntermediate.b"));
                    TestHelper.Assert(ki.Ki.Equals("KnownIntermediate.ki"));
                    TestHelper.Assert(ki.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Exceptions.KnownIntermediate"));
                }
                catch
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
                    testPrx.KnownIntermediateAsKnownIntermediateAsync().Wait();
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
                        TestHelper.Assert(ki.B.Equals("KnownIntermediate.b"));
                        TestHelper.Assert(ki.Ki.Equals("KnownIntermediate.ki"));
                        TestHelper.Assert(ki.GetType().Name.Equals("KnownIntermediate"));
                    }
                    catch
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
                    testPrx.KnownMostDerivedAsKnownIntermediate();
                    TestHelper.Assert(false);
                }
                catch (KnownMostDerived kmd)
                {
                    TestHelper.Assert(kmd.B.Equals("KnownMostDerived.b"));
                    TestHelper.Assert(kmd.Ki.Equals("KnownMostDerived.ki"));
                    TestHelper.Assert(kmd.Kmd.Equals("KnownMostDerived.kmd"));
                    TestHelper.Assert(kmd.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Exceptions.KnownMostDerived"));
                }
                catch
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
                    testPrx.KnownMostDerivedAsKnownIntermediateAsync().Wait();
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
                        TestHelper.Assert(kmd.B.Equals("KnownMostDerived.b"));
                        TestHelper.Assert(kmd.Ki.Equals("KnownMostDerived.ki"));
                        TestHelper.Assert(kmd.Kmd.Equals("KnownMostDerived.kmd"));
                        TestHelper.Assert(kmd.GetType().Name.Equals("KnownMostDerived"));
                    }
                    catch
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
                    testPrx.KnownMostDerivedAsKnownMostDerived();
                    TestHelper.Assert(false);
                }
                catch (KnownMostDerived kmd)
                {
                    TestHelper.Assert(kmd.B.Equals("KnownMostDerived.b"));
                    TestHelper.Assert(kmd.Ki.Equals("KnownMostDerived.ki"));
                    TestHelper.Assert(kmd.Kmd.Equals("KnownMostDerived.kmd"));
                    TestHelper.Assert(kmd.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Exceptions.KnownMostDerived"));
                }
                catch
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
                    testPrx.KnownMostDerivedAsKnownMostDerivedAsync().Wait();
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
                        TestHelper.Assert(kmd.B.Equals("KnownMostDerived.b"));
                        TestHelper.Assert(kmd.Ki.Equals("KnownMostDerived.ki"));
                        TestHelper.Assert(kmd.Kmd.Equals("KnownMostDerived.kmd"));
                        TestHelper.Assert(kmd.GetType().Name.Equals("KnownMostDerived"));
                    }
                    catch
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
                    testPrx.UnknownMostDerived1AsBase();
                    TestHelper.Assert(false);
                }
                catch (KnownIntermediate ki)
                {
                    TestHelper.Assert(ki.B.Equals("UnknownMostDerived1.b"));
                    TestHelper.Assert(ki.Ki.Equals("UnknownMostDerived1.ki"));
                    TestHelper.Assert(ki.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Exceptions.KnownIntermediate"));
                }
                catch
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
                    testPrx.UnknownMostDerived1AsBaseAsync().Wait();
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
                        TestHelper.Assert(ki.B.Equals("UnknownMostDerived1.b"));
                        TestHelper.Assert(ki.Ki.Equals("UnknownMostDerived1.ki"));
                        TestHelper.Assert(ki.GetType().Name.Equals("KnownIntermediate"));
                    }
                    catch
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
                    testPrx.UnknownMostDerived1AsKnownIntermediate();
                    TestHelper.Assert(false);
                }
                catch (KnownIntermediate ki)
                {
                    TestHelper.Assert(ki.B.Equals("UnknownMostDerived1.b"));
                    TestHelper.Assert(ki.Ki.Equals("UnknownMostDerived1.ki"));
                    TestHelper.Assert(ki.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Exceptions.KnownIntermediate"));
                }
                catch
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
                    testPrx.UnknownMostDerived1AsKnownIntermediateAsync().Wait();
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
                        TestHelper.Assert(ki.B.Equals("UnknownMostDerived1.b"));
                        TestHelper.Assert(ki.Ki.Equals("UnknownMostDerived1.ki"));
                        TestHelper.Assert(ki.GetType().Name.Equals("KnownIntermediate"));
                    }
                    catch
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
                    testPrx.UnknownMostDerived2AsBase();
                    TestHelper.Assert(false);
                }
                catch (Base b)
                {
                    TestHelper.Assert(b.B.Equals("UnknownMostDerived2.b"));
                    TestHelper.Assert(b.GetType().FullName!.Equals("ZeroC.Ice.Test.Slicing.Exceptions.Base"));
                }
                catch
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
                    testPrx.UnknownMostDerived2AsBaseAsync().Wait();
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
                        TestHelper.Assert(b.B.Equals("UnknownMostDerived2.b"));
                        TestHelper.Assert(b.GetType().Name.Equals("Base"));
                    }
                    catch
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
                    testPrx.UnknownMostDerived2AsBaseCompact();
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
                catch
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
                    testPrx.ServerPrivateException();
                    TestHelper.Assert(false);
                }
                catch (RemoteException ex)
                {
                    SlicedData slicedData = ex.GetSlicedData()!.Value;
                    TestHelper.Assert(slicedData.Slices.Count == 1);
                    TestHelper.Assert(slicedData.Slices[0].TypeId! == "::ZeroC::Ice::Test::Slicing::Exceptions::ServerPrivateException");
                }
                catch
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
                    testPrx.UnknownPreservedAsBase();
                    TestHelper.Assert(false);
                }
                catch (Base ex)
                {
                    IReadOnlyList<SliceInfo> slices = ex.GetSlicedData()!.Value.Slices;
                    TestHelper.Assert(slices.Count == 2);
                    TestHelper.Assert(slices[1].TypeId!.Equals("::ZeroC::Ice::Test::Slicing::Exceptions::SPreserved1"));
                    TestHelper.Assert(slices[0].TypeId!.Equals("::ZeroC::Ice::Test::Slicing::Exceptions::SPreserved2"));
                }

                try
                {
                    testPrx.UnknownPreservedAsKnownPreserved();
                    TestHelper.Assert(false);
                }
                catch (KnownPreserved ex)
                {
                    TestHelper.Assert(ex.Kp.Equals("preserved"));
                    IReadOnlyList<SliceInfo> slices = ex.GetSlicedData()!.Value.Slices;
                    TestHelper.Assert(slices.Count == 2);
                    TestHelper.Assert(slices[1].TypeId!.Equals("::ZeroC::Ice::Test::Slicing::Exceptions::SPreserved1"));
                    TestHelper.Assert(slices[0].TypeId!.Equals("::ZeroC::Ice::Test::Slicing::Exceptions::SPreserved2"));
                }

                await using var adapter = communicator.CreateObjectAdapter(protocol: helper.Protocol);
                IRelayPrx relay = adapter.AddWithUUID(new Relay(), IRelayPrx.Factory);
                await adapter.ActivateAsync();
                (await testPrx.GetConnectionAsync()).Adapter = adapter;

                try
                {
                    testPrx.RelayKnownPreservedAsBase(relay);
                    TestHelper.Assert(false);
                }
                catch (KnownPreservedDerived ex)
                {
                    TestHelper.Assert(ex.B.Equals("base"));
                    TestHelper.Assert(ex.Kp.Equals("preserved"));
                    TestHelper.Assert(ex.Kpd.Equals("derived"));
                }
                catch (OperationNotExistException)
                {
                }
                catch
                {
                    TestHelper.Assert(false);
                }

                try
                {
                    testPrx.RelayKnownPreservedAsKnownPreserved(relay);
                    TestHelper.Assert(false);
                }
                catch (KnownPreservedDerived ex)
                {
                    TestHelper.Assert(ex.B.Equals("base"));
                    TestHelper.Assert(ex.Kp.Equals("preserved"));
                    TestHelper.Assert(ex.Kpd.Equals("derived"));
                }
                catch (OperationNotExistException)
                {
                }
                catch
                {
                    TestHelper.Assert(false);
                }

                try
                {
                    testPrx.RelayUnknownPreservedAsBase(relay);
                    TestHelper.Assert(false);
                }
                catch (Preserved2 ex)
                {
                    TestHelper.Assert(ex.B.Equals("base"));
                    TestHelper.Assert(ex.Kp.Equals("preserved"));
                    TestHelper.Assert(ex.Kpd.Equals("derived"));
                    TestHelper.Assert(ex.P1!.GetType().GetIceTypeId()!.Equals(typeof(PreservedClass).GetIceTypeId()));
                    var pc = ex.P1 as PreservedClass;
                    TestHelper.Assert(pc!.Bc.Equals("bc"));
                    TestHelper.Assert(pc!.Pc.Equals("pc"));
                    TestHelper.Assert(ex.P2 == ex.P1);
                }
                catch (OperationNotExistException)
                {
                }
                catch
                {
                    TestHelper.Assert(false);
                }

                try
                {
                    testPrx.RelayUnknownPreservedAsKnownPreserved(relay);
                    TestHelper.Assert(false);
                }
                catch (Preserved2 ex)
                {
                    TestHelper.Assert(ex.B.Equals("base"));
                    TestHelper.Assert(ex.Kp.Equals("preserved"));
                    TestHelper.Assert(ex.Kpd.Equals("derived"));
                    TestHelper.Assert(ex.P1!.GetType().GetIceTypeId()!.Equals(typeof(PreservedClass).GetIceTypeId()));
                    var pc = ex.P1 as PreservedClass;
                    TestHelper.Assert(pc!.Bc.Equals("bc"));
                    TestHelper.Assert(pc!.Pc.Equals("pc"));
                    TestHelper.Assert(ex.P2 == ex.P1);
                }
                catch (OperationNotExistException)
                {
                }
                catch
                {
                    TestHelper.Assert(false);
                }

                try
                {
                    testPrx.RelayClientPrivateException(relay);
                    TestHelper.Assert(false);
                }
                catch (ClientPrivateException ex)
                {
                    TestHelper.Assert(ex.Cpe == "ClientPrivate");
                }
                catch
                {
                    TestHelper.Assert(false);
                }
            }
            output.WriteLine("ok");
            await testPrx.ShutdownAsync();
        }
    }
}
