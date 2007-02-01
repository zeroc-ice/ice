// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//package Ice.Ant;

import org.apache.tools.ant.Task;
import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.DirectoryScanner;
import org.apache.tools.ant.types.FileSet;
import org.apache.tools.ant.taskdefs.ExecTask;
import org.apache.tools.ant.types.Commandline.Argument;
import org.apache.tools.ant.types.Path;
import org.apache.tools.ant.types.Reference;

import java.io.File;
import java.io.FileOutputStream;

/**
 * An ant task for slice2freezej. This task extends the abstract
 * SliceTask class which takes care of attributes common to all slice
 * translators (see SliceTask.java for details on these attributes).
 *
 * Attributes:
 *
 * Nested elements:
 *
 *   define - defines a preprocessor symbol
 *   dict - contains the NAME, KEY TYPE, and VALUE TYPE of a Freeze map.
 *   index - contains the NAME, CLASS TYPE, MEMBER NAME and optional
 *   case sensitivity of a Freeze Evictor index.
 *   dictindex - contains the NAME and optional member name and case
 *   sensitivity of a Freeze Map index.
 *
 * Example:
 *
 *    <project ...>
 *        <taskdef name="slice2freezej" classname="Slice2FreezeJTask" />
 *        <property name="slice.dir" value="../include/slice"/>
 *        <target name="generate">
 *            <mkdir dir="tags" />
 *            <slice2freezej tagdir="tags" outputdir="out" output="CharIntMap">
 *                <define name="SYMBOL" value="VALUE"/>
 *                <includepath>
 *                    <pathelement path="${slice.dir}" />
 *                </includepath>
 *                <fileset dir="${slice.dir}">
 *                    <include name="*.ice" />
 *                </fileset>
 *                <dict name="CharIntMap" key="char" value="int"/>
 *                <index name="NameIndex" type="Foo" member="name" casesensitive="false"/>
 *            </slice2freezej>
 *        </target>
 *    </project>
 *
 * The <taskdef> element installs the slice2freezej task.
 */
public class Slice2FreezeJTask extends SliceTask
{
    public
    Slice2FreezeJTask()
    {
    }

    public void
    setTranslator(File prog)
    {
        _translator = prog;
    }

    public Dict
    createDict()
    {
        Dict d = new Dict();
        _dicts.add(d);
        return d;
    }

    public Index
    createIndex()
    {
        Index i = new Index();
        _indices.add(i);
        return i;
    }
    
    public Dictindex
    createDictindex()
    {
        Dictindex i = new Dictindex();
        _dictIndices.add(i);
        return i;
    }

