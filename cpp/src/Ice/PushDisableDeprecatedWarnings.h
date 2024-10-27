// Copyright (c) ZeroC, Inc.

// This file can be included several times in a translation unit.

#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4996) // ... was declared deprecated

#elif defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wdeprecated-declarations"

#elif defined(__GNUC__)
#    pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
