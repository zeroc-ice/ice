// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_STREAM_H
#define ICE_STREAM_H

#include <Ice/InstanceF.h>
#include <Ice/ObjectF.h>
#include <Ice/Buffer.h>
#include <stack>
#include <set>

namespace __Ice
{

class ICE_API Stream : public Buffer
{
public:

    Stream(const Instance_ptr&);

    Instance_ptr instance() const;

    void swap(Stream&);

    void resize(int);
    void reserve(int);

    void pushBigendian(bool);
    void popBigendian();
    bool bigendian() const;

    void startWriteEncaps();
    void endWriteEncaps();
    void startReadEncaps();
    void endReadEncaps();
    void skipEncaps();

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
    void write(const std::vector<std::string>&);
    void read(std::string&);
    void read(std::vector<std::string>&);

    void write(const std::wstring&);
    void write(const std::vector<std::wstring>&);
    void read(std::wstring&);
    void read(std::vector<std::wstring>&);

    void write(const ::Ice::Object_ptr&);
    void read(::Ice::Object_ptr&, const std::string&);

private:

    Instance_ptr instance_;
    bool bigendian_;
    std::stack<bool> bigendianStack_;
    std::stack<int> encapsStartStack_;

    class CmpPosPos
    {
    public:
	CmpPosPos(const Container&);
	bool operator()(int, int) const;
    private:
	const Container& cont_;
    };

    class CmpPosString
    {
    public:
	CmpPosString(const Container&);
	bool operator()(int, const std::string&) const;
    private:
	const Container& cont_;
    };

    std::multiset<int, CmpPosPos> stringSet_;
};

}

#endif
