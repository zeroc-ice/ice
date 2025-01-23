// Copyright (c) ZeroC, Inc.

#include "LoggerI.h"
#include "ConsoleUtil.h"
#include "FileUtil.h"
#include "Ice/LocalExceptions.h"
#include "Ice/StringUtil.h"
#include "TimeUtil.h"

#include <iostream>
#include <mutex>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    mutex outputMutex;

    // Timeout after which rename will be attempted in case of failures renaming files. That is set to 5 minutes.
    const chrono::minutes retryTimeout = chrono::minutes(5); // NOLINT(cert-err58-cpp)
}

Ice::Logger::~Logger() = default; // avoid weak vtable

Ice::LoggerI::LoggerI(string prefix, string file, bool convert, size_t sizeMax)
    : _prefix(std::move(prefix)),
      _convert(convert),
      _converter(getProcessStringConverter()),
      _file(std::move(file)),
      _sizeMax(sizeMax)
{
    if (!_prefix.empty())
    {
        _formattedPrefix = _prefix + ": ";
    }

    if (!_file.empty())
    {
        _out.open(IceInternal::streamFilename(_file).c_str(), fstream::out | fstream::app);
        if (!_out.is_open())
        {
            throw InitializationException(__FILE__, __LINE__, "FileLogger: cannot open " + _file);
        }

        if (_sizeMax > 0)
        {
            _out.seekp(0, _out.end);
        }
    }
}

Ice::LoggerI::~LoggerI()
{
    if (_out.is_open())
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
    string s = "-- " + timePointToDateTimeString(chrono::system_clock::now()) + " " + _formattedPrefix;
    if (!category.empty())
    {
        s += category + ": ";
    }
    s += message;

    write(s, true);
}

void
Ice::LoggerI::warning(const string& message)
{
    write(
        "-! " + timePointToDateTimeString(chrono::system_clock::now()) + " " + _formattedPrefix + "warning: " + message,
        true);
}

void
Ice::LoggerI::error(const string& message)
{
    write(
        "!! " + timePointToDateTimeString(chrono::system_clock::now()) + " " + _formattedPrefix + "error: " + message,
        true);
}

string
Ice::LoggerI::getPrefix()
{
    return _prefix;
}

LoggerPtr
Ice::LoggerI::cloneWithPrefix(std::string prefix)
{
    lock_guard lock(outputMutex); // for _sizeMax
    return make_shared<LoggerI>(std::move(prefix), _file, _convert, _sizeMax);
}

void
Ice::LoggerI::write(const string& message, bool indent)
{
    unique_lock lock(outputMutex);
    string s = message;

    if (indent)
    {
        string::size_type idx = 0;
        while ((idx = s.find('\n', idx)) != string::npos)
        {
            s.insert(idx + 1, "   ");
            ++idx;
        }
    }

    if (_out.is_open())
    {
        if (_sizeMax > 0)
        {
            //
            // If file size + message size exceeds max size we archive the log file,
            // but we do not archive empty files or truncate messages.
            //
            size_t sz = static_cast<size_t>(_out.tellp());
            if (sz > 0 && sz + message.size() >= _sizeMax && _nextRetry <= chrono::steady_clock::now())
            {
                string basename = _file;
                string ext;

                size_t i = basename.rfind('.');
                if (i != string::npos && i + 1 < basename.size())
                {
                    ext = basename.substr(i + 1);
                    basename = basename.substr(0, i);
                }
                _out.close();

                int id = 0;
                string archive;
                string date = timePointToString(chrono::system_clock::now(), "%Y%m%d-%H%M%S");
                while (true)
                {
                    ostringstream oss;
                    oss << basename << "-" << date;
                    if (id > 0)
                    {
                        oss << "-" << id;
                    }
                    if (!ext.empty())
                    {
                        oss << "." << ext;
                    }
                    if (IceInternal::fileExists(oss.str()))
                    {
                        id++;
                        continue;
                    }
                    archive = oss.str();
                    break;
                }

                int err = IceInternal::rename(_file, archive);

                _out.open(IceInternal::streamFilename(_file).c_str(), fstream::out | fstream::app);

                if (err)
                {
                    _nextRetry = chrono::steady_clock::now() + retryTimeout;

                    //
                    // We temporarily set the maximum size to 0 to ensure there isn't more rename attempts
                    // in the nested error call.
                    //
                    size_t sizeMax = _sizeMax;
                    _sizeMax = 0;
                    lock.unlock();
                    error("FileLogger: cannot rename '" + _file + "'\n" + IceInternal::lastErrorToString());
                    lock.lock();
                    _sizeMax = sizeMax;
                }
                else
                {
                    _nextRetry = chrono::steady_clock::time_point();
                }

                if (!_out.is_open())
                {
                    lock.unlock();
                    error("FileLogger: cannot open '" + _file + "':\nlog messages will be sent to stderr");
                    write(message, indent);
                    return;
                }
            }
        }
        _out << s << endl;
    }
    else
    {
#if defined(_WIN32)
        //
        // Convert the message from the native narrow string encoding to the console
        // code page encoding for printing. If the _convert member is set to false
        // we don't do any conversion.
        //
        if (!_convert)
        {
            //
            // Use fprintf_s to avoid encoding conversion when stderr is connected
            // to Windows console. When _convert is set to false we always output
            // UTF-8 encoded messages.
            //
            fprintf_s(stderr, "%s\n", nativeToUTF8(s, _converter).c_str());
            fflush(stderr);
        }
        else
        {
            consoleErr << s << endl;
        }
#else
        cerr << s << endl;
#endif
    }
}
