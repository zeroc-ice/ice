// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BUILTIN_SEQUENCES_ICE
#define ICE_BUILTIN_SEQUENCES_ICE

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

/** A sequence of object proxies. **/
sequence<Object*> ObjectProxySeq;

};

#endif
