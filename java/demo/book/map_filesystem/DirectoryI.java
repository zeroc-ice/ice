// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;
import FilesystemDB.*;

public class DirectoryI extends _DirectoryDisp
{
    public
    DirectoryI(Ice.Communicator communicator, String envName)
    {
        _communicator = communicator;
        _envName = envName;

        Freeze.Connection connection = Freeze.Util.createConnection(_communicator, _envName);
        try
        {
            IdentityDirectoryEntryMap dirDB = new IdentityDirectoryEntryMap(connection, directoriesDB());

            //
            // Create the record for the root directory if necessary.
            //
            for(;;)
            {
                try
                {
                    Ice.Identity rootId = new Ice.Identity("RootDir", "");
                    DirectoryEntry entry = dirDB.get(rootId);
                    if(entry == null)
                    {
                        dirDB.put(rootId, new DirectoryEntry("/", new Ice.Identity("", ""), null));
                    }
                    break;
                }
                catch(Freeze.DeadlockException ex)
                {
                    continue;
                }
                catch(Freeze.DatabaseException ex)
                {
                    halt(ex);
                }
            }
        }
        finally
        {
            connection.close();
        }
    }

    public String
    name(Ice.Current c)
    {
        Freeze.Connection connection = Freeze.Util.createConnection(_communicator, _envName);
        try
        {
            IdentityDirectoryEntryMap dirDB = new IdentityDirectoryEntryMap(connection, directoriesDB());

            for(;;)
            {
                try
                {
                    DirectoryEntry entry = dirDB.get(c.id);
                    if(entry == null)
                    {
                        throw new Ice.ObjectNotExistException();
                    }
                    return entry.name;
                }
                catch(Freeze.DeadlockException ex)
                {
                    continue;
                }
                catch(Freeze.DatabaseException ex)
                {
                    halt(ex);
                }
            }
        }
        finally
        {
            connection.close();
        }
    }

    public NodeDesc[]
    list(Ice.Current c)
    {
        Freeze.Connection connection = Freeze.Util.createConnection(_communicator, _envName);
        try
        {
            IdentityDirectoryEntryMap dirDB = new IdentityDirectoryEntryMap(connection, directoriesDB());

            for(;;)
            {
                try
                {
                    DirectoryEntry entry = dirDB.get(c.id);
                    if(entry == null)
                    {
                        throw new Ice.ObjectNotExistException();
                    }
                    NodeDesc[] result = new NodeDesc[entry.nodes.size()];
                    java.util.Iterator<NodeDesc> p = entry.nodes.values().iterator();
                    for(int i = 0; i < entry.nodes.size(); ++i)
                    {
                        result[i] = p.next();
                    }
                    return result;
                }
                catch(Freeze.DeadlockException ex)
                {
                    continue;
                }
                catch(Freeze.DatabaseException ex)
                {
                    halt(ex);
                }
            }
        }
        finally
        {
            connection.close();
        }
    }

    public NodeDesc
    find(String name, Ice.Current c)
        throws NoSuchName
    {
        Freeze.Connection connection = Freeze.Util.createConnection(_communicator, _envName);
        try
        {
            IdentityDirectoryEntryMap dirDB = new IdentityDirectoryEntryMap(connection, directoriesDB());

            for(;;)
            {
                try
                {
                    DirectoryEntry entry = dirDB.get(c.id);
                    if(entry == null)
                    {
                        throw new Ice.ObjectNotExistException();
                    }
                    NodeDesc nd = entry.nodes.get(name);
                    if(nd == null)
                    {
                        throw new NoSuchName(name);
                    }
                    return nd;
                }
                catch(Freeze.DeadlockException ex)
                {
                    continue;
                }
                catch(Freeze.DatabaseException ex)
                {
                    halt(ex);
                }
            }
        }
        finally
        {
            connection.close();
        }
    }

    public FilePrx
    createFile(String name, Ice.Current c)
        throws NameInUse
    {
        Freeze.Connection connection = Freeze.Util.createConnection(_communicator, _envName);
        try
        {
            IdentityFileEntryMap fileDB = new IdentityFileEntryMap(connection, FileI.filesDB());
            IdentityDirectoryEntryMap dirDB = new IdentityDirectoryEntryMap(connection, directoriesDB());

            for(;;)
            {
                //
                // The transaction is necessary since we are altering
                // two records in one atomic action.
                //
                Freeze.Transaction txn = null;
                try
                {
                    txn = connection.beginTransaction();
                    DirectoryEntry entry = dirDB.get(c.id);
                    if(entry == null)
                    {
                        throw new Ice.ObjectNotExistException();
                    }
                    if(name.length() == 0 || entry.nodes.get(name) != null)
                    {
                        throw new NameInUse(name);
                    }

                    FileEntry newEntry = new FileEntry(name, c.id, null);
                    Ice.Identity id = new Ice.Identity(java.util.UUID.randomUUID().toString(), "file");
                    FilePrx proxy = FilePrxHelper.uncheckedCast(c.adapter.createProxy(id));

                    entry.nodes.put(name, new NodeDesc(name, NodeType.FileType, proxy));
                    dirDB.put(c.id, entry);

                    fileDB.put(id, newEntry);

                    txn.commit();
                    txn = null;

                    return proxy;
                }
                catch(Freeze.DeadlockException ex)
                {
                    continue;
                }
                catch(Freeze.DatabaseException ex)
                {
                    halt(ex);
                }
                finally
                {
                    if(txn != null)
                    {
                        txn.rollback();
                    }
                }
            }
        }
        finally
        {
            connection.close();
        }
    }

