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

#ifndef EVENT_H
#define EVENT_H

#include <Ice/Current.h> // For Ice::Context

namespace IceStorm
{

//
// Note that at present this requires to copy the event which isn't
// nice. If this indeed becomes a bottleneck then either the event can
// carry a reference to the blob, context & op (while event itself
// isn't copied), or the op, blob & context can be passed along as
// arguments (or do copy on write, or some such trick).
//
class Event : public IceUtil::Shared
{
public:

    bool forwarded;
    int cost;
    std::string op;
    Ice::OperationMode mode;
    std::vector<Ice::Byte> data;
    Ice::Context context;
};

typedef IceUtil::Handle<Event> EventPtr;

} // End namespace IceStorm

#endif
