// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.IO;
using System.Text;
using Filesystem;

class Parser
{
    virtual internal string Input
    {
        get
        {
            Console.Write("> ");
            Console.Out.Flush();

            try
            {
                return _in.ReadLine();
            }
            catch(IOException)
            {
                return null;
            }
        }

    }
    internal Parser(DirectoryPrx root)
    {
        _dirs = new ArrayList();
        _dirs.Insert(0, root);
    }

    internal virtual void usage()
    {
        Console.Error.Write(
            "help                    Print this message.\n" +
            "pwd                     Print current directory (/ = root).\n" +
            "cd [DIR]                Change directory (/ or empty = root).\n" +
            "ls                      List current directory.\n" +
            "lr                      Recursively list current directory.\n" +
            "mkdir DIR [DIR...]      Create directories DIR in current directory.\n" +
            "mkfile FILE [FILE...]   Create files FILE in current directory.\n" +
            "rm NAME [NAME...]       Delete directory or file NAME (rm * to delete all).\n" +
            "cat FILE                List the contents of FILE.\n" +
            "write FILE [STRING...]  Write STRING to FILE.\n" +
            "exit, quit              Exit this program.\n");
    }

    internal virtual void list(bool recursive)
    {
        try
        {
            list((DirectoryPrx)_dirs[0], recursive, 0);
        }
        catch(Ice.LocalException ex)
        {
            error(ex.ToString());
        }
    }

    internal virtual void list(Filesystem.DirectoryPrx dir, bool recursive, int depth)
    {
        StringBuilder b = new StringBuilder();
        for(int i = 0; i < depth; ++i)
        {
            b.Append('\t');
        }
        string indent = b.ToString();

        NodeDesc[] contents = dir.list();

        for(int i = 0; i < contents.Length; ++i)
        {
            DirectoryPrx d = contents[i].type == NodeType.DirType
                                ? DirectoryPrxHelper.uncheckedCast(contents[i].proxy) : null;
            Console.Write(indent + contents[i].name + (d != null ? " (directory)" : " (file)"));
            if(d != null && recursive)
            {
                Console.WriteLine(":");
                list(d, true, ++depth);
            }
            else
            {
                Console.WriteLine();
            }
        }
    }

    internal virtual void createFile(IList names)
    {
        DirectoryPrx dir = (DirectoryPrx)_dirs[0];

        foreach(string name in names)
        {
            if(name.Equals(".."))
            {
                Console.WriteLine("Cannot create a file named `..'");
                continue;
            }

            try
            {
                dir.createFile(name);
            }
            catch(NameInUse)
            {
                Console.WriteLine("`" + name + "' exists already");
            }
        }
    }

    internal virtual void createDir(IList names)
    {
        DirectoryPrx dir = (DirectoryPrx)_dirs[0];

        foreach(string name in names)
        {
            if(name.Equals(".."))
            {
                Console.WriteLine("Cannot create a directory named `..'");
                continue;
            }

            try
            {
                dir.createDirectory(name);
            }
            catch(NameInUse)
            {
                Console.WriteLine("`" + name + "' exists already");
            }
        }
    }

    internal virtual void pwd()
    {
        if(_dirs.Count == 1)
        {
            Console.Write("/");
        }
        else
        {
            for(int i = _dirs.Count - 2; i >= 0; --i)
            {
                Console.Write("/" + ((DirectoryPrx)_dirs[i]).name());
            }
        }
        Console.WriteLine();
    }

    internal virtual void cd(string name)
    {
        if(name.Equals("/"))
        {
            while(_dirs.Count > 1)
            {
                _dirs.RemoveAt(0);
            }
            return;
        }

        if(name.Equals(".."))
        {
            if(_dirs.Count > 1)
            {
                _dirs.RemoveAt(0);
            }
            return;
        }

        DirectoryPrx dir = (DirectoryPrx)_dirs[0];
        NodeDesc d;
        try
        {
            d = dir.find(name);
        }
        catch(NoSuchName)
        {
            Console.WriteLine("`" + name + "': no such directory");
            return;
        }
        if(d.type == NodeType.FileType)
        {
            Console.WriteLine("`" + name + "': not a directory");
            return;
        }
        _dirs.Insert(0, DirectoryPrxHelper.uncheckedCast(d.proxy));
    }

    internal virtual void cat(string name)
    {
        DirectoryPrx dir = (DirectoryPrx)_dirs[0];
        NodeDesc d;
        try
        {
            d = dir.find(name);
        }
        catch(NoSuchName)
        {
            Console.WriteLine("`" + name + "': no such file");
            return;
        }
        if(d.type == NodeType.DirType)
        {
            Console.WriteLine("`" + name + "': not a file");
            return;
        }
        FilePrx f = FilePrxHelper.uncheckedCast(d.proxy);
        string[] l = f.read();
        for(int i = 0; i < l.Length; ++i)
        {
            Console.WriteLine(l[i]);
        }
    }

    internal virtual void write(IList args)
    {
        DirectoryPrx dir = (DirectoryPrx)_dirs[0];
        string name = (string)args[0];
        args.RemoveAt(0);
        NodeDesc d;
        try
        {
            d = dir.find(name);
        }
        catch(NoSuchName)
        {
            Console.WriteLine("`" + name + "': no such file");
            return;
        }
        if(d.type == NodeType.DirType)
        {
            Console.WriteLine("`" + name + "': not a file");
            return;
        }
        FilePrx f = FilePrxHelper.uncheckedCast(d.proxy);

        string[] l = new string[args.Count];
        args.CopyTo(l, 0);
        try
        {
            f.write(l);
        }
        catch(GenericError ex)
        {
            Console.WriteLine("`" + name + "': cannot write to file: " + ex.reason);
        }
    }

    internal virtual void destroy(IList names)
    {
        DirectoryPrx dir = (DirectoryPrx)_dirs[0];

        foreach(string name in names)
        {
            if(name.Equals("*"))
            {
                NodeDesc[] nodes = dir.list();
                for(int j = 0; j < nodes.Length; ++j)
                {
                    try
                    {
                        nodes[j].proxy.destroy();
                    }
                    catch(PermissionDenied ex)
                    {
                        Console.WriteLine("cannot remove `" + nodes[j].name + "': " + ex.reason);
                    }
                }
                return;
            }
            else
            {
                NodeDesc d;
                try
                {
                    d = dir.find(name);
                }
                catch(NoSuchName)
                {
                    Console.WriteLine("`" + name + "': no such file or directory");
                    return;
                }
                try
                {
                    d.proxy.destroy();
                }
                catch(PermissionDenied ex)
                {
                    Console.WriteLine("cannot remove `" + name + "': " + ex.reason);
                }
            }
        }
    }

    internal virtual void error(string s)
    {
        Console.Error.WriteLine("error: " + s);
    }

    internal virtual void warning(string s)
    {
        Console.Error.WriteLine("warning: " + s);
    }

    internal virtual int parse()
    {
#if COMPACT
        _in = Console.In;
#else
        _in = new StreamReader(new StreamReader(Console.OpenStandardInput(), Encoding.Default).BaseStream,
                               new StreamReader(Console.OpenStandardInput(), Encoding.Default).CurrentEncoding);
#endif

        Grammar g = new Grammar(this);
        g.parse();

        return 0;
    }

    private ArrayList _dirs;

    private TextReader _in;
}
