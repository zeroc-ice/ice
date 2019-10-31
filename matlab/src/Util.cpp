//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// COMPILERFIX: codecvt_utf8_utf16 is deprecated in C++17
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 1

#include <Ice/LocalException.h>
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include "ice.h"
#include "Util.h"

using namespace std;

namespace
{

string
replace(string s, string patt, string val)
{
    auto r = s;
    auto pos = r.find(patt);
    while(pos != string::npos)
    {
        r.replace(pos, patt.size(), val);
        pos += val.size();
        pos = r.find(patt, pos);
    }
    return r;
}

void
getMajorMinor(mxArray* p, Ice::Byte& major, Ice::Byte& minor)
{
    auto maj = mxGetProperty(p, 0, "major");
    assert(maj);
    if(!mxIsScalar(maj))
    {
        throw std::invalid_argument("major is not a scalar");
    }
    major = static_cast<Ice::Byte>(mxGetScalar(maj));
    auto min = mxGetProperty(p, 0, "minor");
    assert(min);
    if(!mxIsScalar(min))
    {
        throw std::invalid_argument("minor is not a scalar");
    }
    minor = static_cast<Ice::Byte>(mxGetScalar(min));
}

}

mxArray*
IceMatlab::createStringFromUTF8(const string& s)
{
    if(s.empty())
    {
        return mxCreateString("");
    }
    else
    {
#ifdef _MSC_VER
        //
        // Workaround for Visual Studio bug that causes a link error when using char16_t.
        //
        wstring utf16 = wstring_convert<codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(s.data());
#else
        u16string utf16 = wstring_convert<codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(s.data());
#endif
        mwSize dims[2] = { 1, static_cast<mwSize>(utf16.size()) };
        auto r = mxCreateCharArray(2, dims);
        auto buf = mxGetChars(r);
        int i = 0;
#ifdef _MSC_VER
        for(wchar_t c : utf16)
#else
        for(char16_t c : utf16)
#endif
        {
            buf[i++] = static_cast<mxChar>(c);
        }
        return r;
    }
}

string
IceMatlab::getStringFromUTF16(mxArray* p)
{
    auto s = mxArrayToUTF8String(p);
    if(!s)
    {
        throw std::invalid_argument("value is not a char array");
    }
    string str(s);
    mxFree(s);
    return str;
}

