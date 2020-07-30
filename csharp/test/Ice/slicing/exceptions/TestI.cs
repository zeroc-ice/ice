//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Slicing.Exceptions
{
    public sealed class TestIntf : ITestIntf
    {
        public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();

        public void BaseAsBase(Current current) => throw new Base("Base.b");

        public void UnknownDerivedAsBase(Current current) =>
            throw new UnknownDerived("UnknownDerived.b", "UnknownDerived.ud");

        public void KnownDerivedAsBase(Current current) => throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");

        public void KnownDerivedAsKnownDerived(Current current) =>
            throw new KnownDerived("KnownDerived.b", "KnownDerived.kd");

        public void UnknownIntermediateAsBase(Current current) =>
            throw new UnknownIntermediate("UnknownIntermediate.b", "UnknownIntermediate.ui");

        public void KnownIntermediateAsBase(Current current) =>
            throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");

        public void KnownMostDerivedAsBase(Current current) =>
            throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

        public void KnownIntermediateAsKnownIntermediate(Current current) =>
            throw new KnownIntermediate("KnownIntermediate.b", "KnownIntermediate.ki");

        public void KnownMostDerivedAsKnownIntermediate(Current current) =>
            throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

        public void KnownMostDerivedAsKnownMostDerived(Current current) =>
            throw new KnownMostDerived("KnownMostDerived.b", "KnownMostDerived.ki", "KnownMostDerived.kmd");

        public void UnknownMostDerived1AsBase(Current current) =>
            throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");

        public void UnknownMostDerived1AsKnownIntermediate(Current current) =>
            throw new UnknownMostDerived1("UnknownMostDerived1.b", "UnknownMostDerived1.ki", "UnknownMostDerived1.umd1");

        public void UnknownMostDerived2AsBase(Current current) =>
            throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");

        public void UnknownMostDerived2AsBaseCompact(Current current) =>
            throw new UnknownMostDerived2("UnknownMostDerived2.b", "UnknownMostDerived2.ui", "UnknownMostDerived2.umd2");

        public void KnownPreservedAsBase(Current current) =>
            throw new KnownPreservedDerived("base", "preserved", "derived");

        public void KnownPreservedAsKnownPreserved(Current current) =>
            throw new KnownPreservedDerived("base", "preserved", "derived");

        public void ServerPrivateException(Current current) => throw new ServerPrivateException("ServerPrivate");

        public void RelayKnownPreservedAsBase(IRelayPrx? r, Current current)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                p.KnownPreservedAsBase();
            }
            catch (RemoteException ex)
            {
                TestHelper.Assert(ex.ConvertToUnhandled);
                ex.ConvertToUnhandled = false;
                throw;
            }
            TestHelper.Assert(false);
        }

        public void RelayKnownPreservedAsKnownPreserved(IRelayPrx? r, Current current)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                p.KnownPreservedAsKnownPreserved();
            }
            catch (RemoteException ex)
            {
                TestHelper.Assert(ex.ConvertToUnhandled);
                ex.ConvertToUnhandled = false;
                throw;
            }
            TestHelper.Assert(false);
        }

        public void RelayClientPrivateException(IRelayPrx? r, Current current)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                p.ClientPrivateException();
            }
            catch (RemoteException ex)
            {
                TestHelper.Assert(ex.ConvertToUnhandled);
                ex.ConvertToUnhandled = false;
                throw;
            }
            TestHelper.Assert(false);
        }

        public void UnknownPreservedAsBase(Current current)
        {
            var p = new SPreservedClass("bc", "spc");
            throw new SPreserved2("base", "preserved", "derived", p, p);
        }

        public void UnknownPreservedAsKnownPreserved(Current current)
        {
            var p = new SPreservedClass("bc", "spc");
            throw new SPreserved2("base", "preserved", "derived", p, p);
        }

        public void RelayUnknownPreservedAsBase(IRelayPrx? r, Current current)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                p.UnknownPreservedAsBase();
            }
            catch (RemoteException ex)
            {
                TestHelper.Assert(ex.ConvertToUnhandled);
                ex.ConvertToUnhandled = false;
                throw;
            }
            TestHelper.Assert(false);
        }

        public void RelayUnknownPreservedAsKnownPreserved(IRelayPrx? r, Current current)
        {
            TestHelper.Assert(r != null);
            IRelayPrx p = r.Clone(fixedConnection: current.Connection);
            try
            {
                p.UnknownPreservedAsKnownPreserved();
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