    public void
    execute()
        throws BuildException
    {
        if(_dicts.isEmpty() && _indices.isEmpty())
        {
            throw new BuildException("No dictionary or index specified");
        }

        //
        // Read the set of dependencies for this task.
        //
        java.util.HashMap dependencies = readDependencies();

        //
        // Check if the set of slice files changed. If it changed we
        // need to rebuild all the dictionnaries and indices.
        //
        boolean build = false;
        java.util.List sliceFiles = new java.util.LinkedList();

        java.util.Iterator p = _fileSets.iterator();
        while(p.hasNext())
        {
            FileSet fileset = (FileSet)p.next();

            DirectoryScanner scanner = fileset.getDirectoryScanner(getProject());
            String[] files = scanner.getIncludedFiles();
            
            for(int i = 0; i < files.length; i++)
            {
                File slice = new File(fileset.getDir(getProject()), files[i]);
                sliceFiles.add(slice);

                if(!build)
                {
                    //
                    // The dictionnaries need to be re-created since
                    // on dependency changed.
                    //
                    SliceDependency depend = (SliceDependency)dependencies.get(getSliceTargetKey(slice.toString()));
                    if(depend == null || !depend.isUpToDate())
                    {
                        build = true;
                    }
                }
            }
        }

        if(!build)
        {
            //
            // Check that each dictionary has been built at least
            // once.
            //
            p = _dicts.iterator();
            while(p.hasNext())
            {
                SliceDependency depend = (SliceDependency)dependencies.get(getDictTargetKey((Dict)p.next()));
                if(depend == null)
                {
                    build = true;
                    break;
                }
            }

            //
            // Likewise for indices
            //
            p = _indices.iterator();
            while(p.hasNext())
            {
                SliceDependency depend = (SliceDependency)dependencies.get(getIndexTargetKey((Index)p.next()));
                if(depend == null)
                {
                    build = true;
                    break;
                }
            }
        }
        
        //
        // Add the --dict options.
        //
        p = _dicts.iterator();
        StringBuffer dictString = new StringBuffer();
        while(p.hasNext())
        {
            Dict d = (Dict)p.next();

            dictString.append(" --dict ");
            dictString.append(d.getName() + "," + d.getKey() + "," + d.getValue());
        }

        //
        // Add the --dict-index options.
        //
        p = _dictIndices.iterator();
        StringBuffer dictIndexString = new StringBuffer();
        while(p.hasNext())
        {
            Dictindex d = (Dictindex)p.next();

            dictIndexString.append(" --dict-index ");
            dictIndexString.append(d.getName());
            if(d.getMember() != null)
            {
                dictIndexString.append("," + d.getMember());
            }
            if(d.getCasesensitive() == false)
            {
                dictIndexString.append("," + "case-insensitive");
            }
        }
        
        //
        // Add the --index options.
        //
        p = _indices.iterator();
        StringBuffer indexString = new StringBuffer();
        while(p.hasNext())
        {
            Index i = (Index)p.next();

            indexString.append(" --index ");
            indexString.append(i.getName() + "," + i.getType() + "," + i.getMember());
            if(i.getCasesensitive() == false)
            {
                indexString.append("," + "case-insensitive");
            }
        }

        if(!build)
        {
            log("skipping" + dictString + indexString);
            return;
        }

        //
        // Run the translator
        //
        StringBuffer cmd = new StringBuffer();

        //
        // Add --ice
        //
        if(_ice)
        {
            cmd.append(" --ice");
        }

        //
        // Add --output-dir
        //
        if(_outputDir != null)
        {
            cmd.append(" --output-dir ");
            cmd.append(_outputDirString);
        }

        //
        // Add --case-sensitive
        //
        if(_caseSensitive)
        {
            cmd.append(" --case-sensitive");
        }

        //
        // Add include directives
        //
        if(_includePath != null)
        {
            String[] dirs = _includePath.list();
            for(int i = 0; i < dirs.length; i++)
            {
                cmd.append(" -I");
                if(dirs[i].indexOf(' ') != -1)
                {
                    cmd.append('"' + dirs[i] + '"');
                }
                else
                {
                    cmd.append(dirs[i]);
                }
            }
        }

        //
        // Add defines
        //
        if(!_defines.isEmpty())
        {
            java.util.Iterator i = _defines.iterator();
            while(i.hasNext())
            {
                SliceDefine define = (SliceDefine)i.next();
                cmd.append(" -D");
                cmd.append(define.getName());
                String value = define.getValue();
                if(value != null)
                {
                    cmd.append("=");
                    cmd.append(value);
                }
            }
        }

        //
        // Add the --dict options.
        //
        cmd.append(dictString);

        //
        // Add the --dict-index options.
        //
        cmd.append(dictIndexString);

        //
        // Add the --index options.
        //
        cmd.append(indexString);

        //
        // Add the --meta options.
        //
        if(!_meta.isEmpty())
        {
            java.util.Iterator i = _meta.iterator();
            while(i.hasNext())
            {
                SliceMeta m = (SliceMeta)i.next();
                cmd.append(" --meta " + m.getValue());
            }
        }

        //
        // Add the slice files.
        //
        p = sliceFiles.iterator();
        while(p.hasNext())
        {
            File f = (File)p.next();
            cmd.append(" ");
            String s = f.toString();
            if(s.indexOf(' ') != -1)
            {
                cmd.append('"' + s + '"');
            }
            else
            {
                cmd.append(s);
            }
        }

        String translator;
        if(_translator == null)
        {
            if(getIceHome() == null)
            {
                translator = "slice2freezej";
            }
            else
            {
                translator = new File(getIceHome() + File.separator + "bin" + File.separator + "slice2freezej").toString();
            }
        }
        else
        {
            translator = _translator.toString();
        }
        
        //
        // Execute.
        //
        log(translator + " " + cmd);
        ExecTask task = (ExecTask)getProject().createTask("exec");
        task.setFailonerror(true);
        Argument arg = task.createArg();
        arg.setLine(cmd.toString());
        task.setExecutable(translator);
        task.execute();
        
        //
        // Update the dependencies.
        //      
        if(!sliceFiles.isEmpty())
        {
            cmd = new StringBuffer("--depend");
            
            //
            // Add include directives
            //
            if(_includePath != null)
            {
                String[] dirs = _includePath.list();
                for(int i = 0; i < dirs.length; i++)
                {
                    cmd.append(" -I");
                    if(dirs[i].indexOf(' ') != -1)
                    {
                        cmd.append('"' + dirs[i] + '"');
                    }
                    else
                    {
                        cmd.append(dirs[i]);
                    }
                }
            }

            //
            // Add the --dict options.
            //
            cmd.append(dictString);

            //
            // Add the --dict-index options.
            //
            cmd.append(dictIndexString);

             //
            // Add the --index options.
            //
            cmd.append(indexString);

            //
            // Add the slice files.
            //
            p = sliceFiles.iterator();
            while(p.hasNext())
            {
                File f = (File)p.next();
                cmd.append(" ");
                String s = f.toString();
                if(s.indexOf(' ') != -1)
                {
                    cmd.append('"' + s + '"');
                }
                else
                {
                    cmd.append(s);
                }
            }
 

            //
            // It's not possible anymore to re-use the same output property since Ant 1.5.x. so we use a 
            // unique property name here. Perhaps we should output the dependencies to a file instead.
            //
            final String outputProperty = "slice2freezej.depend." + System.currentTimeMillis();

            task = (ExecTask)getProject().createTask("exec");
            task.setFailonerror(true);
            arg = task.createArg();
            arg.setLine(cmd.toString());
            task.setExecutable(translator);
            task.setOutputproperty(outputProperty);
            task.execute();

            //
            // Update dependency file.
            //
            java.util.List newDependencies = parseDependencies(getProject().getProperty(outputProperty));
            p = newDependencies.iterator();
            while(p.hasNext())
            {
                SliceDependency dep = (SliceDependency)p.next();
                dependencies.put(getSliceTargetKey(dep._dependencies[0]), dep);
            }
        }

        p = _dicts.iterator();
        while(p.hasNext())
        {
            dependencies.put(getDictTargetKey((Dict)p.next()), new SliceDependency());
        }

        p = _indices.iterator();
        while(p.hasNext())
        {
            dependencies.put(getIndexTargetKey((Index)p.next()), new SliceDependency());
        }

        writeDependencies(dependencies);
    }

