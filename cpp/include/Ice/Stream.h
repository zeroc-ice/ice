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

private:

    Stream(const Stream&);
    void operator=(const Stream&);

    Instance instance_;
    bool bigendian_;
};

template<typename T>
inline void
streamRead(Stream* s, T& value)
{
    s -> read(value);
}

template<typename T>
inline void
streamRead(Stream* s, std::vector<T>& value)
{
    ::Ice::Int sz;
    s -> read(sz);
    value.resize(sz);
    typename std::vector<T>::iterator p;
    for(p = value.begin(); p != value.end(); ++p)
	streamRead(s, *p);
}

inline void
streamRead(Stream* s, std::vector< ::Ice::Byte>& value)
{
    s -> read(value);
}

inline void
streamRead(Stream* s, std::vector<bool>& value)
{
    s -> read(value);
}

inline void
streamRead(Stream* s, std::vector< ::Ice::Short>& value)
{
    s -> read(value);
}

inline void
streamRead(Stream* s, std::vector< ::Ice::Int>& value)
{
    s -> read(value);
}

inline void
streamRead(Stream* s, std::vector< ::Ice::Long>& value)
{
    s -> read(value);
}

inline void
streamRead(Stream* s, std::vector< ::Ice::Float>& value)
{
    s -> read(value);
}

inline void
streamRead(Stream* s, std::vector< ::Ice::Double>& value)
{
    s -> read(value);
}

template<typename T>
inline void
streamWrite(Stream* s, const T& value)
{
    s -> write(value);
}

template<typename T>
inline void
streamWrite(Stream* s, const std::vector<T>& value)
{
    s -> write(::Ice::Int(value.size()));
    typename std::vector<T>::const_iterator p;
    for(p = value.begin(); p != value.end(); ++p)
	streamWrite(s, *p);
}

inline void
streamWrite(Stream* s, const std::vector< ::Ice::Byte>& value)
{
    s -> write(value);
}

inline void
streamWrite(Stream* s, const std::vector<bool>& value)
{
    s -> write(value);
}

inline void
streamWrite(Stream* s, const std::vector< ::Ice::Short>& value)
{
    s -> write(value);
}

inline void
streamWrite(Stream* s, const std::vector< ::Ice::Int>& value)
{
    s -> write(value);
}

inline void
streamWrite(Stream* s, const std::vector< ::Ice::Long>& value)
{
    s -> write(value);
}

inline void
streamWrite(Stream* s, const std::vector< ::Ice::Float>& value)
{
    s -> write(value);
}

inline void
streamWrite(Stream* s, const std::vector< ::Ice::Double>& value)
{
    s -> write(value);
}

}

#endif
