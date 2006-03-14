// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <Slice.h>
#include <Util.h>
#include <Slice/Preprocessor.h>
#include <Slice/PythonUtil.h>
#include <IceUtil/Options.h>

//
// Python headers needed for PyEval_EvalCode.
//
#include <compile.h>
#include <eval.h>

using namespace std;
using namespace IcePy;
using namespace Slice;
using namespace Slice::Python;

extern "C"
PyObject*
IcePy_loadSlice(PyObject* /*self*/, PyObject* args)
{
    char* cmd;
    PyObject* list = NULL;
    if(!PyArg_ParseTuple(args, STRCAST("s|O!"), &cmd, &PyList_Type, &list))
    {
        return NULL;
    }

    vector<string> argSeq;
    try
    {
	argSeq = IceUtil::Options::split(cmd);
    }
    catch(const IceUtil::Options::Error& ex)
    {
	PyErr_Format(PyExc_RuntimeError, "error in Slice options: %s", ex.reason.c_str());
	return NULL;
    }

    if(list != NULL)
    {
        listToStringSeq(list, argSeq);
    }

    IceUtil::Options opts;
    opts.addOpt("D", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("U", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("I", "", IceUtil::Options::NeedArg, "", IceUtil::Options::Repeat);
    opts.addOpt("d", "debug");
    opts.addOpt("", "ice");
    opts.addOpt("", "checksum");
    opts.addOpt("", "all");
    opts.addOpt("", "case-sensitive");

    vector<string> files;
    try
    {
	argSeq.insert(argSeq.begin(), ""); // dummy argv[0]
	files = opts.parse(argSeq);
	if(files.empty())
	{
	    PyErr_Format(PyExc_RuntimeError, "no Slice files specified in `%s'", cmd);
	    return NULL;
	}
    }
    catch(const IceUtil::Options::BadOpt& ex)
    {
	PyErr_Format(PyExc_RuntimeError, "error in Slice options: %s", ex.reason.c_str());
	return NULL;
    }

    string cppArgs;
    Ice::StringSeq includePaths;
    bool debug = false;
    bool ice = true; // This must be true so that we can create Ice::Identity when necessary.
    bool caseSensitive = false;
    bool all = false;
    bool checksum = false;
    if(opts.isSet("D"))
    {
	vector<string> optargs = opts.argVec("D");
	for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
	{
	    cppArgs += " -D" + *i;
	}
    }
    if(opts.isSet("U"))
    {
	vector<string> optargs = opts.argVec("U");
	for(vector<string>::const_iterator i = optargs.begin(); i != optargs.end(); ++i)
	{
	    cppArgs += " -U" + *i;
	}
    }
    if(opts.isSet("I"))
    {
	includePaths = opts.argVec("I");
	for(vector<string>::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
	{
	    cppArgs += " -I" + *i;
	}
    }
    debug = opts.isSet("d") || opts.isSet("debug");
    caseSensitive = opts.isSet("case-sensitive");
    all = opts.isSet("all");
    checksum = opts.isSet("checksum");

    bool ignoreRedefs = false;

    for(vector<string>::const_iterator p = files.begin(); p != files.end(); ++p)
    {
        string file = *p;
        Slice::Preprocessor icecpp("icecpp", file, cppArgs);
        FILE* cppHandle = icecpp.preprocess(false);

        if(cppHandle == 0)
        {
            PyErr_Format(PyExc_RuntimeError, "Slice preprocessing failed for `%s'", cmd);
            return NULL;
        }

        UnitPtr u = Slice::Unit::createUnit(ignoreRedefs, all, ice, caseSensitive);
        int parseStatus = u->parse(cppHandle, debug);

        if(!icecpp.close() || parseStatus == EXIT_FAILURE)
        {
            PyErr_Format(PyExc_RuntimeError, "Slice parsing failed for `%s'", cmd);
            u->destroy();
            return NULL;
        }

        //
        // Generate the Python code into a string stream.
        //
        ostringstream codeStream;
        IceUtil::Output out(codeStream);
        out.setUseTab(false);
        generate(u, all, checksum, includePaths, out);
        u->destroy();

        string code = codeStream.str();
        PyObjectHandle src = Py_CompileString(const_cast<char*>(code.c_str()), const_cast<char*>(file.c_str()),
                                              Py_file_input);
        if(src.get() == NULL)
        {
            return NULL;
        }

        PyObjectHandle globals = PyDict_New();
        if(globals.get() == NULL)
        {
            return NULL;
        }
        PyDict_SetItemString(globals.get(), "__builtins__", PyEval_GetBuiltins());

        PyObjectHandle val = PyEval_EvalCode((PyCodeObject*)src.get(), globals.get(), 0);
        if(val.get() == NULL)
        {
            return NULL;
        }
    }

    Py_INCREF(Py_None);
    return Py_None;
}
