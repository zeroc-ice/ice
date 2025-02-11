// Copyright (c) ZeroC, Inc.

#include "Util.h"
#include "Ice/LocalExceptions.h"
#include "ice.h"

#include <array>
#include <locale>
#include <string>

using namespace std;

namespace
{
    string replace(string s, string patt, string val)
    {
        auto r = s;
        auto pos = r.find(patt);
        while (pos != string::npos)
        {
            r.replace(pos, patt.size(), val);
            pos += val.size();
            pos = r.find(patt, pos);
        }
        return r;
    }

    void getMajorMinor(mxArray* p, uint8_t& major, uint8_t& minor)
    {
        auto maj = mxGetProperty(p, 0, "major");
        assert(maj);
        if (!mxIsScalar(maj))
        {
            throw std::invalid_argument("major is not a scalar");
        }
        major = static_cast<uint8_t>(mxGetScalar(maj));
        auto min = mxGetProperty(p, 0, "minor");
        assert(min);
        if (!mxIsScalar(min))
        {
            throw std::invalid_argument("minor is not a scalar");
        }
        minor = static_cast<uint8_t>(mxGetScalar(min));
    }
}

mxArray*
IceMatlab::createStringFromUTF8(const string& s)
{
    // mxCreateString accepts a UTF-8 input since MATLAB 2020b.
    return mxCreateString(s.c_str());
}

string
IceMatlab::getStringFromUTF16(mxArray* p)
{
    char* s = mxArrayToUTF8String(p);
    if (!s)
    {
        throw std::invalid_argument("value is not a char array");
    }
    string str{s};
    mxFree(s);
    return str;
}

mxArray*
IceMatlab::createBool(bool v)
{
    auto r = mxCreateNumericMatrix(1, 1, mxLOGICAL_CLASS, mxREAL);
    auto p = reinterpret_cast<bool*>(mxGetPr(r));
    *p = v;
    return r;
}

mxArray*
IceMatlab::createByte(uint8_t v)
{
    auto r = mxCreateNumericMatrix(1, 1, mxUINT8_CLASS, mxREAL);
    auto p = reinterpret_cast<uint8_t*>(mxGetPr(r));
    *p = v;
    return r;
}

mxArray*
IceMatlab::createInt(int v)
{
    auto r = mxCreateNumericMatrix(1, 1, mxINT32_CLASS, mxREAL);
    auto p = reinterpret_cast<int*>(mxGetPr(r));
    *p = v;
    return r;
}

int
IceMatlab::getEnumerator(mxArray* p, const string& type)
{
    if (!mxIsClass(p, type.c_str()))
    {
        throw invalid_argument("expected enumerator of type " + type);
    }
    //
    // Convert the enumerator to an integer.
    //
    mxArray* i;
    mexCallMATLAB(1, &i, 1, &p, "int32");
    int r = static_cast<int>(mxGetScalar(i));
    // Calling this causes MATLAB to crash:
    // mxFree(i);
    return r;
}

mxArray*
IceMatlab::createIdentity(const Ice::Identity& id)
{
    mxArray* params[2];
    params[0] = createStringFromUTF8(id.name);
    params[1] = createStringFromUTF8(id.category);
    mxArray* r;
    mexCallMATLAB(1, &r, 2, params, "Ice.Identity");
    return r;
}

void
IceMatlab::getIdentity(mxArray* p, Ice::Identity& id)
{
    if (!mxIsClass(p, "Ice.Identity"))
    {
        throw std::invalid_argument("argument is not Ice.Identity");
    }
    auto name = mxGetProperty(p, 0, "name");
    assert(name);
    id.name = getStringFromUTF16(name);
    auto category = mxGetProperty(p, 0, "category");
    assert(category);
    id.category = getStringFromUTF16(category);
}

mxArray*
IceMatlab::createStringMap(const map<string, string>& m)
{
    mxArray* r;
    if (m.empty())
    {
        mexCallMATLAB(1, &r, 0, 0, "containers.Map");
    }
    else
    {
        mwSize dims[2] = {1, 0};
        dims[1] = static_cast<int>(m.size());
        auto keys = mxCreateCellArray(2, dims);
        auto values = mxCreateCellArray(2, dims);
        int idx = 0;
        for (auto p : m)
        {
            mxSetCell(keys, idx, createStringFromUTF8(p.first));
            mxSetCell(values, idx, createStringFromUTF8(p.second));
            idx++;
        }
        mxArray* params[2];
        params[0] = keys;
        params[1] = values;
        mexCallMATLAB(1, &r, 2, params, "containers.Map");
    }
    return r;
}

// TODO: reduce duplication with code above

