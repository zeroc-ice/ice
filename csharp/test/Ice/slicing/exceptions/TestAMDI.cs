// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Slicing.Exceptions
{
    public sealed class AsyncTestIntf : IAsyncTestIntf
    {
        public ValueTask ShutdownAsync(Current current, CancellationToken cancel)
        {
            _ = current.Communicator.ShutdownAsync();
            return default;
        }

        public ValueTask BaseAsBaseAsync(Current current, CancellationToken cancel) => throw new Base("Base.b");

        public ValueTask UnknownDerivedAsBaseAsync(Current current, CancellationToken cancel) =>
            throw new UnknownDerived("UnknownDerived.b", "UnknownDerived.ud");

        public ValueTask KnownDerivedAsBaseAsync(Current current, CancellationToken cancel) =>
            throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");

        public ValueTask KnownDerivedAsKnownDerivedAsync(Current current, CancellationToken cancel) =>
            throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");

        public ValueTask UnknownIntermediateAsBaseAsync(Current current, CancellationToken cancel) =>
            throw new UnknownIntermediate("UnknownIntermediate.b", "UnknownIntermediate.ui");

        public ValueTask KnownIntermediateAsBaseAsync(Current current, CancellationToken cancel) =>
            throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");

        public ValueTask KnownMostDerivedAsBaseAsync(Current current, CancellationToken cancel) =>
            throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

        public ValueTask KnownIntermediateAsKnownIntermediateAsync(Current current, CancellationToken cancel) =>
            throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");

        public ValueTask KnownMostDerivedAsKnownIntermediateAsync(Current current, CancellationToken cancel) =>
            throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

        public ValueTask KnownMostDerivedAsKnownMostDerivedAsync(Current current, CancellationToken cancel) =>
            throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

        public ValueTask UnknownMostDerived1AsBaseAsync(Current current, CancellationToken cancel) =>
            throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");

        public ValueTask UnknownMostDerived1AsKnownIntermediateAsync(Current current, CancellationToken cancel) =>
            throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");

        public ValueTask UnknownMostDerived2AsBaseAsync(Current current, CancellationToken cancel) =>
            throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");

        public ValueTask UnknownMostDerived2AsBaseCompactAsync(Current current, CancellationToken cancel) =>
            throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");

        public ValueTask KnownPreservedAsBaseAsync(Current current, CancellationToken cancel) =>
            throw new KnownPreservedDerived("base", "preserved", "derived");

        public ValueTask KnownPreservedAsKnownPreservedAsync(Current current, CancellationToken cancel) =>
            throw new KnownPreservedDerived("base", "preserved", "derived");

        public ValueTask ServerPrivateExceptionAsync(Current current, CancellationToken cancel) =>
            throw new ServerPrivateException("ServerPrivate");

        public async ValueTask RelayKnownPreservedAsBaseAsync(IRelayPrx? r, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                await p.KnownPreservedAsBaseAsync(cancel: cancel);
            }
            catch (RemoteException ex)
            {
                TestHelper.Assert(ex.ConvertToUnhandled);
                ex.ConvertToUnhandled = false;
                throw;
            }
            TestHelper.Assert(false);
        }

        public async ValueTask RelayKnownPreservedAsKnownPreservedAsync(IRelayPrx? r, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                await p.KnownPreservedAsKnownPreservedAsync(cancel: cancel);
            }
            catch (RemoteException ex)
            {
                TestHelper.Assert(ex.ConvertToUnhandled);
                ex.ConvertToUnhandled = false;
                throw;
            }
            TestHelper.Assert(false);
        }

        public ValueTask UnknownPreservedAsBaseAsync(Current current, CancellationToken cancel)
        {
            var p = new SPreservedClass("bc", "spc");
            throw new SPreserved2("base", "preserved", "derived", p, p);
        }

        public ValueTask UnknownPreservedAsKnownPreservedAsync(Current current, CancellationToken cancel)
        {
            var p = new SPreservedClass("bc", "spc");
            throw new SPreserved2("base", "preserved", "derived", p, p);
        }

        public async ValueTask RelayUnknownPreservedAsBaseAsync(IRelayPrx? r, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                await p.UnknownPreservedAsBaseAsync(cancel: cancel);
            }
            catch (RemoteException ex)
            {
                TestHelper.Assert(ex.ConvertToUnhandled);
                ex.ConvertToUnhandled = false;
                throw;
            }
            TestHelper.Assert(false);
        }

        public async ValueTask RelayUnknownPreservedAsKnownPreservedAsync(IRelayPrx? r, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                await p.UnknownPreservedAsKnownPreservedAsync(cancel: cancel);
            }
            catch (RemoteException ex)
            {
                TestHelper.Assert(ex.ConvertToUnhandled);
                ex.ConvertToUnhandled = false;
                throw;
            }
            TestHelper.Assert(false);
        }

        public async ValueTask RelayClientPrivateExceptionAsync(IRelayPrx? r, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                await p.ClientPrivateExceptionAsync(cancel: cancel);
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
