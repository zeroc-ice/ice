// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include <mex.h>

namespace IceMatlab
{
    // We return an empty mxArray to signal success when calling a "void" function with calllib.
    inline mxArray* createEmptyArray() { return mxCreateDoubleMatrix(0, 0, mxREAL); }

    mxArray* createStringFromUTF8(const std::string&);
    std::string getStringFromUTF16(mxArray*);
    mxArray* createBool(bool);
    mxArray* createByte(std::uint8_t);
    mxArray* createInt(int);
    int getEnumerator(mxArray*, const std::string&);
    mxArray* createIdentity(const Ice::Identity&);
    void getIdentity(mxArray*, Ice::Identity&);

    /// Converts a C++ string map into a MATLAB dictionary.
    mxArray* createStringMap(const std::map<std::string, std::string, std::less<>>&);

    /// Alias for createStringMap.
    inline mxArray* createContext(const Ice::Context& ctx) { return createStringMap(ctx); }

    /// Converts a MATLAB dictionary into a C++ context (string map).
    void getContext(mxArray*, Ice::Context&);

    mxArray* createProtocolVersion(const Ice::ProtocolVersion&);
    mxArray* createEncodingVersion(const Ice::EncodingVersion&);
    void getEncodingVersion(mxArray*, Ice::EncodingVersion&);
    mxArray* convertException(std::exception_ptr);
    mxArray* createResultValue(mxArray*);
    mxArray* createResultException(mxArray*);
    mxArray* createOptionalValue(bool, mxArray*);

    /// Converts a vector<string> into a MATLAB string array.
    mxArray* createStringList(const std::vector<std::string>&);

    /// Converts a MATLAB string array or MATLAB cell array of char into a vector<string>.
    void getStringList(mxArray*, std::vector<std::string>&);

    mxArray* createByteArray(const std::byte*, const std::byte*);

    template<typename T> std::shared_ptr<T> deref(void* p) { return *reinterpret_cast<std::shared_ptr<T>*>(p); }

    template<typename T> void* createShared(std::shared_ptr<T> p) { return new std::shared_ptr<T>(std::move(p)); }

    inline void* createProxy(Ice::ObjectPrx p) { return new Ice::ObjectPrx(std::move(p)); }

    inline void* createProxy(std::optional<Ice::ObjectPrx> p)
    {
        return p ? createProxy(std::move(p).value()) : nullptr;
    }

    inline Ice::ObjectPrx restoreProxy(void* p)
    {
        assert(p);
        return *reinterpret_cast<Ice::ObjectPrx*>(p);
    }

    inline std::optional<Ice::ObjectPrx> restoreNullableProxy(void* p)
    {
        if (p)
        {
            return restoreProxy(p);
        }
        else
        {
            return std::nullopt;
        }
    }
}
