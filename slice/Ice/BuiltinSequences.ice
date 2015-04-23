// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc", "js:ice-build"]]

["objc:prefix:ICE"]
module Ice
{

/** A sequence of bools. **/
sequence<bool> BoolSeq;

/** A sequence of bytes. **/
sequence<byte> ByteSeq;

/** A sequence of shorts. **/
sequence<short> ShortSeq;

/** A sequence of ints. **/
sequence<int> IntSeq;

/** A sequence of longs. **/
sequence<long> LongSeq;

/** A sequence of floats. **/
sequence<float> FloatSeq;

/** A sequence of doubles. **/
sequence<double> DoubleSeq;

/** A sequence of strings. **/
sequence<string> StringSeq;

/** A sequence of objects. **/
sequence<Object> ObjectSeq;

/** A sequence of object proxies. **/
sequence<Object*> ObjectProxySeq;

};

