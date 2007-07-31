// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using Ice;
using Filesystem;

namespace FilesystemI
{
    public class DirectoryI : DirectoryDisp_, NodeI, DirectoryOperations_
    {
        // Slice name() operation.

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

        // Return the object identity for this node.

        public virtual Identity id()
        {
            return _id;
        }

        // Slice list() operation.

        public override NodeDesc[] list(Current c)
        {
            lock(this)
            {
                if(_destroyed)
                {
                    throw new ObjectNotExistException();
                }
            }

            lock(_lcMutex)
            {
                reap();

                NodeDesc[] ret = new NodeDesc[_contents.Count];
                int i = 0;
                foreach(DictionaryEntry e in _contents)
                {
                    NodeI p = (NodeI)e.Value;
                    ret[i] = new NodeDesc();
                    ret[i].name = (string)e.Key;
                    ret[i].type = p is FileI ? NodeType.FileType : NodeType.DirType;
                    ret[i].proxy = NodePrxHelper.uncheckedCast(c.adapter.createProxy(p.id()));
                    ++i;
                }
                return ret;
            }
        }

        // Slice find() operation.

        public override NodeDesc find(string name, Current c)
        {
            lock(this)
            {
                if(_destroyed)
                {
                    throw new ObjectNotExistException();
                }
            }

            lock(_lcMutex)
            {
                reap();

                NodeI p = (NodeI) _contents[name];
                if(p == null)
                {
                    throw new NoSuchName(name);
                }

                NodeDesc d = new NodeDesc();
                d.name = name;
                d.type = p is FileI?NodeType.FileType:NodeType.DirType;
                d.proxy = NodePrxHelper.uncheckedCast(c.adapter.createProxy(p.id()));
                return d;
            }
        }

        // Slice createFile() operation.

        public override FilePrx createFile(string name, Current c)
        {
            lock(this)
            {
                if(_destroyed)
                {
                    throw new ObjectNotExistException();
                }
            }

            lock(_lcMutex)
            {
                reap();

                if(_contents.Contains(name))
                {
                    throw new NameInUse(name);
                }
                FileI f = new FileI(c.adapter, name, this);
                return FilePrxHelper.uncheckedCast(c.adapter.createProxy(f.id()));
            }
        }

        // Slice createDirectory() operation.

        public override DirectoryPrx createDirectory(string name, Current c)
        {
            lock(this)
            {
                if(_destroyed)
                {
                    throw new ObjectNotExistException();
                }
            }

            lock(_lcMutex)
            {
                reap();

                if(_contents.Contains(name))
                {
                    throw new NameInUse(name);
                }
                DirectoryI d = new DirectoryI(c.adapter, name, this);
                return DirectoryPrxHelper.uncheckedCast(c.adapter.createProxy(d.id()));
            }
        }

        // Slice destroy() operation.

        public override void destroy(Current c)
        {
            if(_parent == null)
            {
                throw new PermissionDenied("Cannot destroy root directory");
            }

            lock(this)
            {
                if(_destroyed)
                {
                    throw new ObjectNotExistException();
                }

                lock(_lcMutex)
                {
                    reap();

                    if(_contents.Count != 0)
                    {
                        throw new PermissionDenied("Cannot destroy non-empty directory");
                    }

                    c.adapter.remove(id());
                    _parent.addReapEntry(_name);
                    _destroyed = true;
                }
            }
        }

        // DirectoryI constructor for root directory.

        public DirectoryI(ObjectAdapter a)
            : this(a, "RootDir", null)
        {
        }

        // DirectoryI constructor. parent == null indicates root directory.

        public DirectoryI(ObjectAdapter a, string name, DirectoryI parent)
        {
            _lcMutex = new System.Object();
            _name = name;
            _parent = parent;
            _id = new Identity();
            _destroyed = false;
            _contents = new Hashtable();

            if(parent == null)
            {
                _id.name = name;
            }
            else
            {
                _id.name = Util.generateUUID();
                _parent.addChild(name, this);
            }
            a.add(this, _id);
        }

        // Add the name-node pair to the _contents map.

        public virtual void addChild(string name, NodeI node)
        {
            _contents[name] = node;
        }

        // Add this directory and the name of a deleted entry to the reap map.

        public virtual void addReapEntry(string name)
        {
            IList l = (IList)_reapMap[this];
            if(l != null)
            {
                l.Add(name);
            }
            else
            {
                ArrayList v = new ArrayList();
                v.Add(name);
                _reapMap[this] = v;
            }
        }

        // Remove all names in the reap map from the corresponding directory contents.

        private static void reap()
        {
            foreach(DictionaryEntry e in _reapMap)
            {
                DirectoryI dir = (DirectoryI)e.Key;
                IList v = (IList)e.Value;
                foreach(string name in v)
                {
                    dir._contents.Remove(name);
                }
            }
            _reapMap.Clear();
        }

        public static System.Object _lcMutex = new System.Object();

        private string _name; // Immutable
        private DirectoryI _parent; // Immutable
        private Identity _id; // Immutable
        private bool _destroyed;
        private IDictionary _contents;

        private static IDictionary _reapMap = new Hashtable();
    }
}
