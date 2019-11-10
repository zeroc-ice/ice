//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace namespacemd
    {
        public class InitialI : Test.InitialDisp_
        {
            public override NoNamespace.C1 getNoNamespaceC2AsC1(Current current)
            {
                return new NoNamespace.C2();
            }

            public override NoNamespace.C2 getNoNamespaceC2AsC2(Current current)
            {
                return new NoNamespace.C2();
            }

            public override WithNamespace.C1 getWithNamespaceC2AsC1(Current current)
            {
                return new WithNamespace.C2();
            }

            public override WithNamespace.C2 getWithNamespaceC2AsC2(Current current)
            {
                return new WithNamespace.C2();
            }

            public override void shutdown(Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            public override void throwNoNamespaceE2AsE1(Current current)
            {
                throw new NoNamespace.E2();
            }

            public override void throwNoNamespaceE2AsE2(Current current)
            {
                throw new NoNamespace.E2();
            }

            public override void throwNoNamespaceNotify(Current current)
            {
                throw new NoNamespace.@notify();
            }

            public override void throwWithNamespaceE2AsE1(Current current)
            {
                throw new WithNamespace.E2();
            }

            public override void throwWithNamespaceE2AsE2(Current current)
            {
                throw new WithNamespace.E2();
            }
        }
    }
}
