// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_MARSHAL_H
#define ICEPY_MARSHAL_H

#include <Types.h>
#include <Ice/Stream.h>
#include <Ice/CommunicatorF.h>

namespace IcePy
{

class Marshaler;
typedef IceUtil::Handle<Marshaler> MarshalerPtr;

typedef std::map<PyObject*, Ice::ObjectPtr> ObjectMap;

class Marshaler : public IceUtil::Shared
{
public:

    virtual ~Marshaler();

    static MarshalerPtr createMarshaler(const TypeInfoPtr&);
    static MarshalerPtr createExceptionMarshaler(const ExceptionInfoPtr&);

    virtual bool marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*) = 0;
    virtual PyObject* unmarshal(const Ice::CommunicatorPtr&, const Ice::InputStreamPtr&) = 0;

    virtual void destroy() = 0;

protected:

    Marshaler();
};

//
// The Ice encoding for objects involves delayed unmarshaling, in which it may not be
// possible to create the object at the time it is first unmarshaled. Therefore, we
// have to treat object unmarshaling as a special case. ObjectReceiver is the base
// class that defines an interface for receiving an object instance after it has been
// unmarshaled. Various subclasses are necessary to address the contexts in which
// objects are unmarshaled (e.g., sequences, dictionaries, data members, etc.).
//
class ObjectReceiver : public IceUtil::Shared
{
public:

    virtual ~ObjectReceiver();

    virtual void setObject(PyObject*) = 0;
};
typedef IceUtil::Handle<ObjectReceiver> ObjectReceiverPtr;

//
// TupleReceiver is an ObjectReceiver that places the received object into the given
// index in a tuple.
//
class TupleReceiver : public ObjectReceiver
{
public:

    TupleReceiver(PyObject*, int);

    virtual void setObject(PyObject*);

private:

    PyObjectHandle _tuple;
    int _index;
};

//
// ObjectMarshaler supplies a special unmarshalObject member function for unmarshaling
// an object. This function accepts an ObjectReceiver that is invoked when the instance
// is eventually available.
//
class ObjectMarshaler : public Marshaler
{
public:

    ObjectMarshaler(const ClassInfoPtr&);
    ~ObjectMarshaler();

    virtual bool marshal(PyObject*, const Ice::OutputStreamPtr&, ObjectMap*);

    //
    // Not implemented.
    //
    virtual PyObject* unmarshal(const Ice::CommunicatorPtr&, const Ice::InputStreamPtr&);

    //
    // Object unmarshaling must be delayed due to the Ice encoding format.
    //
    void unmarshalObject(const Ice::CommunicatorPtr&, const Ice::InputStreamPtr&, const ObjectReceiverPtr&);

    virtual void destroy();

private:

    ClassInfoPtr _info; // The formal type.
};
typedef IceUtil::Handle<ObjectMarshaler> ObjectMarshalerPtr;

//
// This class is raised as an exception when object marshaling needs to be aborted.
//
class AbortMarshaling
{
};

class ObjectWriter : public Ice::ObjectWriter
{
public:

    ObjectWriter(const ClassInfoPtr&, PyObject*, ObjectMap*);
    ~ObjectWriter();

    virtual void write(const Ice::OutputStreamPtr&) const;

private:

    ClassInfoPtr _info;
    PyObject* _object;
    ObjectMap* _map;
};

class ObjectReader : public Ice::ObjectReader
{
public:

    ObjectReader(PyObject*, const ClassInfoPtr&, const Ice::CommunicatorPtr&);
    ~ObjectReader();

    virtual void read(const Ice::InputStreamPtr&, bool);

    PyObject* getObject() const; // Borrowed reference.

private:

    PyObject* _object;
    ClassInfoPtr _info;
    Ice::CommunicatorPtr _communicator;
};
typedef IceUtil::Handle<ObjectReader> ObjectReaderPtr;

}

#endif
