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

    void write(Ice::Byte v) { b.push_back(v); }
    void write(const std::vector<Ice::Byte>&);
    void read(Ice::Byte&);
    void read(std::vector<Ice::Byte>&);

    void write(bool v) { b.push_back(static_cast<Ice::Byte>(v)); }
    void write(const std::vector<bool>&);
    void read(bool&);
    void read(std::vector<bool>&);

    void write(Ice::Short);
    void write(const std::vector<Ice::Short>&);
    void read(Ice::Short&);
    void read(std::vector<Ice::Short>&);

    void write(Ice::Int);
    void write(const std::vector<Ice::Int>&);
    void read(Ice::Int&);
    void read(std::vector<Ice::Int>&);

    void write(Ice::Long);
    void write(const std::vector<Ice::Long>&);
    void read(Ice::Long&);
    void read(std::vector<Ice::Long>&);

    void write(Ice::Float);
    void read(Ice::Float&);
    void write(const std::vector<Ice::Float>&);
    void read(std::vector<Ice::Float>&);

    void write(Ice::Double);
    void write(const std::vector<Ice::Double>&);
    void read(Ice::Double&);
    void read(std::vector<Ice::Double>&);

    void write(const std::string&);
    void write(const char*); // Optimization
    void read(std::string&);

    void write(const std::wstring&);
    void read(std::wstring&);

private:

    Stream(const Stream&);
    void operator=(const Stream&);

    Instance instance_;
    bool bigendian_;
};

template<typename T>
inline void streamWrite(Stream* s, const T& v)
{
    s -> write(Ice::Int(v.size()));
    typename T::const_iterator p;
    for(p = v.begin(); p != v.end(); ++p)
	streamWrite(s, *p);
}

template<typename T>
inline void streamRead(Stream* s, T& v)
{
    Ice::Int sz;
    s -> read(sz);
    v.resize(sz);
    typename T::iterator p;
    for(p = v.begin(); p != v.end(); ++p)
	streamRead(s, *p);
}

template<>
inline void streamWrite<Ice::Byte>(Stream* s, const Ice::Byte& v)
{
    s -> write(v);
}

template<>
inline void streamWrite<std::vector<Ice::Byte> >(
    Stream* s, const std::vector<Ice::Byte>& v)
{
    s -> write(v);
}

template<>
inline void streamRead<Ice::Byte>(Stream* s, Ice::Byte& v)
{
    s -> read(v);
}

template<>
inline void streamRead<std::vector<Ice::Byte> >(
    Stream* s, std::vector<Ice::Byte>& v)
{
    s -> read(v);
}

template<>
inline void streamWrite<bool>(Stream* s, const bool& v)
{
    s -> write(v);
}

template<>
inline void streamWrite<std::vector<bool> >(
    Stream* s, const std::vector<bool>& v)
{
    s -> write(v);
}

template<>
inline void streamRead<bool>(Stream* s, bool& v)
{
    s -> read(v);
}

template<>
inline void streamRead<std::vector<bool> >(
    Stream* s, std::vector<bool>& v)
{
    s -> read(v);
}

template<>
inline void streamWrite<Ice::Short>(Stream* s, const Ice::Short& v)
{
    s -> write(v);
}

template<>
inline void streamWrite<std::vector<Ice::Short> >(
    Stream* s, const std::vector<Ice::Short>& v)
{
    s -> write(v);
}

template<>
inline void streamRead<Ice::Short>(Stream* s, Ice::Short& v)
{
    s -> read(v);
}

template<>
inline void streamRead<std::vector<Ice::Short> >(
    Stream* s, std::vector<Ice::Short>& v)
{
    s -> read(v);
}

template<>
inline void streamWrite<Ice::Int>(Stream* s, const Ice::Int& v)
{
    s -> write(v);
}

template<>
inline void streamWrite<std::vector<Ice::Int> >(
    Stream* s, const std::vector<Ice::Int>& v)
{
    s -> write(v);
}

template<>
inline void streamRead<Ice::Int>(Stream* s, Ice::Int& v)
{
    s -> read(v);
}

template<>
inline void streamRead<std::vector<Ice::Int> >(
    Stream* s, std::vector<Ice::Int>& v)
{
    s -> read(v);
}

template<>
inline void streamWrite<Ice::Long>(Stream* s, const Ice::Long& v)
{
    s -> write(v);
}

template<>
inline void streamWrite<std::vector<Ice::Long> >(
    Stream* s, const std::vector<Ice::Long>& v)
{
    s -> write(v);
}

template<>
inline void streamRead<Ice::Long>(Stream* s, Ice::Long& v)
{
    s -> read(v);
}

template<>
inline void streamRead<std::vector<Ice::Long> >(
    Stream* s, std::vector<Ice::Long>& v)
{
    s -> read(v);
}

template<>
inline void streamWrite<Ice::Float>(Stream* s, const Ice::Float& v)
{
    s -> write(v);
}

template<>
inline void streamWrite<std::vector<Ice::Float> >(
    Stream* s, const std::vector<Ice::Float>& v)
{
    s -> write(v);
}

template<>
inline void streamRead<Ice::Float>(Stream* s, Ice::Float& v)
{
    s -> read(v);
}

template<>
inline void streamRead<std::vector<Ice::Float> >(
    Stream* s, std::vector<Ice::Float>& v)
{
    s -> read(v);
}

template<>
inline void streamWrite<Ice::Double>(Stream* s, const Ice::Double& v)
{
    s -> write(v);
}

template<>
inline void streamWrite<std::vector<Ice::Double> >(
    Stream* s, const std::vector<Ice::Double>& v)
{
    s -> write(v);
}

template<>
inline void streamRead<Ice::Double>(Stream* s, Ice::Double& v)
{
    s -> read(v);
}

template<>
inline void streamRead<std::vector<Ice::Double> >(
    Stream* s, std::vector<Ice::Double>& v)
{
    s -> read(v);
}

template<>
inline void streamWrite<std::string>(Stream* s, const std::string& v)
{
    s -> write(v);
}

template<>
inline void streamRead<std::string>(Stream* s, std::string& v)
{
    s -> read(v);
}

template<>
inline void streamWrite<std::wstring>(Stream* s, const std::wstring& v)
{
    s -> write(v);
}

template<>
inline void streamRead<std::wstring>(Stream* s, std::wstring& v)
{
    s -> read(v);
}

}

#endif