    public DirectoryPrx
    createDirectory(String name, Ice.Current c)
        throws NameInUse
    {
        Freeze.Connection connection = Freeze.Util.createConnection(_communicator, _envName);
        try
        {
            IdentityDirectoryEntryMap dirDB = new IdentityDirectoryEntryMap(connection, directoriesDB());

            for(;;)
            {
                //
                // The transaction is necessary since we are altering
                // two records in one atomic action.
                //
                Freeze.Transaction txn = null;
                try
                {
                    txn = connection.beginTransaction();
                    DirectoryEntry entry = dirDB.get(c.id);
                    if(entry == null)
                    {
                        throw new Ice.ObjectNotExistException();
                    }
                    if(name.length() == 0 || entry.nodes.get(name) != null)
                    {
                        throw new NameInUse(name);
                    }

                    DirectoryEntry newEntry = new DirectoryEntry(name, c.id, null);
                    Ice.Identity id = new Ice.Identity(java.util.UUID.randomUUID().toString(), "");
                    DirectoryPrx proxy = DirectoryPrxHelper.uncheckedCast(c.adapter.createProxy(id));

                    entry.nodes.put(name, new NodeDesc(name, NodeType.DirType, proxy));
                    dirDB.put(c.id, entry);

                    dirDB.put(id, newEntry);

                    txn.commit();
                    txn = null;

                    return proxy;
                }
                catch(Freeze.DeadlockException ex)
                {
                    continue;
                }
                catch(Freeze.DatabaseException ex)
                {
                    halt(ex);
                }
                finally
                {
                    if(txn != null)
                    {
                        txn.rollback();
                    }
                }
            }
        }
        finally
        {
            connection.close();
        }
    }

    public void
    destroy(Ice.Current c)
        throws PermissionDenied
    {
        Freeze.Connection connection = Freeze.Util.createConnection(_communicator, _envName);
        try
        {
            IdentityDirectoryEntryMap dirDB = new IdentityDirectoryEntryMap(connection, directoriesDB());

            for(;;)
            {
                //
                // The transaction is necessary since we are altering
                // two records in one atomic action.
                //
                Freeze.Transaction txn = null;
                try
                {
                    txn = connection.beginTransaction();
                    DirectoryEntry entry = dirDB.get(c.id);
                    if(entry == null)
                    {
                        throw new Ice.ObjectNotExistException();
                    }
                    if(entry.parent.name.length() == 0)
                    {
                        throw new PermissionDenied("Cannot destroy root directory");
                    }
                    if(!entry.nodes.isEmpty())
                    {
                        throw new PermissionDenied("Cannot destroy non-empty directory");
                    }

                    DirectoryEntry dirEntry = dirDB.get(entry.parent);
                    if(dirEntry == null)
                    {
                        halt(new Freeze.DatabaseException("consistency error: directory without parent"));
                    }

                    dirEntry.nodes.remove(entry.name);
                    dirDB.put(entry.parent, dirEntry);

                    dirDB.remove(c.id);

                    txn.commit();
                    txn = null;
                    break;
                }
                catch(Freeze.DeadlockException ex)
                {
                    continue;
                }
                catch(Freeze.DatabaseException ex)
                {
                    halt(ex);
                }
                finally
                {
                    if(txn != null)
                    {
                        txn.rollback();
                    }
                }
            }
        }
        finally
        {
            connection.close();
        }
    }

    private void
    halt(Freeze.DatabaseException e)
    {
        //
        // If this is called it's very bad news. We log the error and
        // then kill the server.
        //
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        e.printStackTrace(pw);
        pw.flush();
        _communicator.getLogger().error("fatal database error\n" + sw.toString() + "\n*** Halting JVM ***");
        Runtime.getRuntime().halt(1);
    }

    public static String
    directoriesDB()
    {
        return "directories";
    }

    private Ice.Communicator _communicator;
    private String _envName;
}
