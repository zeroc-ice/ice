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
    void write(const std::vector<Ice::Byte>&);
    void read(std::vector<Ice::Byte>&);

    void write(bool value) { b.push_back(static_cast<Ice::Byte>(value)); }
    void read(bool&);
    void write(const std::vector<bool>&);
    void read(std::vector<bool>&);

    void write(Ice::Short);
    void read(Ice::Short&);
    void write(const std::vector<Ice::Short>&);
    void read(std::vector<Ice::Short>&);

    void write(Ice::Int);
    void read(Ice::Int&);
    void write(const std::vector<Ice::Int>&);
    void read(std::vector<Ice::Int>&);

    void write(Ice::Long);
    void read(Ice::Long&);
    void write(const std::vector<Ice::Long>&);
    void read(std::vector<Ice::Long>&);

    void write(Ice::Float);
    void read(Ice::Float&);
    void write(const std::vector<Ice::Float>&);
    void read(std::vector<Ice::Float>&);

    void write(Ice::Double);
    void read(Ice::Double&);
    void write(const std::vector<Ice::Double>&);
    void read(std::vector<Ice::Double>&);

    void write(const std::string&);
    void write(const char*); // Optimized version for const char*
    void read(std::string&);
    void write(const std::vector<std::string>&);
    void read(std::vector<std::string>&);

private:

    Stream(const Stream&);
    void operator=(const Stream&);

    Instance instance_;
    bool bigendian_;
};

}

#endif
