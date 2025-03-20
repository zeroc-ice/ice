// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using Test;

public class AllTests : Test.AllTests
{
    private class Callback
    {
        internal Callback() => _called = false;

        public virtual void check()
        {
            lock (this)
            {
                while (!_called)
                {
                    Monitor.Wait(this);
                }

                _called = false;
            }
        }

        public virtual void called()
        {
            lock (this)
            {
                Debug.Assert(!_called);
                _called = true;
                Monitor.Pulse(this);
            }
        }

        private bool _called;
    }

    public static async Task<TestIntfPrx> allTests(Test.TestHelper helper, bool collocated)
    {
        Ice.Communicator communicator = helper.communicator();
        var output = helper.getWriter();
        output.Write("testing stringToProxy... ");
        output.Flush();
        string @ref = "Test:" + helper.getTestEndpoint(0) + " -t 2000";
        Ice.ObjectPrx @base = communicator.stringToProxy(@ref);
        test(@base != null);
        output.WriteLine("ok");

        output.Write("testing checked cast... ");
        output.Flush();
        TestIntfPrx testPrx = TestIntfPrxHelper.checkedCast(@base);
        test(testPrx != null);
        test(testPrx.Equals(@base));
        output.WriteLine("ok");

        output.Write("base... ");
        output.Flush();
        {
            try
            {
                testPrx.baseAsBase();
                test(false);
            }
            catch (Base b)
            {
                test(b.b == "Base.b");
                test(b.GetType().FullName == "Test.Base");
            }
            catch (Exception)
            {
                test(false);
            }
        }
        output.WriteLine("ok");

        output.Write("base (AMI)... ");
        output.Flush();
        {
            try
            {
                await testPrx.baseAsBaseAsync();
            }
            catch (Base b)
            {
                test(b.b == "Base.b");
                test(b.GetType().Name == "Base");
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown derived... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownDerivedAsBase();
                test(false);
            }
            catch (Base b)
            {
                test(b.b == "UnknownDerived.b");
                test(b.GetType().FullName == "Test.Base");
            }
            catch (Exception)
            {
                test(false);
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown derived (AMI)... ");
        output.Flush();
        {
            try
            {
                await testPrx.unknownDerivedAsBaseAsync();
                test(false);
            }
            catch (Base b)
            {
                test(b.b == "UnknownDerived.b");
                test(b.GetType().Name == "Base");
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known derived as base... ");
        output.Flush();
        {
            try
            {
                testPrx.knownDerivedAsBase();
                test(false);
            }
            catch (KnownDerived k)
            {
                test(k.b == "KnownDerived.b");
                test(k.kd == "KnownDerived.kd");
                test(k.GetType().FullName == "Test.KnownDerived");
            }
            catch (Exception)
            {
                test(false);
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known derived as base (AMI)... ");
        output.Flush();
        {
            try
            {
                await testPrx.knownDerivedAsBaseAsync();
                test(false);
            }
            catch (KnownDerived k)
            {
                test(k.b == "KnownDerived.b");
                test(k.kd == "KnownDerived.kd");
                test(k.GetType().Name == "KnownDerived");
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known derived as derived... ");
        output.Flush();
        {
            try
            {
                testPrx.knownDerivedAsKnownDerived();
                test(false);
            }
            catch (KnownDerived k)
            {
                test(k.b == "KnownDerived.b");
                test(k.kd == "KnownDerived.kd");
                test(k.GetType().FullName == "Test.KnownDerived");
            }
            catch (Exception)
            {
                test(false);
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known derived as derived (AMI)... ");
        output.Flush();
        {
            try
            {
                await testPrx.knownDerivedAsKnownDerivedAsync();
                test(false);
            }
            catch (KnownDerived k)
            {
                test(k.b == "KnownDerived.b");
                test(k.kd == "KnownDerived.kd");
                test(k.GetType().Name == "KnownDerived");
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown intermediate as base... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownIntermediateAsBase();
                test(false);
            }
            catch (Base b)
            {
                test(b.b == "UnknownIntermediate.b");
                test(b.GetType().FullName == "Test.Base");
            }
            catch (Exception)
            {
                test(false);
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown intermediate as base (AMI)... ");
        output.Flush();
        {
            try
            {
                await testPrx.unknownIntermediateAsBaseAsync();
                test(false);
            }
            catch (Base b)
            {
                test(b.b == "UnknownIntermediate.b");
                test(b.GetType().Name == "Base");
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of known intermediate as base... ");
        output.Flush();
        {
            try
            {
                testPrx.knownIntermediateAsBase();
                test(false);
            }
            catch (KnownIntermediate ki)
            {
                test(ki.b == "KnownIntermediate.b");
                test(ki.ki == "KnownIntermediate.ki");
                test(ki.GetType().FullName == "Test.KnownIntermediate");
            }
            catch (Exception)
            {
                test(false);
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of known intermediate as base (AMI)... ");
        output.Flush();
        {
            try
            {
                await testPrx.knownIntermediateAsBaseAsync();
                test(false);
            }
            catch (KnownIntermediate ki)
            {
                test(ki.b == "KnownIntermediate.b");
                test(ki.ki == "KnownIntermediate.ki");
                test(ki.GetType().Name == "KnownIntermediate");
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of known most derived as base... ");
        output.Flush();
        {
            try
            {
                testPrx.knownMostDerivedAsBase();
                test(false);
            }
            catch (KnownMostDerived kmd)
            {
                test(kmd.b == "KnownMostDerived.b");
                test(kmd.ki == "KnownMostDerived.ki");
                test(kmd.kmd == "KnownMostDerived.kmd");
                test(kmd.GetType().FullName == "Test.KnownMostDerived");
            }
            catch (Exception)
            {
                test(false);
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of known most derived as base (AMI)... ");
        output.Flush();
        {
            try
            {
                await testPrx.knownMostDerivedAsBaseAsync();
                test(false);
            }
            catch (KnownMostDerived kmd)
            {
                test(kmd.b == "KnownMostDerived.b");
                test(kmd.ki == "KnownMostDerived.ki");
                test(kmd.kmd == "KnownMostDerived.kmd");
                test(kmd.GetType().Name == "KnownMostDerived");
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known intermediate as intermediate... ");
        output.Flush();
        {
            try
            {
                testPrx.knownIntermediateAsKnownIntermediate();
                test(false);
            }
            catch (KnownIntermediate ki)
            {
                test(ki.b == "KnownIntermediate.b");
                test(ki.ki == "KnownIntermediate.ki");
                test(ki.GetType().FullName == "Test.KnownIntermediate");
            }
            catch (Exception)
            {
                test(false);
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known intermediate as intermediate (AMI)... ");
        output.Flush();
        {
            try
            {
                await testPrx.knownIntermediateAsKnownIntermediateAsync();
                test(false);
            }
            catch (KnownIntermediate ki)
            {
                test(ki.b == "KnownIntermediate.b");
                test(ki.ki == "KnownIntermediate.ki");
                test(ki.GetType().Name == "KnownIntermediate");
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known most derived as intermediate... ");
        output.Flush();
        {
            try
            {
                testPrx.knownMostDerivedAsKnownIntermediate();
                test(false);
            }
            catch (KnownMostDerived kmd)
            {
                test(kmd.b == "KnownMostDerived.b");
                test(kmd.ki == "KnownMostDerived.ki");
                test(kmd.kmd == "KnownMostDerived.kmd");
                test(kmd.GetType().FullName == "Test.KnownMostDerived");
            }
            catch (Exception)
            {
                test(false);
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known most derived as intermediate (AMI)... ");
        output.Flush();
        {
            try
            {
                await testPrx.knownMostDerivedAsKnownIntermediateAsync();
                test(false);
            }
            catch (KnownMostDerived kmd)
            {
                test(kmd.b == "KnownMostDerived.b");
                test(kmd.ki == "KnownMostDerived.ki");
                test(kmd.kmd == "KnownMostDerived.kmd");
                test(kmd.GetType().Name == "KnownMostDerived");
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known most derived as most derived... ");
        output.Flush();
        {
            try
            {
                testPrx.knownMostDerivedAsKnownMostDerived();
                test(false);
            }
            catch (KnownMostDerived kmd)
            {
                test(kmd.b == "KnownMostDerived.b");
                test(kmd.ki == "KnownMostDerived.ki");
                test(kmd.kmd == "KnownMostDerived.kmd");
                test(kmd.GetType().FullName == "Test.KnownMostDerived");
            }
            catch (Exception)
            {
                test(false);
            }
        }
        output.WriteLine("ok");

        output.Write("non-slicing of known most derived as most derived (AMI)... ");
        output.Flush();
        {
            try
            {
                await testPrx.knownMostDerivedAsKnownMostDerivedAsync();
                test(false);
            }
            catch (KnownMostDerived kmd)
            {
                test(kmd.b == "KnownMostDerived.b");
                test(kmd.ki == "KnownMostDerived.ki");
                test(kmd.kmd == "KnownMostDerived.kmd");
                test(kmd.GetType().Name == "KnownMostDerived");
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown most derived, known intermediate as base... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownMostDerived1AsBase();
                test(false);
            }
            catch (KnownIntermediate ki)
            {
                test(ki.b == "UnknownMostDerived1.b");
                test(ki.ki == "UnknownMostDerived1.ki");
                test(ki.GetType().FullName == "Test.KnownIntermediate");
            }
            catch (Exception)
            {
                test(false);
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown most derived, known intermediate as base (AMI)... ");
        output.Flush();
        {
            try
            {
                await testPrx.unknownMostDerived1AsBaseAsync();
                test(false);
            }
            catch (KnownIntermediate ki)
            {
                test(ki.b == "UnknownMostDerived1.b");
                test(ki.ki == "UnknownMostDerived1.ki");
                test(ki.GetType().Name == "KnownIntermediate");
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown most derived, known intermediate as intermediate... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownMostDerived1AsKnownIntermediate();
                test(false);
            }
            catch (KnownIntermediate ki)
            {
                test(ki.b == "UnknownMostDerived1.b");
                test(ki.ki == "UnknownMostDerived1.ki");
                test(ki.GetType().FullName == "Test.KnownIntermediate");
            }
            catch (Exception)
            {
                test(false);
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown most derived, known intermediate as intermediate (AMI)... ");
        output.Flush();
        {
            try
            {
                await testPrx.unknownMostDerived1AsKnownIntermediateAsync();
                test(false);
            }
            catch (KnownIntermediate ki)
            {
                test(ki.b == "UnknownMostDerived1.b");
                test(ki.ki == "UnknownMostDerived1.ki");
                test(ki.GetType().Name == "KnownIntermediate");
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown most derived, unknown intermediate thrown as base... ");
        output.Flush();
        {
            try
            {
                testPrx.unknownMostDerived2AsBase();
                test(false);
            }
            catch (Base b)
            {
                test(b.b == "UnknownMostDerived2.b");
                test(b.GetType().FullName == "Test.Base");
            }
            catch (Exception)
            {
                test(false);
            }
        }
        output.WriteLine("ok");

        output.Write("slicing of unknown most derived, unknown intermediate thrown as base (AMI)... ");
        output.Flush();
        {
            try
            {
                await testPrx.unknownMostDerived2AsBaseAsync();
                test(false);
            }
            catch (Base b)
            {
                test(b.b == "UnknownMostDerived2.b");
                test(b.GetType().Name == "Base");
            }
        }
        output.WriteLine("ok");

        return testPrx;
    }
}
