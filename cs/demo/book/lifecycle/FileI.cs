// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Ice;
using Filesystem;
using FilesystemI;

namespace FilesystemI
{
    public class FileI : FileDisp_, NodeI, FileOperations_
    {
        public override string name(Current c)
        {
            lock(this)
            {
                if(_destroyed)
                {
                    throw new ObjectNotExistException();
                }
                return _name;
            }
        }

        public virtual Identity id()
        {
            return _id;
        }

        public override string[] read(Current c)
        {
            lock(this)
            {
                if(_destroyed)
                {
                    throw new ObjectNotExistException();
                }

                return _lines;
            }
        }

        public override void write(string[] text, Current c)
        {
            lock(this)
            {
                if(_destroyed)
                {
                    throw new ObjectNotExistException();
                }

                _lines = new string[text.Length];
                text.CopyTo((string[])_lines, 0);
            }
        }

        public override void destroy(Current c)
        {
            lock(this)
            {
                if(_destroyed)
                {
                    throw new ObjectNotExistException();
                }
                _destroyed = true;
            }

            lock(DirectoryI._lcMutex)
            {
                c.adapter.remove(id());
                _parent.addReapEntry(_name);
            }
        }

        public FileI(ObjectAdapter a, string name, DirectoryI parent)
        {
            _name = name;
            _parent = parent;
            _destroyed = false;
            _id = new Identity();
            _id.name = Util.generateUUID();
            _parent.addChild(name, this);
            a.add(this, _id);
        }

        private string _name;
        private DirectoryI _parent;
        private bool _destroyed;
        private Identity _id;
        private string[] _lines;
    }
}
