// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:no-default-include"]]
[["cpp:include:Ice/Config.h"]]
[["cpp:include:Ice/ObjectF.h"]]
[["cpp:include:Ice/ProxyF.h"]]
[["cpp:include:Ice/ValueF.h"]]
[["cpp:include:cstdint"]]
[["cpp:include:optional"]]
[["cpp:include:string"]]
[["cpp:include:vector"]]

[["js:module:@zeroc/ice"]]

["java:identifier:com.zeroc.Ice"]
module Ice
{
    /// A sequence of bools.
    sequence<bool> BoolSeq;

    /// A sequence of bytes.
    sequence<byte> ByteSeq;

    /// A sequence of shorts.
    sequence<short> ShortSeq;

    /// A sequence of ints.
    sequence<int> IntSeq;

    /// A sequence of longs.
    sequence<long> LongSeq;

    /// A sequence of floats.
    sequence<float> FloatSeq;

    /// A sequence of doubles.
    sequence<double> DoubleSeq;

    /// A sequence of strings.
    sequence<string> StringSeq;

    /// A sequence of class instances.
    ["deprecated:Define your own sequence instead and avoid using the term `object` to designate a class instance."]
    sequence<Value> ObjectSeq;

    /// A sequence of object proxies.
    sequence<Object*> ObjectProxySeq;
}
