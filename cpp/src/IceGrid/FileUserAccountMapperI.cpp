// Copyright (c) ZeroC, Inc.

#include "FileUserAccountMapperI.h"
#include "../Ice/FileUtil.h"
#include "Ice/StringUtil.h"
#include <fstream>

using namespace std;
using namespace IceGrid;

FileUserAccountMapperI::FileUserAccountMapperI(const string& filename)
{
    ifstream file(IceInternal::streamFilename(filename).c_str()); // filename is a UTF-8 string
    if (!file)
    {
        throw runtime_error("cannot open '" + filename + "' for reading: " + IceInternal::errorToString(errno));
    }

    const string delim = " \t\r\n";
    while (true)
    {
        string line;
        getline(file, line);
        if (!file)
        {
            break;
        }

        string::size_type idx = line.find('#');
        if (idx != string::npos)
        {
            line.erase(idx);
        }

        idx = line.find_last_not_of(delim);
        if (idx != string::npos && idx + 1 < line.length())
        {
            line.erase(idx + 1);
        }

        string::size_type beg = line.find_first_not_of(delim);
        if (beg == string::npos)
        {
            continue;
        }

        string::size_type end = line.find_first_of(delim, beg);
        if (end == string::npos || end <= beg)
        {
            continue;
        }
        string account = line.substr(beg, end - beg);

        beg = line.find_first_not_of(delim, end);
        if (beg == string::npos)
        {
            continue;
        }
        string user = line.substr(beg);

        assert(!user.empty());
        assert(!account.empty());

        _accounts[user] = account;
    }
}

string
FileUserAccountMapperI::getUserAccount(string user, const Ice::Current&)
{
    auto p = _accounts.find(user);
    if (p == _accounts.end())
    {
        throw UserAccountNotFoundException();
    }
    return p->second;
}
