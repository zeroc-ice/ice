// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_LOGGER_H
#define ICEPY_LOGGER_H

#include <Config.h>
#include <Util.h>
#include <Ice/Logger.h>

namespace IcePy
{

//
// LoggerWrapper delegates to a Python implementation.
//
class LoggerWrapper : public Ice::Logger
{
public:

    LoggerWrapper(PyObject*);

    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);
    virtual std::string getPrefix();
    virtual Ice::LoggerPtr cloneWithPrefix(const std::string&);
    PyObject* getObject();

private:

    PyObjectHandle _logger;
};
typedef IceUtil::Handle<LoggerWrapper> LoggerWrapperPtr;

bool initLogger(PyObject*);

void cleanupLogger();

//
// Create a Python object that delegates to a C++ implementation.
//
PyObject* createLogger(const Ice::LoggerPtr&);

}

extern "C" PyObject* IcePy_getProcessLogger(PyObject*);
extern "C" PyObject* IcePy_setProcessLogger(PyObject*, PyObject*);

#endif
