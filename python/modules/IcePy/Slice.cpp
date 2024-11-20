//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Slice.h"
#include "Ice/ConsoleUtil.h"
#include "Ice/Options.h"
#include "Slice/Preprocessor.h"
#include "Slice/Util.h"
#include "Util.h"
#include "slice2py/PythonUtil.h"

//
// Python headers needed for PyEval_EvalCode.
//
#include <ceval.h>
#include <compile.h>

using namespace std;
using namespace IcePy;
using namespace Slice;
using namespace Slice::Python;
using namespace IceInternal;

extern "C" PyObject*
IcePy_loadSlice(PyObject* /*self*/, PyObject* args)
{
    char* cmd;
    PyObject* list = 0;
    if (!PyArg_ParseTuple(args, "s|O!", &cmd, &PyList_Type, &list))
    {
        return nullptr;
    }

    vector<string> argSeq;
    try
    {
        argSeq = IceInternal::Options::split(cmd);
    }
    catch (const IceInternal::BadOptException& ex)
    {
        PyErr_Format(PyExc_RuntimeError, "error in Slice options: %s", ex.what());
        return nullptr;
    }
    catch (const IceInternal::APIException& ex)
    {
        PyErr_Format(PyExc_RuntimeError, "error in Slice options: %s", ex.what());
        return nullptr;
    }

    if (list)
    {
        if (!listToStringSeq(list, argSeq))
        {
            return nullptr;
        }
    }

    IceInternal::Options opts;
    opts.addOpt("D", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("U", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("I", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("d", "debug");
    opts.addOpt("", "all");

    vector<string> files;
    try
    {
        argSeq.insert(argSeq.begin(), ""); // dummy argv[0]
        files = opts.parse(argSeq);
        if (files.empty())
        {
            PyErr_Format(PyExc_RuntimeError, "no Slice files specified in `%s'", cmd);
            return nullptr;
        }
    }
    catch (const IceInternal::BadOptException& ex)
    {
        PyErr_Format(PyExc_RuntimeError, "error in Slice options: %s", ex.what());
        return nullptr;
    }
    catch (const IceInternal::APIException& ex)
    {
        PyErr_Format(PyExc_RuntimeError, "error in Slice options: %s", ex.what());
        return nullptr;
    }

    vector<string> cppArgs;
    Ice::StringSeq includePaths;
    bool debug = false;
    bool all = false;
    if (opts.isSet("D"))
    {
        vector<string> optargs = opts.argVec("D");
        for (vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
        {
            cppArgs.push_back("-D" + *i);
        }
    }
    if (opts.isSet("U"))
    {
        vector<string> optargs = opts.argVec("U");
        for (vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
        {
            cppArgs.push_back("-U" + *i);
        }
    }
    if (opts.isSet("I"))
    {
        includePaths = opts.argVec("I");
        for (vector<string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
        {
            cppArgs.push_back("-I" + *i);
        }
    }
    debug = opts.isSet("d") || opts.isSet("debug");
    all = opts.isSet("all");

    for (const auto& file : files)
    {
        Slice::PreprocessorPtr icecpp = Slice::Preprocessor::create("icecpp", file, cppArgs);
        FILE* cppHandle = icecpp->preprocess(true, "-D__SLICE2PY__");

        if (cppHandle == 0)
        {
            PyErr_Format(PyExc_RuntimeError, "Slice preprocessing failed for `%s'", cmd);
            return nullptr;
        }

        UnitPtr u = Slice::Unit::createUnit(all);
        int parseStatus = u->parse(file, cppHandle, debug);

        if (!icecpp->close() || parseStatus == EXIT_FAILURE)
        {
            PyErr_Format(PyExc_RuntimeError, "Slice parsing failed for `%s'", cmd);
            u->destroy();
            return nullptr;
        }

        //
        // Generate the Python code into a string stream.
        //
        ostringstream codeStream;
        IceInternal::Output out(codeStream);
        out.setUseTab(false);

        //
        // Emit a Python magic comment to set the file encoding.
        // It must be the first or second line.
        //
        out << "# -*- coding: utf-8 -*-\n";
        generate(u, all, includePaths, out);
        u->destroy();

        string code = codeStream.str();

        //
        // We need to invoke Ice.updateModules() so that all of the types we've just generated
        // are made "public".
        //
        code += "\nIce.updateModules()\n";

        PyObjectHandle src{
            Py_CompileString(const_cast<char*>(code.c_str()), const_cast<char*>(file.c_str()), Py_file_input)};
        if (!src.get())
        {
            return nullptr;
        }

        PyObjectHandle globals{PyDict_New()};
        if (!globals.get())
        {
            return nullptr;
        }

        PyDict_SetItemString(globals.get(), "__builtins__", PyEval_GetBuiltins());
        PyObjectHandle val{PyEval_EvalCode(src.get(), globals.get(), 0)};
        if (!val.get())
        {
            return nullptr;
        }
    }

    return Py_None;
}

extern "C" PyObject*
IcePy_compile(PyObject* /*self*/, PyObject* args)
{
    PyObject* list = 0;
    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &list))
    {
        return nullptr;
    }

    vector<string> argSeq;
    if (list)
    {
        if (!listToStringSeq(list, argSeq))
        {
            return nullptr;
        }
    }

    int rc;
    try
    {
        rc = Slice::Python::compile(argSeq);
    }
    catch (const std::exception& ex)
    {
        consoleErr << argSeq[0] << ": error:" << ex.what() << endl;
        rc = EXIT_FAILURE;
    }
    catch (...)
    {
        consoleErr << argSeq[0] << ": error:"
                   << "unknown exception" << endl;
        rc = EXIT_FAILURE;
    }

    // PyInt_FromLong doesn't exist in python 3.
    return PyLong_FromLong(rc);
}