mxArray*
IceMatlab::createContext(const Ice::Context& m)
{
    mxArray* r;
    if (m.empty())
    {
        mexCallMATLAB(1, &r, 0, 0, "containers.Map");
    }
    else
    {
        mwSize dims[2] = {1, 0};
        dims[1] = static_cast<int>(m.size());
        auto keys = mxCreateCellArray(2, dims);
        auto values = mxCreateCellArray(2, dims);
        int idx = 0;
        for (auto p : m)
        {
            mxSetCell(keys, idx, createStringFromUTF8(p.first));
            mxSetCell(values, idx, createStringFromUTF8(p.second));
            idx++;
        }
        mxArray* params[2];
        params[0] = keys;
        params[1] = values;
        mexCallMATLAB(1, &r, 2, params, "containers.Map");
    }
    return r;
}

void
IceMatlab::getContext(mxArray* p, Ice::Context& m)
{
    if (mxIsEmpty(p))
    {
        m.clear();
    }
    else if (!mxIsClass(p, "containers.Map"))
    {
        throw std::invalid_argument("argument is not a containers.Map");
    }
    else
    {
        mxArray* params[1];
        params[0] = p;
        mxArray* keys;
        mexCallMATLAB(1, &keys, 1, params, "keys");
        mxArray* values;
        mexCallMATLAB(1, &values, 1, params, "values");
        assert(mxGetM(keys) == 1 && mxGetM(values) == 1);
        assert(mxGetN(keys) == mxGetN(values));
        const size_t n = mxGetN(keys);
        try
        {
            for (size_t i = 0; i < n; ++i)
            {
                auto k = getStringFromUTF16(mxGetCell(keys, static_cast<int>(i)));
                auto v = getStringFromUTF16(mxGetCell(values, static_cast<int>(i)));
                m[k] = v;
            }
            mxDestroyArray(keys);
            mxDestroyArray(values);
        }
        catch (...)
        {
            mxDestroyArray(keys);
            mxDestroyArray(values);
            throw;
        }
    }
}

mxArray*
IceMatlab::createEncodingVersion(const Ice::EncodingVersion& v)
{
    mxArray* params[2];
    params[0] = mxCreateDoubleScalar(v.major);
    params[1] = mxCreateDoubleScalar(v.minor);
    mxArray* r;
    mexCallMATLAB(1, &r, 2, params, "Ice.EncodingVersion");
    return r;
}

void
IceMatlab::getEncodingVersion(mxArray* p, Ice::EncodingVersion& v)
{
    if (!mxIsClass(p, "Ice.EncodingVersion"))
    {
        throw std::invalid_argument("argument is not Ice.EncodingVersion");
    }
    getMajorMinor(p, v.major, v.minor);
}

mxArray*
IceMatlab::createProtocolVersion(const Ice::ProtocolVersion& v)
{
    mxArray* params[2];
    params[0] = mxCreateDoubleScalar(v.major);
    params[1] = mxCreateDoubleScalar(v.minor);
    mxArray* r;
    mexCallMATLAB(1, &r, 2, params, "Ice.ProtocolVersion");
    return r;
}

namespace
{
    template<size_t N> mxArray* createMatlabException(const char* typeId, std::array<mxArray*, N> params)
    {
        string className = replace(string{typeId}.substr(2), "::", ".");
        mxArray* ex;
        mexCallMATLAB(1, &ex, static_cast<int>(N), params.data(), className.c_str()); // error is fatal
        return ex;
    }

    // Create a "standard" MATLAB exception for the given typeId then fallback to LocalException.
    mxArray* createMatlabException(const char* typeId, const char* what)
    {
        string errID = replace(string{typeId}.substr(2), "::", ":");
        std::array params{IceMatlab::createStringFromUTF8(errID), IceMatlab::createStringFromUTF8(what)};

        string className = replace(string{typeId}.substr(2), "::", ".");
        mxArray* ex;

        // keep going on error
        mexCallMATLABWithTrap(1, &ex, static_cast<int>(params.size()), params.data(), className.c_str());
        if (!ex)
        {
            mexCallMATLAB(1, &ex, static_cast<int>(params.size()), params.data(), "Ice.LocalException");
        }
        return ex;
    }
}

