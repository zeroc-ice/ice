// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;
import FilesystemDB.*;

public class FileI extends _FileDisp
{
    public
    FileI(Ice.Communicator communicator, String envName)
    {
        _communicator = communicator;
        _envName = envName;
    }

    public String
    name(Ice.Current c)
    {
        Freeze.Connection connection = Freeze.Util.createConnection(_communicator, _envName);
        try
        {
            IdentityFileEntryMap fileDB = new IdentityFileEntryMap(connection, filesDB());

            for(;;)
            {
                try
                {
                    FileEntry entry = fileDB.get(c.id);
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

    public String[]
    read(Ice.Current c)
    {
        Freeze.Connection connection = Freeze.Util.createConnection(_communicator, _envName);
        try
        {
            IdentityFileEntryMap fileDB = new IdentityFileEntryMap(connection, filesDB());

            for(;;)
            {
                try
                {
                    FileEntry entry = fileDB.get(c.id);
                    if(entry == null)
                    {
                        throw new Ice.ObjectNotExistException();
                    }
                    return entry.text;
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

    public void
    write(String[] text, Ice.Current c)
        throws GenericError
    {
        Freeze.Connection connection = Freeze.Util.createConnection(_communicator, _envName);
        try
        {
            IdentityFileEntryMap fileDB = new IdentityFileEntryMap(connection, filesDB());

            for(;;)
            {
                try
                {
                    FileEntry entry = fileDB.get(c.id);
                    if(entry == null)
                    {
                        throw new Ice.ObjectNotExistException();
                    }
                    entry.text = text;
                    fileDB.put(c.id, entry);
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

    public void
    destroy(Ice.Current c)
        throws PermissionDenied
    {
        Freeze.Connection connection = Freeze.Util.createConnection(_communicator, _envName);
        try
        {
            IdentityFileEntryMap fileDB = new IdentityFileEntryMap(connection, filesDB());
            IdentityDirectoryEntryMap dirDB = new IdentityDirectoryEntryMap(connection, DirectoryI.directoriesDB());

            for(;;)
            {
                Freeze.Transaction txn = null;
                try
                {
                    //
                    // The transaction is necessary since we are
                    // altering two records in one atomic action.
                    //
                    txn = connection.beginTransaction();
                    FileEntry entry = fileDB.get(c.id);
                    if(entry == null)
                    {
                        throw new Ice.ObjectNotExistException();
                    }

                    DirectoryEntry dirEntry = (DirectoryEntry)dirDB.get(entry.parent);
                    if(dirEntry == null)
                    {
                        halt(new Freeze.DatabaseException("consistency error: file without parent"));
                    }

                    dirEntry.nodes.remove(entry.name);
                    dirDB.put(entry.parent, dirEntry);

                    fileDB.remove(c.id);

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

    public static String
    filesDB()
    {
        return "files";
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

    private Ice.Communicator _communicator;
    private String _envName;
}
