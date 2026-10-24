// Copyright (c) ZeroC, Inc.

#ifndef GEN_H
#define GEN_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice
{
    /// The RPC framework the generated code targets: Ice, or a version of IceRPC C#. IceRPC C# 0.6 and 0.7 share the
    /// same generated code.
    enum class GenMode
    {
        Ice,
        IceRpc06,
        IceRpc08
    };

    class Gen final
    {
    public:
        Gen(const std::string& base, const std::string& dir, GenMode genMode, bool enableAnalysis);
        Gen(const Gen&) = delete;
        ~Gen();

        void generate(const UnitPtr& unit);

    private:
        const GenMode _genMode;
        IceInternal::Output _out;
        const bool _enableAnalysis;
        std::string _fileBase;

        void printHeader(const std::string& iceFile);
    };
}

#endif
