//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[3.7]]
[[suppress-warning(reserved-identifier)]]

#include <Namespace.ice>
#include <NoNamespace.ice>

module ZeroC::Ice::Test::NamespaceMD
{

interface Initial
{
    NoNamespace::C1 getNoNamespaceC2AsC1();
    NoNamespace::C2 getNoNamespaceC2AsC2();
    void throwNoNamespaceE2AsE1();
    void throwNoNamespaceE2AsE2();
    void throwNoNamespaceNotify();

    WithNamespace::C1 getWithNamespaceC2AsC1();
    WithNamespace::C2 getWithNamespaceC2AsC2();
    void throwWithNamespaceE2AsE1();
    void throwWithNamespaceE2AsE2();

    void shutdown();
}

}
