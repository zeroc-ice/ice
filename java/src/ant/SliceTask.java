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
import org.apache.tools.ant.taskdefs.Execute;
import org.apache.tools.ant.taskdefs.PumpStreamHandler;
import org.apache.tools.ant.types.Commandline;
import org.apache.tools.ant.types.Environment;
import org.apache.tools.ant.types.Commandline.Argument;
import org.apache.tools.ant.types.Path;
import org.apache.tools.ant.types.Reference;

import java.util.List;
import java.util.ArrayList;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.StringReader;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayInputStream;
import java.io.BufferedInputStream;
import java.io.InputStream;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;
import org.xml.sax.SAXException;

/**
 * An abstract ant task for slice translators. The task minimizes
 * regeneration by checking the dependencies between slice files.
 *
 * Attributes:
 *
 *   dependencyfile - The file in which dependencies are stored (default: ".depend").
 *   outputdir - The value for the --output-dir translator option.
 *   ice - Enables the --ice translator option.
 *   underscore - Enables the --underscore translator option.
 *
 * Nested elements:
 *
 *   includepath - The directories in which to search for Slice files.
 *     These are converted into -I directives for the translator.
 *   define - Defines a preprocessor symbol. The "name" attribute
 *     specifies the symbol name, and the optional "value" attribute
 *     specifies the symbol's value.
 *   fileset - The set of Slice files which contain relevent types.
 *
 */
public class SliceTask extends org.apache.tools.ant.Task
{
    public
    SliceTask()
    {
        _dependencyFile = null;
        _outputDir = null;
        _outputDirString = null;
        _ice = false;
        _includePath = null;
    }

    public void
    setDependencyFile(File file)
    {
        _dependencyFile = file;
    }

    public void
    setOutputdir(File dir)
    {
        _outputDir = dir;
        _outputDirString = _outputDir.toString();
        if(_outputDirString.indexOf(' ') != -1)
        {
            _outputDirString = '"' + _outputDirString + '"';
        }
    }

    public void
    setIce(boolean ice)
    {
        _ice = ice;
    }

    public void
    setUnderscore(boolean underscore)
    {
        _underscore = underscore;
    }

    public Path
    createIncludePath()
    {
        if(_includePath == null)
        {
            _includePath = new Path(getProject());
        }
        return _includePath.createPath();
    }

    public void
    setIncludePathRef(Reference ref)
    {
        createIncludePath().setRefid(ref);
    }

    public void
    setIncludePath(Path includePath)
    {
        if(_includePath == null)
        {
            _includePath = includePath;
        }
        else
        {
            _includePath.append(includePath);
        }
    }

    public FileSet
    createFileset()
    {
        FileSet fileset = new FileSet();
        _fileSets.add(fileset);

        return fileset;
    }

    public void
    addConfiguredDefine(SliceDefine define)
        throws BuildException
    {
        if(define.getName() == null)
        {
            throw new BuildException("The name attribute must be supplied in a <define> element");
        }

        _defines.add(define);
    }

    public void
    addConfiguredMeta(SliceMeta meta)
    {
        if(meta.getValue().length() > 0)
        {
            _meta.add(meta);
        }
    }

    //
    // Read the dependency file.
    //
    @SuppressWarnings("unchecked")
    protected java.util.HashMap<String, SliceDependency>
    readDependencies()
    {
        if(_dependencyFile == null)
        {
            if(_outputDir != null)
            {
                _dependencyFile = new File(_outputDir, ".depend");
            }
            else
            {
                _dependencyFile = new File(".depend");
            }
        }

        try
        {
            java.io.ObjectInputStream in = new java.io.ObjectInputStream(new java.io.FileInputStream(_dependencyFile));
            java.util.HashMap<String, SliceDependency> dependencies =
                (java.util.HashMap<String, SliceDependency>)in.readObject();
            in.close();
            return dependencies;
        }
        catch(java.io.IOException ex)
        {
        }
        catch(java.lang.ClassNotFoundException ex)
        {
        }

        return new java.util.HashMap<String, SliceDependency>();
    }

    protected void
    writeDependencies(java.util.HashMap<String, SliceDependency> dependencies)
    {
        try
        {
            java.io.ObjectOutputStream out = new java.io.ObjectOutputStream(new FileOutputStream(_dependencyFile));
            out.writeObject(dependencies);
            out.close();
        }
        catch(java.io.IOException ex)
        {
            throw new BuildException("Unable to write dependencies in file " + _dependencyFile.getPath() + ": " + ex);
        }
    }

