// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FILE_TRACKER_H
#define FILE_TRACKER_H

#include <IceUtil/Shared.h>
#include <Slice/Parser.h>

namespace Slice
{

class SLICE_API FileException : public ::IceUtil::Exception
{
public:

    FileException(const char*, int, const std::string&);
    ~FileException() throw();
    virtual std::string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual ::IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason() const;

private:

    static const char* _name;
    const std::string _reason;
};

class FileTracker;
typedef IceUtil::Handle<FileTracker> FileTrackerPtr;

class SLICE_API FileTracker : public ::IceUtil::SimpleShared
{
public:

    FileTracker();
    ~FileTracker();

    static FileTrackerPtr instance();

    void addFile(const std::string&);
    void addDirectory(const std::string&);

    void cleanup();

private:

    std::list<std::pair< std::string, bool> > _files;
};

}

#endif
