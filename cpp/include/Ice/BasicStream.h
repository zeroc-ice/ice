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

//#define ICE_ACTIVE_STRING_INDIRECTION

namespace Ice
{

class UserException;

}

namespace IceInternal
{

class ICE_API BasicStream : public Buffer
{
public:

    BasicStream(const InstancePtr&);

    InstancePtr instance() const;

    void swap(BasicStream&);

    void resize(int);
    void reserve(int);

    void startWriteEncaps();
    void endWriteEncaps();
    void startReadEncaps();
    void endReadEncaps();
    void checkReadEncaps();
    Ice::Int getReadEncapsSize();
    void skipEncaps();

    void writeBlob(const std::vector<Ice::Byte>&);
    void readBlob(std::vector<Ice::Byte>&, Ice::Int);

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
    void write(const std::vector<std::string>&);
    void read(std::string&);
    void read(std::vector<std::string>&);

    void write(const std::wstring&);
    void write(const wchar_t*);
    void write(const std::vector<std::wstring>&);
    void read(std::wstring&);
    void read(std::vector<std::wstring>&);

    void write(const Ice::ObjectPrx&);
    void read(Ice::ObjectPrx&);

    void write(const Ice::ObjectPtr&);
    bool read(const std::string&, Ice::ObjectPtr&);
    void read(const Ice::ObjectPtr&);

    void write(const Ice::UserException&);
    ::Ice::Int throwException(const std::string*, const std::string*);

private:

    //
    // Optimization. The instance may not be deleted while a
    // stack-allocated Incoming still holds it.
    //
    // TODO: On WIN32 I get crashes with this optimization from time
    // to time. I don't know why yet.
#ifdef WIN32
    InstancePtr _instance;
#else
    const InstancePtr& _instance;
#endif

    struct Encaps
    {
	Container::size_type start;
	Ice::Byte encoding;
	std::vector<std::string> stringsRead;
#ifdef ICE_ACTIVE_STRING_INDIRECTION
	std::map<std::string, Ice::Int> stringsWritten;
#endif
	std::vector<std::wstring> wstringsRead;
#ifdef ICE_ACTIVE_STRING_INDIRECTION
	std::map<std::wstring, Ice::Int> wstringsWritten;
#endif
	std::vector<Ice::ObjectPtr> objectsRead;
	std::map<Ice::ObjectPtr, Ice::Int> objectsWritten;
    };

    std::vector<Encaps> _encapsStack;
};

}

#endif