    //
    // Parse dependencies returned by the slice translator (Makefile
    // dependencies).
    //
    protected java.util.List<SliceDependency>
    parseDependencies(String allDependencies)
    {
        Slice2JavaDependenciesParser parser = new Slice2JavaDependenciesParser();
        try
        {
            InputStream in = new ByteArrayInputStream(allDependencies.getBytes());
            Document doc = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(new BufferedInputStream(in));
            parser.visit(doc);
        }
        catch(SAXException ex)
        {
            throw new BuildException("Unable to read dependencies from slice translator: " + ex);
        }
        catch(ParserConfigurationException ex)
        {
            throw new BuildException("Unable to read dependencies from slice translator: " + ex);
        }
        catch(java.io.IOException ex)
        {
            throw new BuildException("Unable to read dependencies from slice translator: " + ex);
        }

        return parser.dependencies;
    }

    protected String
    getDefaultTranslator(String name)
    {
        String iceInstall = getIceHome();
        if(iceInstall != null)
        {
            return new File(iceInstall + File.separator + "bin" + File.separator + name).toString();
        }
        else
        {
            //
            // If the location of the Ice install is not known, we
            // rely on a path search to find the translator.
            //
            return name;
        }
    }

    protected void
    addLdLibraryPath(ExecTask task)
    {
        String iceInstall = getIceHome();
        boolean srcdist = getProject().getProperties().containsKey("ice.src.dist");
        if(iceInstall != null)
        {
            String ldLibPathEnv = null;
            String ldLib64PathEnv = null;
            String libPath = new File(iceInstall + File.separator + "lib").toString();
            String lib64Path = null;

            String os = System.getProperty("os.name");
            if(os.equals("Mac OS X"))
            {
                ldLibPathEnv = "DYLD_LIBRARY_PATH";
            }
            else if(os.equals("AIX"))
            {
                ldLibPathEnv = "LIBPATH";
            }
            else if(os.equals("HP-UX"))
            {
                ldLibPathEnv = "SHLIB_PATH";
                ldLib64PathEnv = "LD_LIBRARY_PATH";
                if(srcdist)
                {
                    lib64Path = libPath;
                }
                else
                {
                    lib64Path = new File(iceInstall + File.separator + "lib" + File.separator + "pa20_64").toString();
                }
            }
            else if(os.startsWith("Windows"))
            {
                //
                // No need to change the PATH environment variable on Windows, the DLLs should be found
                // in the translator local directory.
                //
                //ldLibPathEnv = "PATH";
            }
            else if(os.equals("SunOS"))
            {
                ldLibPathEnv = "LD_LIBRARY_PATH";
                ldLib64PathEnv = "LD_LIBRARY_PATH_64";
                String arch = System.getProperty("os.arch");
                if(srcdist)
                {
                    lib64Path = libPath;
                }
                else if(arch.equals("x86"))
                {
                    lib64Path = new File(iceInstall + File.separator + "lib" + File.separator + "amd64").toString();
                }
                else // Sparc
                {
                    lib64Path = new File(iceInstall + File.separator + "lib" + File.separator + "sparcv9").toString();
                }
            }
            else
            {
                ldLibPathEnv = "LD_LIBRARY_PATH";
                ldLib64PathEnv = "LD_LIBRARY_PATH";
                if(srcdist)
                {
                    lib64Path = libPath;
                }
                else
                {
                    lib64Path = new File(iceInstall + File.separator + "lib64").toString();
                }
            }

            if(ldLibPathEnv != null)
            {
                if(ldLibPathEnv.equals(ldLib64PathEnv))
                {
                    libPath = libPath + File.pathSeparator + lib64Path;
                }

                String envLibPath = getEnvironment(ldLibPathEnv);
                if(envLibPath != null)
                {
                    libPath = libPath + File.pathSeparator + envLibPath;
                }

                Environment.Variable v = new Environment.Variable();
                v.setKey(ldLibPathEnv);
                v.setValue(libPath);
                task.addEnv(v);
            }

            if(ldLib64PathEnv != null && !ldLib64PathEnv.equals(ldLibPathEnv))
            {
                String envLib64Path = getEnvironment(ldLib64PathEnv);
                if(envLib64Path != null)
                {
                    lib64Path = lib64Path + File.pathSeparator + envLib64Path;
                }

                Environment.Variable v = new Environment.Variable();
                v.setKey(ldLib64PathEnv);
                v.setValue(lib64Path);
                task.addEnv(v);
            }
        }
    }

    //
    // Query for the location of the Ice install. The Ice install
    // location may be indicated in one of two ways:
    //
    //  1. Through the ice.home property
    //  2. Through the ICE_HOME environment variable.
    //
    //  If both the property and environment variable is specified, the
    //  property takes precedence. If neither is available, getIceHome()
    //  returns null.
    //
    protected String
    getIceHome()
    {
        if(_iceHome == null)
        {
            if(getProject().getProperties().containsKey("ice.home"))
            {
                _iceHome = (String)getProject().getProperties().get("ice.home");
            }
            else
            {
                _iceHome = getEnvironment("ICE_HOME");
            }
        }
        return _iceHome;
    }

