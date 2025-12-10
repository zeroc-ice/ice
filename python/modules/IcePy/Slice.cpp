// Copyright (c) ZeroC, Inc.

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

#include <iostream>
#include <unordered_set>

using namespace std;
using namespace IcePy;
using namespace Slice;
using namespace Slice::Python;
using namespace IceInternal;

namespace
{
    // Track loaded fragments to avoid redefining types that were already loaded.
    unordered_set<string> loadedFragments;
}

extern "C" PyObject*
IcePy_loadSlice(PyObject* /*self*/, PyObject* args)
{
    PyObject* list{nullptr};
    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &list))
    {
        return nullptr;
    }

    vector<string> argSeq;
    if (list && !listToStringSeq(list, argSeq))
    {
        return nullptr;
    }

    IceInternal::Options opts;
    opts.addOpt("D", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("U", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("I", "", IceInternal::Options::NeedArg, "", IceInternal::Options::Repeat);
    opts.addOpt("d", "debug");

    vector<string> sliceFiles;
    try
    {
        argSeq.insert(argSeq.begin(), ""); // dummy argv[0]
        sliceFiles = opts.parse(argSeq);
        if (sliceFiles.empty())
        {
            PyErr_Format(PyExc_RuntimeError, "no Slice files specified in `%S'", args);
            return nullptr;
        }
    }
    catch (const std::exception& ex)
    {
        PyErr_Format(PyExc_RuntimeError, "error in Slice options: %s", ex.what());
        return nullptr;
    }

    vector<string> preprocessorArgs;
    bool debug = false;

    for (const auto& arg : opts.argVec("D"))
    {
        preprocessorArgs.push_back("-D" + arg);
    }

    for (const auto& arg : opts.argVec("U"))
    {
        preprocessorArgs.push_back("-U" + arg);
    }

    for (const auto& path : opts.argVec("I"))
    {
        preprocessorArgs.push_back("-I" + path);
    }

    debug = opts.isSet("d") || opts.isSet("debug");

    CompilationResult compilationResult;
    try
    {
        PackageVisitor packageVisitor;
        compilationResult = Slice::Python::compile(
            "Ice.loadSlice",
            nullptr, // No dependency generator
            packageVisitor,
            sliceFiles,
            preprocessorArgs,
            true, // Sort fragments to ensure they are evaluated in the correct order.
            CompilationKind::All,
            debug);
    }
    catch (const std::exception& ex)
    {
        PyErr_Format(PyExc_RuntimeError, "error during Slice compilation: %s", ex.what());
        return nullptr;
    }

    if (compilationResult.status == EXIT_FAILURE)
    {
        PyErr_Format(PyExc_RuntimeError, "Slice compilation failed");
        return nullptr;
    }

    for (const auto& fragment : compilationResult.fragments)
    {
        // Skip if the Python module for this fragment was already loaded
        if (loadedFragments.find(fragment.moduleName) != loadedFragments.end())
        {
            continue;
        }

        PyObject* moduleRef = PyImport_AddModule(fragment.moduleName.c_str());
        if (!moduleRef)
        {
            PyErr_Format(PyExc_RuntimeError, "failed to add module `%s'", fragment.moduleName.c_str());
            return nullptr;
        }

        // The file name is set to the Slice file the code was generated from, or to the package index file name for
        // package index fragments.
        string inputFile = fragment.isPackageIndex ? fragment.fileName : fragment.sliceFileName;

        PyObjectHandle code{Py_CompileString(fragment.code.c_str(), inputFile.c_str(), Py_file_input)};
        if (!code)
        {
            return nullptr;
        }

        PyObject* moduleDict = PyModule_GetDict(moduleRef); // Borrowed reference
        if (fragment.isPackageIndex)
        {
            // If this is a package index, we need to set the `__path__` attribute.
            PyObject* path = PyList_New(1);
            PyList_SetItem(path, 0, PyUnicode_FromString(fragment.fileName.c_str()));
            PyDict_SetItemString(moduleDict, "__path__", path);

            // Link package to the parent package if it exists.
            size_t dotPos = fragment.moduleName.rfind('.');
            if (dotPos != std::string::npos)
            {
                std::string parentName = fragment.moduleName.substr(0, dotPos);
                std::string childName = fragment.moduleName.substr(dotPos + 1);

                // Get a borrowed reference to the parent module.
                PyObject* parent = PyImport_AddModule(parentName.c_str());
                if (!parent)
                {
                    return nullptr;
                }

                PyObject* parentDict = PyModule_GetDict(parent);
                PyDict_SetItemString(parentDict, childName.c_str(), moduleRef);
            }
        }

        PyObject* builtins = PyEval_GetBuiltins();
        PyDict_SetItemString(moduleDict, "__builtins__", builtins);

        PyObjectHandle result{PyEval_EvalCode(code.get(), moduleDict, moduleDict)};
        if (!result)
        {
            return nullptr;
        }

        // Mark this fragment as loaded
        loadedFragments.insert(fragment.moduleName);
    }

    return Py_None;
}

extern "C" PyObject*
IcePy_compileSlice(PyObject* /*self*/, PyObject* args)
{
    PyObject* list{nullptr};
    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &list))
    {
        return nullptr;
    }

    vector<string> argSeq;
    if (list && !listToStringSeq(list, argSeq))
    {
        return nullptr;
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
        consoleErr << argSeq[0] << ": error:unknown exception" << endl;
        rc = EXIT_FAILURE;
    }

    return PyLong_FromLong(rc);
}
