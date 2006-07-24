// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef __FilesystemI_h__
#define __FilesystemI_h__

#include <Ice/Ice.h>
#include <Filesystem.h>

namespace Filesystem {
    class DirectoryI;
    typedef IceUtil::Handle<DirectoryI> DirectoryIPtr;

    class NodeI : virtual public Node {
    public:
	virtual std::string name(const Ice::Current &) const;
	NodeI(const Ice::CommunicatorPtr &, const std::string &, const DirectoryIPtr & parent);
	static Ice::ObjectAdapterPtr _adapter;
    private:
	const std::string _name;
	DirectoryIPtr _parent;
    };

    class FileI : virtual public File,
		  virtual public NodeI {
    public:
	virtual Lines read(const Ice::Current &) const;
	virtual void write(const Lines &,
			   const Ice::Current &);
	FileI(const Ice::CommunicatorPtr &, const std::string &, const DirectoryIPtr &);
    private:
	Lines _lines;
    };

    class DirectoryI : virtual public Directory,
		       virtual public NodeI {
    public:
	virtual NodeSeq list(const Ice::Current &) const;
        DirectoryI(const Ice::CommunicatorPtr &, const std::string &, const DirectoryIPtr &);
	void addChild(NodePrx child);
    private:
	NodeSeq _contents;
    };
}

#endif
