// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_STREAM_H
#define ICE_STREAM_H

#include <Ice/InstanceF.h>
#include <Ice/Buffer.h>

namespace __Ice
{

class ICE_API Stream : public Buffer
{
public:

    Stream(const Instance&);

    Instance instance() const;

    void bigendian(bool);
    bool bigendian() const;

    void swap(Stream&);

    void write(Ice::Byte value) { b.push_back(value); }
    void read(Ice::Byte&);

    void write(bool value) { b.push_back(static_cast<Ice::Byte>(value)); }
    void read(bool&);

    void write(Ice::Short);
    void read(Ice::Short&);

    void write(Ice::Int);
    void read(Ice::Int&);

    void write(Ice::Long);
    void read(Ice::Long&);

    void write(Ice::Float);
    void read(Ice::Float&);

    void write(Ice::Double);
    void read(Ice::Double&);

    void write(const std::string&);
    void write(const char*); // Optimized version for const char*
    void read(std::string&);

private:

    Stream(const Stream&);
    void operator=(const Stream&);

    Instance instance_;
    bool bigendian_;
};

}

#endif
