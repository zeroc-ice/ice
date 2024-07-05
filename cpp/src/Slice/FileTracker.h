//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef FILE_TRACKER_H
#define FILE_TRACKER_H

#include "Parser.h"

namespace Slice
{
    class FileException final : public Ice::LocalException
    {
    public:
        using Ice::LocalException::LocalException;

        const char* ice_id() const noexcept override;
    };

    class FileTracker;
    using FileTrackerPtr = std::shared_ptr<FileTracker>;
    class FileTracker final
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
        std::list<std::pair<std::string, bool>> _files;
        std::string _source;
        std::map<std::string, std::list<std::string>> _generated;
        std::map<std::string, std::list<std::string>>::iterator _curr;
    };
}

#endif
