// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
