// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef ICE_BUILTIN_SEQUENCES_ICE
#define ICE_BUILTIN_SEQUENCES_ICE

module Ice
{

/** A sequence of bools. **/
["cs:array"] sequence<bool> BoolSeq;

/** A sequence of bytes. **/
["cs:array"] sequence<byte> ByteSeq;

/** A sequence of shorts. **/
["cs:array"] sequence<short> ShortSeq;

/** A sequence of ints. **/
["cs:array"] sequence<int> IntSeq;

/** A sequence of longs. **/
["cs:array"] sequence<long> LongSeq;

/** A sequence of floats. **/
["cs:array"] sequence<float> FloatSeq;

/** A sequence of doubles. **/
["cs:array"] sequence<double> DoubleSeq;

/** A sequence of strings. **/
["cs:array"] sequence<string> StringSeq;

/** A sequence of objects. **/
sequence<Object> ObjectSeq;

/** A sequence of object proxies. **/
sequence<Object*> ObjectProxySeq;

};

#endif
