// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_PY_SLICE_LOADER_H
#define ICEPY_PY_SLICE_LOADER_H

#include "Config.h"
#include "Ice/Ice.h"

namespace IcePy
{
    /// Implements SliceLoader using a Python SliceLoader.
    class PySliceLoader final : public Ice::SliceLoader
    {
    public:
        explicit PySliceLoader(PyObject* sliceLoader);
        ~PySliceLoader() final;

        [[nodiscard]] Ice::ValuePtr newClassInstance(std::string_view typeId) const final;

        // TODO: we currently don't support loading of custom Slice exceptions from Python.

    private:
        PyObject* _sliceLoader;
    };
}

#endif
