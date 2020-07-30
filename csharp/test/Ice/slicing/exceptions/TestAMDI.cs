//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Slicing.Exceptions
{
    public sealed class TestIntfAsync : ITestIntfAsync
    {
        public ValueTask ShutdownAsync(Current current)
        {
            _ = current.Adapter.Communicator.ShutdownAsync();
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask BaseAsBaseAsync(Current current) => throw new Base("Base.b");

        public ValueTask UnknownDerivedAsBaseAsync(Current current) =>
            throw new UnknownDerived("UnknownDerived.b", "UnknownDerived.ud");

        public ValueTask KnownDerivedAsBaseAsync(Current current) =>
            throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");

        public ValueTask KnownDerivedAsKnownDerivedAsync(Current current) =>
            throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");

        public ValueTask UnknownIntermediateAsBaseAsync(Current current) =>
            throw new UnknownIntermediate("UnknownIntermediate.b", "UnknownIntermediate.ui");

        public ValueTask KnownIntermediateAsBaseAsync(Current current) =>
            throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");

        public ValueTask KnownMostDerivedAsBaseAsync(Current current) =>
            throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

        public ValueTask KnownIntermediateAsKnownIntermediateAsync(Current current) =>
            throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");

        public ValueTask KnownMostDerivedAsKnownIntermediateAsync(Current current) =>
            throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

        public ValueTask KnownMostDerivedAsKnownMostDerivedAsync(Current current) =>
            throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

        public ValueTask UnknownMostDerived1AsBaseAsync(Current current) =>
            throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");

        public ValueTask UnknownMostDerived1AsKnownIntermediateAsync(Current current) =>
            throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");

        public ValueTask UnknownMostDerived2AsBaseAsync(Current current) =>
            throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");

        public ValueTask UnknownMostDerived2AsBaseCompactAsync(Current current) =>
            throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");

        public ValueTask KnownPreservedAsBaseAsync(Current current) =>
            throw new KnownPreservedDerived("base", "preserved", "derived");

        public ValueTask KnownPreservedAsKnownPreservedAsync(Current current) =>
            throw new KnownPreservedDerived("base", "preserved", "derived");

        public ValueTask ServerPrivateExceptionAsync(Current current) =>
            throw new ServerPrivateException("ServerPrivate");

        public async ValueTask RelayKnownPreservedAsBaseAsync(IRelayPrx? r, Current current)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                await p.KnownPreservedAsBaseAsync();
            }
            catch (RemoteException ex)
            {
                TestHelper.Assert(ex.ConvertToUnhandled);
                ex.ConvertToUnhandled = false;
                throw;
            }
            TestHelper.Assert(false);
        }

        public async ValueTask RelayKnownPreservedAsKnownPreservedAsync(IRelayPrx? r, Current current)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                await p.KnownPreservedAsKnownPreservedAsync();
            }
            catch (RemoteException ex)
            {
                TestHelper.Assert(ex.ConvertToUnhandled);
                ex.ConvertToUnhandled = false;
                throw;
            }
            TestHelper.Assert(false);
        }

        public ValueTask UnknownPreservedAsBaseAsync(Current current)
        {
            var p = new SPreservedClass("bc", "spc");
            throw new SPreserved2("base", "preserved", "derived", p, p);
        }

        public ValueTask UnknownPreservedAsKnownPreservedAsync(Current current)
        {
            var p = new SPreservedClass("bc", "spc");
            throw new SPreserved2("base", "preserved", "derived", p, p);
        }

        public async ValueTask RelayUnknownPreservedAsBaseAsync(IRelayPrx? r, Current current)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                await p.UnknownPreservedAsBaseAsync();
            }
            catch (RemoteException ex)
            {
                TestHelper.Assert(ex.ConvertToUnhandled);
                ex.ConvertToUnhandled = false;
                throw;
            }
            TestHelper.Assert(false);
        }

        public async ValueTask RelayUnknownPreservedAsKnownPreservedAsync(IRelayPrx? r, Current current)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                await p.UnknownPreservedAsKnownPreservedAsync();
            }
            catch (RemoteException ex)
            {
                TestHelper.Assert(ex.ConvertToUnhandled);
                ex.ConvertToUnhandled = false;
                throw;
            }
            TestHelper.Assert(false);
        }

        public async ValueTask RelayClientPrivateExceptionAsync(IRelayPrx? r, Current current)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                await p.ClientPrivateExceptionAsync();
            }
            catch (RemoteException ex)
            {
                TestHelper.Assert(ex.ConvertToUnhandled);
                ex.ConvertToUnhandled = false;
                throw;
            }
            TestHelper.Assert(false);
        }
    }
}