    //
    // A slice dependency.
    //
    // * the _timeStamp attribute contains the last time the slice
    //   file was compiled.
    //
    // * the _dependencies attribute contains an array with all the
    //   files this slice file depends on.
    //
    // This dependency represents the dependencies for the slice file
    // _dependencies[0].
    //
    protected class SliceDependency implements java.io.Serializable
    {
        private void writeObject(java.io.ObjectOutputStream out)
            throws java.io.IOException
        {
            out.writeObject(_dependencies);
            out.writeLong(_timeStamp);
        }

        private void readObject(java.io.ObjectInputStream in)
            throws java.io.IOException, java.lang.ClassNotFoundException
        {
            _dependencies = (String[])in.readObject();
            _timeStamp = in.readLong();
        }

        public boolean
        isUpToDate()
        {
            for(String d : _dependencies)
            {
                File dep = new File(d);
                if(!dep.exists() || _timeStamp < dep.lastModified())
                {
                    return false;
                }
            }

            return true;
        }

        public String[] _dependencies;
        public long _timeStamp;
        public static final long serialVersionUID = 0L;
    }

    @SuppressWarnings("deprecation")
    private String
    getEnvironment(String key)
    {
        java.util.Vector env = Execute.getProcEnvironment();
        java.util.Enumeration e = env.elements();
        while(e.hasMoreElements())
        {
            String entry = (String)e.nextElement();
            if(entry.startsWith(key + "="))
            {
                return entry.substring(entry.indexOf('=') + 1);
            }
        }
        return null;
    }

    private class Slice2JavaDependenciesParser
    {
        java.util.List<SliceDependency> dependencies = new java.util.LinkedList<SliceDependency>();
        
        private Node findNode(Node n, String qName)
            throws SAXException
        {
            NodeList children = n.getChildNodes();
            for(int i = 0; i < children.getLength(); ++i)
            {
                Node child = children.item(i);
                if(child.getNodeType() == Node.ELEMENT_NODE && child.getNodeName().equals(qName))
                {
                    return child;
                }
            }
            throw new SAXException("no such node: " + qName);
        }

        private void visitDependencies(Node n) throws SAXException
        {
            NodeList children = n.getChildNodes();
            for(int i = 0; i < children.getLength(); ++i)
            {
                if(children.item(i).getNodeType() == Node.ELEMENT_NODE && children.item(i).getNodeName().equals("source"))
                {
                    String source = ((Element)children.item(i)).getAttribute("name");
                    if(source.length() == 0)
                    {
                        throw new SAXException("empty name attribute");
                    }
                    List<String> dependsOn = visitDependsOn(children.item(i));
                    SliceDependency depend = new SliceDependency();
                    depend._timeStamp = new java.util.Date().getTime();
                    depend._dependencies = new String[dependsOn.size() + 1];
                    depend._dependencies[0] = source;
                    for(int j = 0; j < dependsOn.size(); j++)
                    {
                        depend._dependencies[j + 1] = dependsOn.get(j);
                    }
                    dependencies.add(depend);
                }
            }
        }

        private List<String> visitDependsOn(Node source) throws SAXException
        {
            List<String> depends = new ArrayList<String>();
            NodeList dependencies = source.getChildNodes();
            for(int j = 0; j < dependencies.getLength(); ++j)
            {
                if(dependencies.item(j).getNodeType() == Node.ELEMENT_NODE && dependencies.item(j).getNodeName().equals("dependsOn"))
                {
                    Element dependsOn = (Element)dependencies.item(j);
                    String name = dependsOn.getAttribute("name");
                    if(name.length() == 0)
                    {
                        throw new SAXException("empty name attribute");
                    }
                    depends.add(name);
                }
            }
            return depends;
        }
        
        public void visit(Node doc) throws SAXException
        {
            Node n = findNode(doc, "dependencies");
            visitDependencies(n);
        }
    }

    protected File _dependencyFile;
    protected File _outputDir;
    protected String _outputDirString;
    protected boolean _ice;
    protected boolean _underscore;
    protected Path _includePath;
    protected java.util.List<FileSet> _fileSets = new java.util.LinkedList<FileSet>();
    protected java.util.List<SliceDefine> _defines = new java.util.LinkedList<SliceDefine>();
    protected java.util.List<SliceMeta> _meta = new java.util.LinkedList<SliceMeta>();
    private String _iceHome;
}
