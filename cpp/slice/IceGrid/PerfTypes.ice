// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_PERF_TYPES_ICE
#define ICE_GRID_PERF_TYPES_ICE

#include <Ice/BuiltinSequences.ice>

module IcePerf
{

/**
 *
 * Base class for all performance monitoring data
 *
 **/
class PerfData
{
};

sequence<PerfData> PerfDataSeq;

/**
 *
 * Returned when the key requested does not map to anything on this platform
 *
 **/
class Unknown extends PerfData
{
};

/**
 *
 * "Boxed" string
 *
 **/
class BoxedString extends PerfData
{
    string value;
};

/**
 *
 * "Boxed" int
 *
 **/
class BoxedInt extends PerfData
{
    int value;
};

/**
 *
 * "Boxed" long
 *
 **/
class BoxedLong extends PerfData
{
    long value;
};

/**
 *
 * "Boxed" float
 *
 **/
class BoxedFloat extends PerfData
{
    float value;
};


/**
 *
 * "Boxed" double
 *
 **/
class BoxedDouble extends PerfData
{
    double value;
};

/**
 *
 * LoadAvg: load in the past 1, 5 and 15 minutes
 *
 **/
class LoadAvg extends PerfData
{
    Ice::FloatSeq value;
};


};

#endif
