// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Time.h>
#include <Ice/LoggerI.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>

#ifdef _WIN32
#  include <IceUtil/StringUtil.h>
#  include <IceUtil/ScopedArray.h>
#  include <IceUtil/StringConverter.h>
#endif

#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

IceUtil::Mutex* outputMutex = 0;

class Init
{
public:

    Init()
    {
        outputMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete outputMutex;
        outputMutex = 0;
    }
};

Init init;

#ifdef _WIN32
string
UTF8ToCodePage(const string& in, int codePage)
{
    string out;
    if(!in.empty())
    {
        if(CP_UTF8 == codePage)
        {
            out = in;
        }
        else
        {
            IceUtil::ScopedArray<wchar_t> wbuffer;
            int size = 0;
            int wlength = 0;
            do
            {
                size == 0 ? 2 * in.size() : 2 * size;
                wbuffer.reset(new wchar_t[size]);
                wlength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in.c_str(), -1, wbuffer.get(), size);
            }
            while(wlength == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

            if(wlength == 0)
            {
                throw IceUtil::IllegalConversionException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());
            }

            //
            // WC_ERR_INVALID_CHARS conversion flag is only supported with 65001 (UTF-8) and
            // 54936 (GB18030 Simplified Chinese)
            //
            DWORD conversionFlags = (codePage == 65001 || codePage == 54936) ? WC_ERR_INVALID_CHARS : 0;

            IceUtil::ScopedArray<char> buffer;

            size = 0;
            int length = 0;
            do
            {
                size == 0 ? wlength + 2 : 2 * size;
                buffer.reset(new char[length]);
                length = WideCharToMultiByte(codePage, conversionFlags, wbuffer.get(), wlength, buffer.get(), size, 0, 0);
            }
            while(length == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

            if(!length)
            {
                throw IceUtil::IllegalConversionException(__FILE__, __LINE__, IceUtilInternal::lastErrorToString());;
            }
            out.assign(buffer.get(), length);
        }
    }
    return out;
}
#endif

}

Ice::LoggerI::LoggerI(const string& prefix, const string& file, 
                      bool convert, const IceUtil::StringConverterPtr& converter) :
    _convert(convert),
    _converter(converter)
{
    if(!prefix.empty())
    {
        _prefix = prefix + ": ";
    }

    if(!file.empty())
    {
        _file = file;
        _out.open(file, fstream::out | fstream::app);
        if(!_out.is_open())
        {
            throw InitializationException(__FILE__, __LINE__, "FileLogger: cannot open " + _file);
        }
    }
}


Ice::LoggerI::~LoggerI()
{
    if(_out.is_open())
    {
        _out.close();
    }
}

void
Ice::LoggerI::print(const string& message)
{
    write(message, false);
}

void
Ice::LoggerI::trace(const string& category, const string& message)
{
    string s = "-- " + IceUtil::Time::now().toDateTime() + " " + _prefix;
    if(!category.empty())
    {
        s += category + ": ";
    }
    s += message;

    write(s, true);
}

void
Ice::LoggerI::warning(const string& message)
{
    write("-! " + IceUtil::Time::now().toDateTime() + " " + _prefix + "warning: " + message, true);
}

void
Ice::LoggerI::error(const string& message)
{
    write("!! " + IceUtil::Time::now().toDateTime() + " " + _prefix + "error: " + message, true);
}

LoggerPtr
Ice::LoggerI::cloneWithPrefix(const std::string& prefix)
{
    return new LoggerI(prefix, _file, _convert, _converter);
}

void
Ice::LoggerI::write(const string& message, bool indent)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(outputMutex);

    string s = message;

    if(indent)
    {
        string::size_type idx = 0;
        while((idx = s.find("\n", idx)) != string::npos)
        {
            s.insert(idx + 1, "   ");
            ++idx;
        }
    }

    if(_out.is_open())
    {
        _out << s << endl;
    }
    else
    {
#if !defined(_WIN32) || defined(ICE_OS_WINRT)
        cerr << s << endl;
#else
        //
        // Convert the message from the native narrow string encoding to the console
        // code page encoding for printing. If the _convert member is set to false 
        // we don't do any conversion.
        //
        if(!_convert)
        {
            cerr << s << endl;
        }
        else
        {
            try
            {
                //
                // First we convert the message to UTF8 using nativeToUTF8
                // then we convert the message to the console code page
                // using UTF8ToCodePage.
                //
                // nativeToUTF8 doesn't do any conversion if the converter is
                // null likewise UTF8ToCodePage doesn't do any conversion if
                // the code page is UTF8.
                //
                // We cannot use cerr here as writing to console using cerr
                // will do its own conversion and will corrupt the messages.
                //
               fprintf_s(stderr, "%s\n", UTF8ToCodePage(IceUtil::nativeToUTF8(_converter, s), 
                                                        GetConsoleOutputCP()).c_str());
            }
            catch(const IceUtil::IllegalConversionException&)
            {
                //
                // If there is a problem with the encoding conversions we just
                // write the original message without encoding conversions.
                //
                fprintf_s(stderr, "%s\n", s.c_str());
            }
        }
#endif
    }
}
