//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

#include <Ice/Context.ice>

// 'Test.ice' and 'TestAMD.ice' need to generate code into separate namespaces, but with identical type-ids. So we use
// the 'cs:namespace' metadata here to place the AMD code into a separate AMD namespace, instead of another module;
// Which would result in differing type-ids.
[cs:namespace:ZeroC.Ice.Test.AMD]
module Proxy
{

[amd] interface MyClass
{
    void shutdown();

    Ice::Context getContext();
}

[amd] interface MyDerivedClass : MyClass
{
    Object* echo(Object* obj);
}

}
