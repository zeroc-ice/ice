// Copyright (c) ZeroC, Inc.

#ifndef ICE_RUBY_RUBY_SLICE_LOADER_H
#define ICE_RUBY_RUBY_SLICE_LOADER_H

#include "Config.h"
#include "Ice/Ice.h"

namespace IceRuby
{
    /// Implements SliceLoader using a Ruby SliceLoader.
    class RubySliceLoader final : public Ice::SliceLoader
    {
    public:
        explicit RubySliceLoader(VALUE sliceLoader);
        ~RubySliceLoader() final;

        // Registers a GC root tied to the address of _sliceLoader, so it's neither copyable nor movable.
        RubySliceLoader(const RubySliceLoader&) = delete;
        RubySliceLoader(RubySliceLoader&&) = delete;
        RubySliceLoader& operator=(const RubySliceLoader&) = delete;
        RubySliceLoader& operator=(RubySliceLoader&&) = delete;

        [[nodiscard]] Ice::ValuePtr newClassInstance(std::string_view typeId) const final;

        // TODO: we currently don't support loading of custom Slice exceptions from Ruby.

    private:
        VALUE _sliceLoader;
    };
}

#endif
