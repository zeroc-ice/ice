// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