mxArray*
IceMatlab::convertException(const std::exception_ptr exc)
{
    const char* const localExceptionTypeId = "::Ice::LocalException";
    mxArray* result;

    try
    {
        rethrow_exception(exc);
    }
    // We need to catch and convert:
    // - local exceptions thrown from MATLAB code for which we provide a convience constructor (e.g. MarshalException)
    // - local exceptions that define extra properties we want to expose to MATLAB users (e.g. ObjectNotExistException
    // via its base class, RequestFailedException)
    catch (const Ice::AlreadyRegisteredException& e)
    {
        // Adapt to convenience constructor. We don't pass what() to MATLAB.
        std::array params{createStringFromUTF8(e.kindOfObject()), createStringFromUTF8(e.id())};
        result = createMatlabException(e.ice_id(), std::move(params));
    }
    catch (const Ice::NotRegisteredException& e)
    {
        // Adapt to convenience constructor. We don't pass what() to MATLAB.
        std::array params{createStringFromUTF8(e.kindOfObject()), createStringFromUTF8(e.id())};
        result = createMatlabException(e.ice_id(), std::move(params));
    }
    catch (const Ice::ConnectionAbortedException& e)
    {
        // ConnectionAbortedException does not have a convenience constructor since it's never thrown from MATLAB code.
        string errID = replace(string{e.ice_id()}.substr(2), "::", ":");
        std::array params{
            createBool(e.closedByApplication()),
            createStringFromUTF8(errID),
            createStringFromUTF8(e.what())};
        result = createMatlabException(e.ice_id(), std::move(params));
    }
    catch (const Ice::ConnectionClosedException& e)
    {
        // ConnectionClosedException does not have a convenience constructor since it's never thrown from MATLAB code.
        string errID = replace(string{e.ice_id()}.substr(2), "::", ":");
        std::array params{
            createBool(e.closedByApplication()),
            createStringFromUTF8(errID),
            createStringFromUTF8(e.what())};
        result = createMatlabException(e.ice_id(), std::move(params));
    }
    catch (const Ice::MarshalException& e)
    {
        // Adapt to convenience constructor.
        std::array params{createStringFromUTF8(e.what())};
        result = createMatlabException(e.ice_id(), std::move(params));
    }
    catch (const Ice::RequestFailedException& e)
    {
        // The *NotExist exceptions are thrown only from the C++ code. They don't have a convenience constructor, but
        // they have extra properties.
        string errID = replace(string{e.ice_id()}.substr(2), "::", ":");
        std::array params{
            createIdentity(e.id()),
            createStringFromUTF8(e.facet()),
            createStringFromUTF8(e.operation()),
            createStringFromUTF8(errID),
            createStringFromUTF8(e.what())};

        result = createMatlabException(e.ice_id(), std::move(params));
    }
    catch (const Ice::UnknownUserException& e)
    {
        // Adapt to convenience constructor. First parameter is ignored.
        std::array params{createStringFromUTF8(""), createStringFromUTF8(e.what())};
        result = createMatlabException(e.ice_id(), std::move(params));
    }
    catch (const Ice::TwowayOnlyException&)
    {
        // The Ice C++ client runtime does not throw this exception. We handle it here because it has a special
        // constructor in MATLAB.
        assert(false);
        result = nullptr;
    }
    catch (const Ice::LocalException& e)
    {
        result = createMatlabException(e.ice_id(), e.what());
    }
    catch (const std::exception& e)
    {
        std::array params{createStringFromUTF8("Ice:CppException"), createStringFromUTF8(e.what())};
        result = createMatlabException(localExceptionTypeId, std::move(params));
    }
    catch (...)
    {
        std::array params{createStringFromUTF8("Ice:CppException"), createStringFromUTF8("unknown C++ exception")};
        result = createMatlabException(localExceptionTypeId, std::move(params));
    }

    return result;
}

static const char* resultFields[] = {"exception", "result"};

mxArray*
IceMatlab::createResultValue(mxArray* result)
{
    mwSize dims[2] = {1, 1};
    auto r = mxCreateStructArray(2, dims, 2, resultFields);
    mxSetFieldByNumber(r, 0, 1, result);
    return r;
}

mxArray*
IceMatlab::createResultException(mxArray* ex)
{
    mwSize dims[2] = {1, 1};
    auto r = mxCreateStructArray(2, dims, 2, resultFields);
    mxSetFieldByNumber(r, 0, 0, ex);
    return r;
}

static const char* optionalFields[] = {"hasValue", "value"};

mxArray*
IceMatlab::createOptionalValue(bool hasValue, mxArray* value)
{
    mwSize dims[2] = {1, 1};
    auto r = mxCreateStructArray(2, dims, 2, optionalFields);
    mxSetFieldByNumber(r, 0, 0, createBool(hasValue));
    if (hasValue)
    {
        mxSetFieldByNumber(r, 0, 1, value);
    }
    return r;
}

mxArray*
IceMatlab::createStringList(const vector<string>& strings)
{
    auto r = mxCreateCellMatrix(1, static_cast<int>(strings.size()));
    mwIndex i = 0;
    for (auto s : strings)
    {
        mxSetCell(r, i++, createStringFromUTF8(s));
    }
    return r;
}

void
IceMatlab::getStringList(mxArray* m, vector<string>& v)
{
    if (!mxIsCell(m))
    {
        throw std::invalid_argument("argument is not a cell array");
    }
    if (mxGetM(m) > 1)
    {
        throw std::invalid_argument("invalid dimension in cell array");
    }
    size_t n = mxGetN(m);
    v.clear();
    for (mwIndex i = 0; i < n; ++i)
    {
        mxArray* c = mxGetCell(m, i);
        v.push_back(getStringFromUTF16(c));
    }
}

mxArray*
IceMatlab::createByteArray(const byte* begin, const byte* end)
{
    mxArray* r = mxCreateUninitNumericMatrix(1, end - begin, mxUINT8_CLASS, mxREAL);
    memcpy(reinterpret_cast<uint8_t*>(mxGetData(r)), begin, end - begin);
    return r;
}
