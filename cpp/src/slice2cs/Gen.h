// Copyright (c) ZeroC, Inc.

#ifndef GEN_H
#define GEN_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice
{
    class Gen final
    {
    public:
        Gen(const std::string&, const std::string&, bool);
        Gen(const Gen&) = delete;
        ~Gen();

        void generate(const UnitPtr&);

    private:
        IceInternal::Output _out;
        bool _enableAnalysis;

        void printHeader();
    };
}

#endif
