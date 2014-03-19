// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;

class Parser
{
    Parser(DirectoryPrx root)
    {
        _dirs = new java.util.LinkedList<DirectoryPrx>();
        _dirs.addFirst(root);
    }

    void
    usage()
    {
        System.err.print(
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

    void
    list(boolean recursive)
    {
        try
        {
            list(_dirs.get(0), recursive, 0);
        }
        catch(Ice.LocalException ex)
        {
            error(ex.toString());
        }
    }

    void
    list(Filesystem.DirectoryPrx dir, boolean recursive, int depth)
    {
        StringBuilder b = new StringBuilder();
        for(int i = 0; i < depth; ++i)
        {
            b.append('\t');
        }
        String indent = b.toString();

        NodeDesc[] contents = dir.list();

        for(int i = 0; i < contents.length; ++i)
        {
            DirectoryPrx d
                = contents[i].type == NodeType.DirType
                    ? DirectoryPrxHelper.uncheckedCast(contents[i].proxy)
                    : null;
            System.out.print(indent + contents[i].name + (d != null ? " (directory)" : " (file)"));
            if(d != null && recursive)
            {
                System.out.println(":");
                list(d, true, ++depth);
            }
            else
            {
                System.out.println();
            }
        }
    }

    void
    createFile(java.util.List<String> names)
    {
        DirectoryPrx dir = _dirs.getFirst();

        for(String name : names)
        {
            if(name.equals(".."))
            {
                System.out.println("Cannot create a file named `..'");
                continue;
            }

            try
            {
                dir.createFile(name);
            }
            catch(NameInUse ex)
            {
                System.out.println("`" + name + "' exists already");
            }
        }
    }

    void
    createDir(java.util.List<String> names)
    {
        DirectoryPrx dir = _dirs.getFirst();

        for(String name : names)
        {
            if(name.equals(".."))
            {
                System.out.println("Cannot create a directory named `..'");
                continue;
            }

            try
            {
                dir.createDirectory(name);
            }
            catch(NameInUse ex)
            {
                System.out.println("`" + name + "' exists already");
            }
        }
    }

    void
    pwd()
    {
        if(_dirs.size() == 1)
        {
            System.out.print("/");
        }
        else
        {
            java.util.ListIterator<DirectoryPrx> i = _dirs.listIterator(_dirs.size());
            i.previous();
            while(i.hasPrevious())
            {
                System.out.print("/" + i.previous().name());
            }
        }
        System.out.println();
    }

    void
    cd(String name)
    {
        if(name.equals("/"))
        {
            while(_dirs.size() > 1)
            {
                _dirs.removeFirst();
            }
            return;
        }

        if(name.equals(".."))
        {
            if(_dirs.size() > 1)
            {
                _dirs.removeFirst();
            }
            return;
        }

        DirectoryPrx dir = _dirs.getFirst();
        NodeDesc d;
        try
        {
            d = dir.find(name);
        }
        catch(NoSuchName ex)
        {
            System.out.println("`" + name + "': no such directory");
            return;
        }
        if(d.type == NodeType.FileType)
        {
            System.out.println("`" + name + "': not a directory");
            return;
        }
        _dirs.addFirst(DirectoryPrxHelper.uncheckedCast(d.proxy));
    }

    void
    cat(String name)
    {
        DirectoryPrx dir = _dirs.getFirst();
        NodeDesc d;
        try
        {
            d = dir.find(name);
        }
        catch(NoSuchName ex)
        {
            System.out.println("`" + name + "': no such file");
            return;
        }
        if(d.type == NodeType.DirType)
        {
            System.out.println("`" + name + "': not a file");
            return;
        }
        FilePrx f = FilePrxHelper.uncheckedCast(d.proxy);
        String[] l = f.read();
        for(int i = 0; i < l.length; ++i)
        {
            System.out.println(l[i]);
        }
    }

    void
    write(java.util.LinkedList<String> args)
    {
        DirectoryPrx dir = _dirs.getFirst();
        String name = args.getFirst();
        args.removeFirst();
        NodeDesc d;
        try
        {
            d = dir.find(name);
        }
        catch(NoSuchName ex)
        {
            System.out.println("`" + name + "': no such file");
            return;
        }
        if(d.type == NodeType.DirType)
        {
            System.out.println("`" + name + "': not a file");
            return;
        }
        FilePrx f = FilePrxHelper.uncheckedCast(d.proxy);

        String[] l = args.toArray(new String[0]);
        try
        {
            f.write(l);
        }
        catch(GenericError ex)
        {
            System.out.println("`" + name + "': cannot write to file: " + ex.reason);
        }
    }

    void
    destroy(java.util.List<String> names)
    {
        DirectoryPrx dir = _dirs.getFirst();

        for(String name : names)
        {
            if(name.equals("*"))
            {
                NodeDesc[] nodes = dir.list();
                for(NodeDesc node : nodes)
                {
                    try
                    {
                        node.proxy.destroy();
                    }
                    catch(PermissionDenied ex)
                    {
                        System.out.println("cannot remove `" + node.name + "': " + ex.reason);
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
                catch(NoSuchName ex)
                {
                    System.out.println("`" + name + "': no such file or directory");
                    return;
                }
                try
                {
                    d.proxy.destroy();
                }
                catch(PermissionDenied ex)
                {
                    System.out.println("cannot remove `" + name + "': " + ex.reason);
                }
            }
        }
    }

    void
    error(String s)
    {
        System.err.println("error: " + s);
    }

    void
    warning(String s)
    {
        System.err.println("warning: " + s);
    }

    String
    getInput()
    {
        System.out.print("> ");
        System.out.flush();

        try
        {
            return _in.readLine();
        }
        catch(java.io.IOException e)
        {
            return null;
        }
    }

    int
    parse()
    {
        _in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        Grammar g = new Grammar(this);
        g.parse();

        return 0;
    }

    int
    parse(java.io.BufferedReader in)
    {
        _in = in;

        Grammar g = new Grammar(this);
        g.parse();

        return 0;
    }

    private java.util.LinkedList<DirectoryPrx> _dirs;

    private java.io.BufferedReader _in;
}
