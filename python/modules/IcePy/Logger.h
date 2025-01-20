// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_LOGGER_H
#define ICEPY_LOGGER_H

#include "Config.h"
#include "Ice/Logger.h"
#include "Util.h"

#include <memory>

namespace IcePy
{
    //
    // LoggerWrapper delegates to a Python implementation.
    //
    class LoggerWrapper final : public Ice::Logger
    {
    public:
        LoggerWrapper(PyObject*);

        void print(const std::string&) final;
        void trace(const std::string&, const std::string&) final;
        void warning(const std::string&) final;
        void error(const std::string&) final;
        std::string getPrefix() final;
        Ice::LoggerPtr cloneWithPrefix(std::string) final;
        PyObject* getObject();

    private:
        PyObjectHandle _logger;
    };
    using LoggerWrapperPtr = std::shared_ptr<LoggerWrapper>;

    bool initLogger(PyObject*);

    void cleanupLogger();

    //
    // Create a Python object that delegates to a C++ implementation.
    //
    PyObject* createLogger(const Ice::LoggerPtr&);
}

extern "C" PyObject* IcePy_getProcessLogger(PyObject*, PyObject*);
extern "C" PyObject* IcePy_setProcessLogger(PyObject*, PyObject*);

#endif
