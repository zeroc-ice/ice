// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_OPERATION_H
#define ICEPY_OPERATION_H

#include <Config.h>
#include <Ice/Current.h>
#include <Ice/Object.h>

namespace IcePy
{

bool initOperation(PyObject*);

//
// Builtin operations.
//
PyObject* iceIsA(const Ice::ObjectPrx&, PyObject*);
PyObject* icePing(const Ice::ObjectPrx&, PyObject*);
PyObject* iceIds(const Ice::ObjectPrx&, PyObject*);
PyObject* iceId(const Ice::ObjectPrx&, PyObject*);

//
// Blobject invocations.
//
PyObject* iceInvoke(const Ice::ObjectPrx&, PyObject*);
PyObject* iceInvokeAsync(const Ice::ObjectPrx&, PyObject*);

//
// ServantWrapper handles dispatching to a Python servant.
//
class ServantWrapper : public Ice::BlobjectArrayAsync
{
public:

    ServantWrapper(PyObject*);
    ~ServantWrapper();

    PyObject* getObject();

protected:

    PyObject* _servant;
};
typedef IceUtil::Handle<ServantWrapper> ServantWrapperPtr;

ServantWrapperPtr createServantWrapper(PyObject*);

}

#endif
