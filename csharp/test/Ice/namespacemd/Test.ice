// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Namespace.ice>
#include <NoNamespace.ice>

["cs:namespace:Ice.namespacemd"]
module Test
{

interface Initial
{
    NoNamespace::C1 getNoNamespaceC2AsC1();
    NoNamespace::C2 getNoNamespaceC2AsC2();
    void throwNoNamespaceE2AsE1() throws NoNamespace::E1;
    void throwNoNamespaceE2AsE2() throws NoNamespace::E2;
    void throwNoNamespaceNotify() throws NoNamespace::notify;

    WithNamespace::C1 getWithNamespaceC2AsC1();
    WithNamespace::C2 getWithNamespaceC2AsC2();
    void throwWithNamespaceE2AsE1() throws WithNamespace::E1;
    void throwWithNamespaceE2AsE2() throws WithNamespace::E2;

    void shutdown();
}

}
