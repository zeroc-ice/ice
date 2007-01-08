// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_THREAD_NOTIFICATION_H
#define ICEPY_THREAD_NOTIFICATION_H

#include <Config.h>
#include <Util.h>
#include <Ice/Initialize.h>

namespace IcePy
{

//
// ThreadNotificationWrapper delegates to a Python implementation.
//
class ThreadNotificationWrapper : public Ice::ThreadNotification
{
public:

    ThreadNotificationWrapper(PyObject*);

    virtual void start();
    virtual void stop();

    PyObject* getObject();

private:

    PyObjectHandle _threadNotification;
};
typedef IceUtil::Handle<ThreadNotificationWrapper> ThreadNotificationWrapperPtr;

}

#endif
