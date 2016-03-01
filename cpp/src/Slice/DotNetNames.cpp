// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Slice/DotNetNames.h>
#include <ctype.h>
#include <cstring>

using namespace std;

namespace Slice
{

namespace DotNet
{

struct Node
{
    const char** names;
    const Node** parents;
};

static const char* ObjectNames[] =
                    {
                        "Equals", "Finalize", "GetHashCode", "GetType",
                        "MemberwiseClone", "ReferenceEquals", "ToString", 0
                    };
static const Node* ObjectParents[] =
                    {
                        0
                    };
static const Node ObjectNode =
                    {
                        ObjectNames, &ObjectParents[0]
                    };

static const char* ICloneableNames[] =
                    {
                        "Clone", 0
                    };
static const Node* ICloneableParents[] =
                    {
                        &ObjectNode, 0
                    };
static const Node ICloneableNode =
                    {
                        ICloneableNames, &ICloneableParents[0]
                    };

static const char* ExceptionNames[] =
                    {
                        "Data", "GetBaseException", "GetObjectData", "HelpLink", "HResult", "InnerException",
                        "Message", "Source", "StackTrace", "TargetSite", 0
                    };
static const Node* ExceptionParents[] =
                    {
                        &ObjectNode, 0
                    };
static const Node ExceptionNode =
                    {
                        ExceptionNames, &ExceptionParents[0]
                    };

//
// Must be kept in same order as definition of BaseType in header file!
//
static const Node* nodes[] =
                    {
                        &ObjectNode, &ICloneableNode, &ExceptionNode
                    };

static bool
ciEquals(const string& s, const char* p)
{
    if(s.size() != strlen(p))
    {
        return false;
    }
    string::const_iterator i = s.begin();
    while(i != s.end())
    {
        if(tolower(static_cast<unsigned char>(*i++)) != tolower(static_cast<unsigned char>(*p++)))
        {
            return false;
        }
    }
    return true;
}

const char* manglePrefix = "ice_";
const char* mangleSuffix = "_";

static bool
mangle(const string& s, const Node* np, string& newName)
{
    const char** namep = np->names;
    while(*namep)
    {
        if(ciEquals(s, *namep))
        {
            newName = manglePrefix + s + mangleSuffix;
            return true;
        }
        ++namep;
    }
    const Node** parentp = np->parents;
    while(*parentp)
    {
        if(mangle(s, *parentp, newName))
        {
            return true;
        }
        ++parentp;
    }
    return false;
}

}

}

string
Slice::DotNet::mangleName(const string& s, int baseTypes)
{
    if(baseTypes == 0)
    {
        return s;
    }
    string newName;
    for(unsigned int mask = 1, i=0; mask < END; mask <<= 1, ++i)
    {
        if(baseTypes & mask)
        {
            if(mangle(s, nodes[i], newName))
            {
                return newName;
            }
        }
    }
    return s;
}
