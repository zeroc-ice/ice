// Copyright (c) ZeroC, Inc.

#include "Current.h"
#include "Connection.h"
#include "Ice/ObjectAdapter.h"
#include "ObjectAdapter.h"
#include "Util.h"

using namespace std;
using namespace IcePy;

PyObject*
IcePy::createCurrent(const Ice::Current& current)
{
    PyObject* currentType{lookupType("Ice.Current")};

    PyObjectHandle args{PyTuple_New(9)};

    PyObject* adapter{wrapObjectAdapter(current.adapter)};
    if (!adapter)
    {
        return nullptr;
    }
    PyTuple_SetItem(args.get(), 0, adapter);

    if (current.con)
    {
        PyObject* connection{createConnection(current.con, current.adapter->getCommunicator())};
        if (!connection)
        {
            return nullptr;
        }
        PyTuple_SetItem(args.get(), 1, connection);
    }
    else
    {
        PyTuple_SetItem(args.get(), 1, Py_None);
    }

    PyObject* id{createIdentity(current.id)};
    if (!id)
    {
        return nullptr;
    }
    PyTuple_SetItem(args.get(), 2, id);

    PyObject* facet{createString(current.facet)};
    if (!facet)
    {
        return nullptr;
    }
    PyTuple_SetItem(args.get(), 3, facet);

    PyObject* operation{createString(current.operation)};
    if (!operation)
    {
        return nullptr;
    }
    PyTuple_SetItem(args.get(), 4, operation);

    PyObject* operationModeType = lookupType("Ice.OperationMode");
    assert(operationModeType);
    const char* enumerator{nullptr};
    switch (current.mode)
    {
        case Ice::OperationMode::Normal:
            enumerator = "Normal";
            break;
#include "Ice/PushDisableDeprecatedWarnings.h"
        case Ice::OperationMode::Nonmutating:
#include "Ice/PopDisableWarnings.h"
            enumerator = "Nonmutating";
            break;
        case Ice::OperationMode::Idempotent:
            enumerator = "Idempotent";
            break;
    }
    PyTuple_SetItem(args.get(), 5, getAttr(operationModeType, enumerator, false));

    PyObjectHandle ctx{PyDict_New()};
    if (!contextToDictionary(current.ctx, ctx.get()))
    {
        return nullptr;
    }
    PyTuple_SetItem(args.get(), 6, ctx.release());

    PyObject* requestId{PyLong_FromLong(current.requestId)};
    if (!requestId)
    {
        return nullptr;
    }
    PyTuple_SetItem(args.get(), 7, requestId);

    PyObject* encoding{IcePy::createEncodingVersion(current.encoding)};
    if (!encoding)
    {
        return nullptr;
    }
    PyTuple_SetItem(args.get(), 8, encoding);

    return PyObject_CallObject(currentType, args.get());
}
