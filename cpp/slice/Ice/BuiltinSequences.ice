// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

/** A sequence of objects. **/
sequence<Object> ObjectSeq;

/** A sequence of object proxies. **/
sequence<Object*> ObjectProxySeq;

/** A sequence of local objects. **/
local sequence<Object*> LocalObjectSeq;

};

#endif