mxArray*
IceMatlab::createEmpty()
{
    return mxCreateNumericMatrix(0, 0, mxDOUBLE_CLASS, mxREAL);
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
IceMatlab::createByte(Ice::Byte v)
{
    auto r = mxCreateNumericMatrix(1, 1, mxUINT8_CLASS, mxREAL);
    auto p = reinterpret_cast<Ice::Byte*>(mxGetPr(r));
    *p = v;
    return r;
}

mxArray*
IceMatlab::createShort(short v)
{
    auto r = mxCreateNumericMatrix(1, 1, mxINT16_CLASS, mxREAL);
    auto p = reinterpret_cast<short*>(mxGetPr(r));
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

mxArray*
IceMatlab::createLong(long long v)
{
    auto r = mxCreateNumericMatrix(1, 1, mxINT64_CLASS, mxREAL);
    auto p = reinterpret_cast<long long*>(mxGetPr(r));
    *p = v;
    return r;
}

mxArray*
IceMatlab::createFloat(float v)
{
    auto r = mxCreateNumericMatrix(1, 1, mxSINGLE_CLASS, mxREAL);
    auto p = reinterpret_cast<float*>(mxGetPr(r));
    *p = v;
    return r;
}

mxArray*
IceMatlab::createDouble(double v)
{
    auto r = mxCreateNumericMatrix(1, 1, mxDOUBLE_CLASS, mxREAL);
    auto p = reinterpret_cast<double*>(mxGetPr(r));
    *p = v;
    return r;
}

mxArray*
IceMatlab::createEnumerator(const string& type, int v)
{
    auto func = type + ".ice_getValue";
    auto param = createInt(v);
    mxArray* r;
    mexCallMATLAB(1, &r, 1, &param, func.c_str());
    // Calling this causes MATLAB to crash:
    //mxFree(param);
    return r;
}

int
IceMatlab::getEnumerator(mxArray* p, const string& type)
{
    if(!mxIsClass(p, type.c_str()))
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
    //mxFree(i);
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
    if(!mxIsClass(p, "Ice.Identity"))
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
    if(m.empty())
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
        for(auto p : m)
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
IceMatlab::getStringMap(mxArray* p, map<string, string>& m)
{
    if(mxIsEmpty(p))
    {
        m.clear();
    }
    else if(!mxIsClass(p, "containers.Map"))
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
            for(size_t i = 0; i < n; ++i)
            {
                auto k = getStringFromUTF16(mxGetCell(keys, static_cast<int>(i)));
                auto v = getStringFromUTF16(mxGetCell(values, static_cast<int>(i)));
                m[k] = v;
            }
            mxDestroyArray(keys);
            mxDestroyArray(values);
        }
        catch(...)
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
    if(!mxIsClass(p, "Ice.EncodingVersion"))
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

void
IceMatlab::getProtocolVersion(mxArray* p, Ice::ProtocolVersion& v)
{
    if(!mxIsClass(p, "Ice.ProtocolVersion"))
    {
        throw std::invalid_argument("argument is not Ice.ProtocolVersion");
    }
    getMajorMinor(p, v.major, v.minor);
}

mxArray*
IceMatlab::convertException(const std::exception& exc)
{
    mxArray* ex;
    if(dynamic_cast<const Ice::LocalException*>(&exc))
    {
        auto iceEx = dynamic_cast<const Ice::LocalException*>(&exc);
        auto typeId = iceEx->ice_id();
        //
        // The exception ID uses single colon separators.
        //
        auto id = typeId.substr(2); // Remove leading "::" from type ID
        id = replace(id, "::", ":");

        auto cls = typeId.substr(2); // Remove leading "::" from type ID
        cls = replace(cls, "::", ".");

        mxArray* params[10];
        params[0] = createStringFromUTF8(id);
        int idx = 2;
        auto msg = typeId; // Use the type ID as the default exception message

        try
        {
            iceEx->ice_throw();
        }
        catch(const Ice::InitializationException& e)
        {
            msg = e.reason;
            params[idx++] = createStringFromUTF8(e.reason);
        }
        catch(const Ice::PluginInitializationException& e)
        {
            msg = e.reason;
            params[idx++] = createStringFromUTF8(e.reason);
        }
        catch(const Ice::AlreadyRegisteredException& e)
        {
            params[idx++] = createStringFromUTF8(e.kindOfObject);
            params[idx++] = createStringFromUTF8(e.id);
        }
        catch(const Ice::NotRegisteredException& e)
        {
            params[idx++] = createStringFromUTF8(e.kindOfObject);
            params[idx++] = createStringFromUTF8(e.id);
        }
        catch(const Ice::TwowayOnlyException& e)
        {
            params[idx++] = createStringFromUTF8(e.operation);
        }
        catch(const Ice::UnknownException& e)
        {
            params[idx++] = createStringFromUTF8(e.unknown);
        }
        catch(const Ice::ObjectAdapterDeactivatedException& e)
        {
            params[idx++] = createStringFromUTF8(e.name);
        }
        catch(const Ice::ObjectAdapterIdInUseException& e)
        {
            params[idx++] = createStringFromUTF8(e.id);
        }
        catch(const Ice::NoEndpointException& e)
        {
            params[idx++] = createStringFromUTF8(e.proxy);
        }
        catch(const Ice::EndpointParseException& e)
        {
            params[idx++] = createStringFromUTF8(e.str);
        }
        catch(const Ice::EndpointSelectionTypeParseException& e)
        {
            params[idx++] = createStringFromUTF8(e.str);
        }
        catch(const Ice::VersionParseException& e)
        {
            params[idx++] = createStringFromUTF8(e.str);
        }
        catch(const Ice::IdentityParseException& e)
        {
            params[idx++] = createStringFromUTF8(e.str);
        }
        catch(const Ice::ProxyParseException& e)
        {
            params[idx++] = createStringFromUTF8(e.str);
        }
        catch(const Ice::IllegalIdentityException& e)
        {
            params[idx++] = createIdentity(e.id);
        }
        catch(const Ice::IllegalServantException& e)
        {
            params[idx++] = createStringFromUTF8(e.reason);
        }
        catch(const Ice::RequestFailedException& e)
        {
            params[idx++] = createIdentity(e.id);
            params[idx++] = createStringFromUTF8(e.facet);
            params[idx++] = createStringFromUTF8(e.operation);
        }
        catch(const Ice::FileException& e)
        {
            params[idx++] = mxCreateDoubleScalar(e.error);
            params[idx++] = createStringFromUTF8(e.path);
        }
        catch(const Ice::SyscallException& e) // This must appear after all subclasses of SyscallException.
        {
            params[idx++] = mxCreateDoubleScalar(e.error);
        }
        catch(const Ice::DNSException& e)
        {
            params[idx++] = mxCreateDoubleScalar(e.error);
            params[idx++] = createStringFromUTF8(e.host);
        }
        catch(const Ice::BadMagicException& e)
        {
            params[idx++] = createStringFromUTF8(e.reason);
            params[idx++] = createByteList(e.badMagic);
        }
        catch(const Ice::UnsupportedProtocolException& e)
        {
            params[idx++] = createStringFromUTF8(e.reason);
            params[idx++] = createProtocolVersion(e.bad);
            params[idx++] = createProtocolVersion(e.supported);
        }
        catch(const Ice::UnsupportedEncodingException& e)
        {
            params[idx++] = createStringFromUTF8(e.reason);
            params[idx++] = createEncodingVersion(e.bad);
            params[idx++] = createEncodingVersion(e.supported);
        }
        catch(const Ice::NoValueFactoryException& e)
        {
            params[idx++] = createStringFromUTF8(e.reason);
            params[idx++] = createStringFromUTF8(e.type);
        }
        catch(const Ice::UnexpectedObjectException& e)
        {
            params[idx++] = createStringFromUTF8(e.reason);
            params[idx++] = createStringFromUTF8(e.type);
            params[idx++] = createStringFromUTF8(e.expectedType);
        }
        catch(const Ice::ProtocolException& e) // This must appear after all subclasses of ProtocolException.
        {
            params[idx++] = createStringFromUTF8(e.reason);
        }
        catch(const Ice::ConnectionManuallyClosedException& e)
        {
            params[idx++] = mxCreateLogicalScalar(e.graceful ? 1 : 0);
        }
        catch(const Ice::FeatureNotSupportedException& e)
        {
            params[idx++] = createStringFromUTF8(e.unsupportedFeature);
        }
        catch(const Ice::SecurityException& e)
        {
            params[idx++] = createStringFromUTF8(e.reason);
        }
        catch(const Ice::LocalException&)
        {
            //
            // Nothing to do.
            //
        }

        //
        // NOTE: Matlab interprets the msg argument as an sprintf format string. It will complain if it
        // finds invalid syntax.
        //
        params[1] = createStringFromUTF8(msg);
        mexCallMATLAB(1, &ex, idx, params, cls.c_str());
    }
    else if(dynamic_cast<const std::invalid_argument*>(&exc))
    {
        mxArray* params[2];
        params[0] = createStringFromUTF8("Ice:InvalidArgumentException");
        params[1] = createStringFromUTF8(exc.what());
        mexCallMATLAB(1, &ex, 2, params, "MException");
    }
    else
    {
        mxArray* params[2];
        params[0] = createStringFromUTF8("Ice:CppException");
        params[1] = createStringFromUTF8(exc.what());
        mexCallMATLAB(1, &ex, 2, params, "MException");
    }
    return ex;
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
    if(hasValue)
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
    for(auto s : strings)
    {
        mxSetCell(r, i++, createStringFromUTF8(s));
    }
    return r;
}

void
IceMatlab::getStringList(mxArray* m, vector<string>& v)
{
    if(!mxIsCell(m))
    {
        throw std::invalid_argument("argument is not a cell array");
    }
    if(mxGetM(m) > 1)
    {
        throw std::invalid_argument("invalid dimension in cell array");
    }
    size_t n = mxGetN(m);
    v.clear();
    for(auto i = 0; i < n; ++i)
    {
        mxArray* c = mxGetCell(m, i);
        v.push_back(getStringFromUTF16(c));
    }
}

mxArray*
IceMatlab::createByteArray(const Ice::Byte* begin, const Ice::Byte* end)
{
    mxArray* r = mxCreateUninitNumericMatrix(1, end - begin, mxUINT8_CLASS, mxREAL);
    memcpy(reinterpret_cast<Ice::Byte*>(mxGetData(r)), begin, end - begin);
    return r;
}

mxArray*
IceMatlab::createByteList(const vector<Ice::Byte>& bytes)
{
    auto r = mxCreateCellMatrix(1, static_cast<int>(bytes.size()));
    mwIndex i = 0;
    for(auto byte : bytes)
    {
        mxSetCell(r, i++, createByte(byte));
    }
    return r;
}

mxArray*
IceMatlab::createCertificateList(const vector<IceSSL::CertificatePtr>& certs)
{
    auto r = mxCreateCellMatrix(1, static_cast<int>(certs.size()));
    mwIndex i = 0;
    for(auto cert : certs)
    {
        mxSetCell(r, i++, createStringFromUTF8(cert->encode()));
    }
    return r;
}

namespace
{

string
lookupKwd(const string& name)
{
    //
    // Keyword list. *Must* be kept in alphabetical order.
    //
    // This list must match the one in slice2matlab.
    //
    static const string keywordList[] =
    {
        "break", "case", "catch", "classdef", "continue", "else", "elseif", "end", "for", "function", "global",
        "if", "otherwise", "parfor", "persistent", "return", "spmd", "switch", "try", "while"
    };
    bool found = binary_search(&keywordList[0],
                               &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                               name);
    return found ? "slice_" + name : name;
}

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
vector<string>
splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    vector<string> ids;
    string::size_type next = 0;
    string::size_type pos;
    while((pos = scoped.find("::", next)) != string::npos)
    {
        pos += 2;
        if(pos != scoped.size())
        {
            string::size_type endpos = scoped.find("::", pos);
            if(endpos != string::npos)
            {
                ids.push_back(scoped.substr(pos, endpos - pos));
            }
        }
        next = pos;
    }
    if(next != scoped.size())
    {
        ids.push_back(scoped.substr(next));
    }
    else
    {
        ids.push_back("");
    }

    return ids;
}

}

string
IceMatlab::idToClass(const string& id)
{
    auto ids = splitScopedName(id);
    transform(ids.begin(), ids.end(), ids.begin(), [](const auto& id)
                                                   {
                                                       return lookupKwd(id);
                                                   });
    stringstream result;
    for(auto i = ids.begin(); i != ids.end(); ++i)
    {
        if(i != ids.begin())
        {
            result << ".";
        }
        result << *i;
    }
    return result.str();
}
