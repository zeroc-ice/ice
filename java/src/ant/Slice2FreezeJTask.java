// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
        java.util.HashMap<String, SliceDependency> dependencies = readDependencies();

        //
        // Check if the set of slice files changed. If it changed we
        // need to rebuild all the dictionnaries and indices.
        //
        boolean build = false;
        java.util.List<File> sliceFiles = new java.util.LinkedList<File>();

        for(FileSet fileset : _fileSets)
        {
            DirectoryScanner scanner = fileset.getDirectoryScanner(getProject());
            String[] files = scanner.getIncludedFiles();

            for(String file : files)
            {
                File slice = new File(fileset.getDir(getProject()), file);
                sliceFiles.add(slice);

                if(!build)
                {
                    //
                    // The dictionnaries need to be re-created since
                    // on dependency changed.
                    //
                    SliceDependency depend = dependencies.get(getSliceTargetKey(slice.toString()));
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
            for(Dict p : _dicts)
            {
                SliceDependency depend = dependencies.get(getDictTargetKey(p));
                if(depend == null)
                {
                    build = true;
                    break;
                }
            }

            //
            // Likewise for indices
            //
            for(Index p : _indices)
            {
                SliceDependency depend = dependencies.get(getIndexTargetKey(p));
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
        StringBuilder dictString = new StringBuilder(128);
        for(Dict d : _dicts)
        {
            dictString.append(" --dict ");
            dictString.append(d.getName());
            dictString.append(",");
            dictString.append(d.getKey());
            dictString.append(",");
            dictString.append(d.getValue());
        }

        //
        // Add the --dict-index options.
        //
        StringBuilder dictIndexString = new StringBuilder(128);
        for(Dictindex d : _dictIndices)
        {
            dictIndexString.append(" --dict-index ");
            dictIndexString.append(d.getName());
            if(d.getMember() != null)
            {
                dictIndexString.append(",");
                dictIndexString.append(d.getMember());
            }
            if(d.getCasesensitive() == false)
            {
                dictIndexString.append(",case-insensitive");
            }
        }

        //
        // Add the --index options.
        //
        StringBuilder indexString = new StringBuilder();
        for(Index i : _indices)
        {
            indexString.append(" --index ");
            indexString.append(i.getName());
            indexString.append(",");
            indexString.append(i.getType());
            indexString.append(",");
            indexString.append(i.getMember());
            if(i.getCasesensitive() == false)
            {
                indexString.append(",case-insensitive");
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
        StringBuilder cmd = new StringBuilder(256);

        //
        // Add --ice
        //
        if(_ice)
        {
            cmd.append(" --ice");
        }

        //
        // Add --underscore
        //
        if(_underscore)
        {
            cmd.append(" --underscore");
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
        // Add include directives
        //
        if(_includePath != null)
        {
            String[] dirs = _includePath.list();
            for(String dir : dirs)
            {
                cmd.append(" -I");
                if(dir.indexOf(' ') != -1)
                {
                    cmd.append('"' + dir + '"');
                }
                else
                {
                    cmd.append(dir);
                }
            }
        }

        //
        // Add defines
        //
        if(!_defines.isEmpty())
        {
            for(SliceDefine define : _defines)
            {
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
        cmd.append(dictString.toString());

        //
        // Add the --dict-index options.
        //
        cmd.append(dictIndexString.toString());

        //
        // Add the --index options.
        //
        cmd.append(indexString.toString());

        //
        // Add the --meta options.
        //
        if(!_meta.isEmpty())
        {
            for(SliceMeta m : _meta)
            {
                cmd.append(" --meta " + m.getValue());
            }
        }

        //
        // Add the slice files.
        //
        for(File f : sliceFiles)
        {
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
            translator = getDefaultTranslator("slice2freezej");
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
        addLdLibraryPath(task);
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
            cmd = new StringBuilder(256);
            cmd.append("--depend-xml");

            //
            // Add --ice
            //
            if(_ice)
            {
                cmd.append(" --ice");
            }

            //
            // Add --underscore
            //
            if(_underscore)
            {
                cmd.append(" --underscore");
            }

            //
            // Add include directives
            //
            if(_includePath != null)
            {
                String[] dirs = _includePath.list();
                for(String dir : dirs)
                {
                    cmd.append(" -I");
                    if(dir.indexOf(' ') != -1)
                    {
                        cmd.append('"');
                        cmd.append(dir);
                        cmd.append('"');
                    }
                    else
                    {
                        cmd.append(dir);
                    }
                }
            }

            //
            // Add the --dict options.
            //
            cmd.append(dictString.toString());

            //
            // Add the --dict-index options.
            //
            cmd.append(dictIndexString.toString());

             //
            // Add the --index options.
            //
            cmd.append(indexString.toString());

            //
            // Add the slice files.
            //
            for(File f : sliceFiles)
            {
                cmd.append(" ");
                String s = f.toString();
                if(s.indexOf(' ') != -1)
                {
                    cmd.append('"');
                    cmd.append(s);
                    cmd.append('"');
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
            addLdLibraryPath(task);
            task.setFailonerror(true);
            arg = task.createArg();
            arg.setLine(cmd.toString());
            task.setExecutable(translator);
            task.setOutputproperty(outputProperty);
            task.execute();

            //
            // Update dependency file.
            //
            java.util.List<SliceDependency> newDependencies =
                parseDependencies(getProject().getProperty(outputProperty));
            for(SliceDependency dep : newDependencies)
            {
                dependencies.put(getSliceTargetKey(dep._dependencies[0]), dep);
            }
        }

        for(Dict d : _dicts)
        {
            dependencies.put(getDictTargetKey(d), new SliceDependency());
        }

        for(Index i : _indices)
        {
            dependencies.put(getIndexTargetKey(i), new SliceDependency());
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
            name = (_dicts.get(0)).getName();
        }
        else
        {
            name = (_indices.get(0)).getName();
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

    private java.util.List<Dict> _dicts = new java.util.LinkedList<Dict>();
    private java.util.List<Dictindex> _dictIndices = new java.util.LinkedList<Dictindex>();
    private java.util.List<Index> _indices = new java.util.LinkedList<Index>();
}
