// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;

namespace ZeroC.Ice.Test.NamespaceMD
{
    public class Initial : IInitial
    {
        public NoNamespace.C1 GetNoNamespaceC2AsC1(Current current, CancellationToken cancel) =>
            new NoNamespace.C2();

        public NoNamespace.C2 GetNoNamespaceC2AsC2(Current current, CancellationToken cancel) =>
            new NoNamespace.C2();

        public WithNamespace.C1 GetWithNamespaceC2AsC1(Current current, CancellationToken cancel) =>
            new WithNamespace.C2();

        public WithNamespace.C2 GetWithNamespaceC2AsC2(Current current, CancellationToken cancel) =>
            new WithNamespace.C2();

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();

        public void ThrowNoNamespaceE2AsE1(Current current, CancellationToken cancel) => throw new NoNamespace.E2();

        public void ThrowNoNamespaceE2AsE2(Current current, CancellationToken cancel) => throw new NoNamespace.E2();

        public void ThrowNoNamespaceNotify(Current current, CancellationToken cancel) =>
            throw new NoNamespace.@notify();

        public void ThrowWithNamespaceE2AsE1(Current current, CancellationToken cancel) =>
            throw new WithNamespace.E2();

        public void ThrowWithNamespaceE2AsE2(Current current, CancellationToken cancel) =>
            throw new WithNamespace.E2();
    }
}