    private String
    getSliceTargetKey(String slice)
    {
        //
        // Since the dependency file can be shared by several slice
        // tasks we need to make sure that each dependency has a
        // unique key. We use the name of the task, the output
        // directory, the first dictionary or index name and the name of the
        // slice file to be compiled.
        //
        String name;
        if(_dicts.size() > 0)
        {
            name = ((Dict)_dicts.get(0)).getName();
        }
        else
        {
            name = ((Index)_indices.get(0)).getName();
        }
        return "slice2freezej " + _outputDir.toString() + name + slice;
    }

    private String
    getDictTargetKey(Dict d)
    {
        return "slice2freezej " + _outputDir.toString() + d.getName();
    }

    private String
    getIndexTargetKey(Index i)
    {
        return "slice2freezej " + _outputDir.toString() + i.getName();
    }

    private File _translator = null;

    public class Dict
    {
        private String _name;
        private String _key;
        private String _value;

        public void
        setName(String name)
        {
            _name = name;
        }

        public String
        getName()
        {
            return _name;
        }

        public void
        setKey(String key)
        {
            _key = key;
        }

        public String
        getKey()
        {
            return _key;
        }

        public void
        setValue(String value)
        {
            _value = value;
        }

        public String
        getValue()
        {
            return _value;
        }
    }

    public class Dictindex
    {
        private String _name;
        private String _member;
        private boolean _caseSensitive = true;

        public void
        setName(String name)
        {
            _name = name;
        }

        public String
        getName()
        {
            return _name;
        }

        public void
        setMember(String member)
        {
            _member = member;
        }

        public String
        getMember()
        {
            return _member;
        }
        
        public void
        setCasesensitive(boolean caseSensitive)
        {
            _caseSensitive = caseSensitive;
        }

        public boolean
        getCasesensitive()
        {
            return _caseSensitive;
        }
    }


    public class Index
    {
        private String _name;
        private String _type;
        private String _member;
        private boolean _caseSensitive = true;

        public void
        setName(String name)
        {
            _name = name;
        }

        public String
        getName()
        {
            return _name;
        }

        public void
        setType(String type)
        {
            _type = type;
        }

        public String
        getType()
        {
            return _type;
        }

        public void
        setMember(String member)
        {
            _member = member;
        }

        public String
        getMember()
        {
            return _member;
        }
        
        public void
        setCasesensitive(boolean caseSensitive)
        {
            _caseSensitive = caseSensitive;
        }

        public boolean
        getCasesensitive()
        {
            return _caseSensitive;
        }
    }

    private java.util.List _dicts = new java.util.LinkedList();
    private java.util.List _dictIndices = new java.util.LinkedList();
    private java.util.List _indices = new java.util.LinkedList();
}
