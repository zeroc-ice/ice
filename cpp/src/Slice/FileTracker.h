// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef FILE_TRACKER_H
#define FILE_TRACKER_H

#include <IceUtil/Shared.h>
#include <Slice/Parser.h>

namespace Slice
{

class FileException : public ::IceUtil::Exception
{
public:

    FileException(const char*, int, const std::string&);
#ifndef ICE_CPP11_COMPILER
    ~FileException() throw();
#endif
    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;
#ifndef ICE_CPP11_MAPPING
    virtual FileException* ice_clone() const;
#endif
    virtual void ice_throw() const;

    std::string reason() const;

private:

    static const char* _name;
    const std::string _reason;
};

class FileTracker;
typedef IceUtil::Handle<FileTracker> FileTrackerPtr;

class FileTracker : public ::IceUtil::SimpleShared
{
public:

    FileTracker();
    ~FileTracker();

    static FileTrackerPtr instance();

    void setSource(const std::string&);
    void addFile(const std::string&);
    void addDirectory(const std::string&);
    void error();
    void cleanup();
    void dumpxml();

private:

    std::list<std::pair< std::string, bool> > _files;
    std::string _source;
    std::map<std::string, std::list<std::string> > _generated;
    std::map<std::string, std::list<std::string> >::iterator _curr;
};

}

#endif
