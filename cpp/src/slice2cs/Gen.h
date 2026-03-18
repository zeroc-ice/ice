// Copyright (c) ZeroC, Inc.

#ifndef GEN_H
#define GEN_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice
{
    enum class GenMode
    {
        Ice,
        IceRpc
    };

    class Gen final
    {
    public:
        Gen(const std::string& base, const std::string& dir, GenMode genMode, bool enableAnalysis);
        Gen(const Gen&) = delete;
        ~Gen();

        void generate(const UnitPtr&);

    private:
        const GenMode _genMode;
        IceInternal::Output _out;
        IceInternal::Output _iceRpcOut;
        const bool _enableAnalysis;

        static void printHeader(IceInternal::Output& out, const std::string& iceFile, bool enableAnalysis);
    };
}

#endif
