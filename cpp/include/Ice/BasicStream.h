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
#include <Ice/ProxyF.h>
#include <Ice/Buffer.h>

namespace IceInternal
{

class ICE_API BasicStream : public Buffer
{
public:

    BasicStream(const InstancePtr&);
    ~BasicStream();

    InstancePtr instance() const;

    void swap(BasicStream&);

    void resize(int);
    void reserve(int);

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
    void write(const std::vector<Ice::Float>&);
    void read(Ice::Float&);
    void read(std::vector<Ice::Float>&);

    void write(Ice::Double);
    void write(const std::vector<Ice::Double>&);
    void read(Ice::Double&);
    void read(std::vector<Ice::Double>&);

    void write(const std::string&);
    void write(const char*);
    void write(const std::vector<std::string>&);
    void read(std::string&);
    void read(std::vector<std::string>&);

    void write(const std::wstring&);
    void write(const wchar_t*);
    void write(const std::vector<std::wstring>&);
    void read(std::wstring&);
    void read(std::vector<std::wstring>&);

    void write(const ::Ice::ObjectPrx&);
    void read(::Ice::ObjectPrx&);

    void write(const ::Ice::ObjectPtr&);
    void read(::Ice::ObjectPtr&, const std::string&);

private:

    InstancePtr _instance;

    struct Encaps
    {
	Container::size_type start;
	Ice::Byte encoding;
	std::vector<std::string> stringsRead;
	std::map<std::string, ::Ice::Int> stringsWritten;
	std::vector<std::wstring> wstringsRead;
	std::map<std::wstring, ::Ice::Int> wstringsWritten;
    };

    std::vector<Encaps> _encapsStack;
};

}

#endif
