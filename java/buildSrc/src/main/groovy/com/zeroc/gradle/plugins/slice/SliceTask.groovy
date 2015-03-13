// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.gradle.plugins.slice;

import org.gradle.api.DefaultTask
import org.gradle.api.tasks.TaskAction
import org.gradle.api.tasks.InputFiles
import org.gradle.api.tasks.OutputDirectory
import org.gradle.api.GradleException
import java.io.*
import org.slf4j.Logger
import org.slf4j.LoggerFactory

import groovy.util.XmlSlurper
import groovy.xml.MarkupBuilder

class SliceTask extends DefaultTask {
    private static final Logger LOGGER = LoggerFactory.getLogger(SliceTask)

    @TaskAction
    def action() {
        if (!project.slice.output.isDirectory()) {
            if (!project.slice.output.mkdirs()) {
                throw new GradleException("Could not create slice output directory: " + project.slice.output);
            }
        }

        // Make sure default source set is present
        if (project.slice.java.isEmpty()) {
            project.slice.java.create("default")
        }

        project.slice.java.each {
            processJavaSet(it)
        }

        processFreezeJ(project.slice.freezej)
    }

    @InputFiles
    def getInputFiles() {
        def files = []

        // Make sure default source set is present
        if (project.slice.java.isEmpty()) {
            project.slice.java.create("default")
        }

        project.slice.java.each {
            if (it.files == null) {
                files.addAll(project.fileTree(dir: it.srcDir).include('**/*.ice'))
            } else {
                files.addAll(it.files)
            }
        }

        if(project.slice.freezej.files) {
        	files.addAll(project.slice.freezej.files)
        }

        return files
    }

    @OutputDirectory
    def getOutputDirectory() {
        return project.slice.output
    }

	class FreezeJBuildState {
    	// Dictionary of file -> timestamp.
    	def slice = [:]

    	// List of generated source files.
    	def generated = []
    };

    def processFreezeJ(freezej) {
    	if((freezej.dict == null || freezej.dict.isEmpty()) && (freezej.index == null || freezej.index.isEmpty())) {
    		return;
    	}

    	def sourceFiles = freezej.files

        // Dictionary of A  -> [B] where A depends on B.
        def sliceDependencies = getFreezejDependencies(freezej, sourceFiles)

        // Set of source files and all dependencies.
        def allSourceFiles = new HashSet<>()
        allSourceFiles.addAll(sourceFiles)
        sliceDependencies.each {
        	allSourceFiles.addAll(it.value)
        }

		// Dictionary to A -> timestamp, where A is a slice file
        // we want to process.
        def timestamps = getTimestamps(allSourceFiles)

        // Dictionary of A -> timestamp, [B] where A is a slice file,
        // timestamp is the  modified time for A at last build and B is the
        // list of produced java source files.
        def state = new FreezeJBuildState()
        def stateFile = new File(project.buildDir, "slice2freezej.df.xml")
        if(stateFile.isFile()) {
            try {
                state = parseFreezeJBuildState(new XmlSlurper().parse(stateFile));
            }
            catch(Exception ex) {
                LOGGER.info("State file {} not found, or invalid", stateFile);
                println ex
            }
        }

        def count = 0
        timestamps.each {
            def timestamp = state.slice[it.key]
            // If the dependency doesn't exist, or the timestamp
            // is older than the current timestamp then we need
            // to build the source file.
            if(timestamp == null || timestamp < it.value) {
            	++count
            }
        }
        // Bail out if there is nothing to do (in theory this should not occur).
        if(count == 0) {
            LOGGER.info("Nothing to do");
            return;
        }

        LOGGER.info("Running slice2freezej on the following slice files")
        sourceFiles.each {
            LOGGER.info("    {}", it)
        }

        // List of generated java source files.
        def generated = executeSlice2Freezej(freezej, sourceFiles)

        // Gather up the list of source files that we previously built for those
        // files which are building.
        def oldfiles = new HashSet<>()
        oldfiles.addAll(state.generated)

        // Remove all source files that we have generated from the list of
        // sources file that we previously have built.
        generated.each {
			oldfiles.remove(it)
        }

        LOGGER.info("The following generated java source files will be removed")
        oldfiles.each {
            LOGGER.info("    {}", it)
        }

        String buildDirPath = project.slice.output.getPath()
        oldfiles.each {
            String parent = it.getParent()
            if(!parent.startsWith(buildDirPath)) {
                LOGGER.info("Not removing {} as it is outside the build dir {}", it, buildDirPath)
            } else {
                it.delete()
            }
        }

        // Update the build state.
        state.slice = timestamps
        state.generated = generated

        // Write the new dependencies file.
        writeFreezeJBuildState(stateFile, state)
	}

	def getFreezejGenerated(freezej) {
		def files = []
        freezej.dict.each {
        	files.add(it.javaType)
        }

		freezej.index.each {
			files.add(it.javaType)
        }

        // Convert each file name from java package convention to file name.
        files = files.collect {
        	it.tr('.', '/') + ".java"
        }
        // Convert to a file with the generated path.
        files = files.collect {
        	new File(project.slice.output, it)
        }

        return files
	}

    // Executes slice2java to determine the slice file dependencies.
    // Returns a dictionary of A  -> [B] where A depends on B.
    def getFreezejDependencies(freezej, files) {
		def command = buildFreezeJCommandLine(freezej)
        command.add("--depend-xml")

        files.each {
            command.add(it.getAbsolutePath() )
        }

        LOGGER.info("Processing dependencies:\n{}", command);

        def sout = new StringBuffer()
        def serr = new StringBuffer()

        def env = addLdLibraryPath()
        def p = command.execute(env, null)
        p.waitForProcessOutput(sout, serr)
        if (p.exitValue() != 0) {
            println serr.toString()
            def slice2freezej = getSlice2FreezeJ()
            throw new GradleException("${slice2freezej} command failed: " + p.exitValue())
        }

        return parseSliceDependencyXML(new XmlSlurper().parseText(sout.toString()))
    }

	// Run slice2java. Returns a dictionary of A -> [B] where A is a slice file,
    // and B is the list of produced java source files.
    def executeSlice2Freezej(freezej, files) {
        def command = buildFreezeJCommandLine(freezej)
        files.each {
            command.add(it.getAbsolutePath() )
        }

        LOGGER.info("Processing slice:\n{}", command);

        def sout = new StringBuffer()
        def serr = new StringBuffer()

        def env = addLdLibraryPath()
        def p = command.execute(env, null)
        p.waitForProcessOutput(sout, serr)
        if (p.exitValue() != 0) {
            println serr.toString()
            def slice2freezej = getSlice2FreezeJ()
            throw new GradleException("${slice2freezej} command failed: " + p.exitValue())
        }
        return getFreezejGenerated(freezej)
    }

    def buildFreezeJCommandLine(freezej) {
        def command = []
        command.add(getSlice2FreezeJ());
    	command.add("--output-dir=" + project.slice.output.getAbsolutePath())
        command.add('-I' + getIceSliceDir())
        freezej.include.each {
            command.add('-I' + it)
        }

		freezej.args.split().each {
            command.add(it)
        }

        freezej.dict.each {
        	def javaType = it.javaType
        	command.add("--dict")
            command.add(javaType + "," + it.key + "," + it.value)
        	it.index.each {
                command.add("--dict-index")
				def buf = new StringBuffer()
				buf << javaType
				if(it.containsKey('member')) {
					buf << ','
					buf << it['member']
				}
				if(it.containsKey('casesensitive')) {
					buf << ','
					if(it['casesensitive']) {
						buf << "case-sensitive"
					}else {
						buf << "case-insensitive"
					}
				}
				command.add(buf.toString())
        	}
        }

		freezej.index.each {
            command.add("--index")
			def buf = new StringBuffer()
			buf << it.javaType
			buf << ','
			buf << it.type
			buf << ','
			buf << it.member
			buf << ','
			if(it.casesensitive) {
				buf << "case-sensitive"
			}else {
				buf << "case-insensitive"
			}
        	command.add(buf.toString())
        }
        return command
    }

	def writeFreezeJBuildState(dependencyFile, state) {
        def writer = new StringWriter()
        def xml = new MarkupBuilder(writer)
        xml.build {
            state.slice.each {
                def key = it.key
                def value = it.value
                xml.source("name": key, "timestamp": value)
            }
            state.generated.each {
                xml.generated("name": it)
            }
        }
        dependencyFile.write(writer.toString())
    }

    // Process the written slice file which is of the format:
    //
    // <dependencies>
    //   <source name="A.ice" timestamp="XXXX">
    //     <file name="Demo/Foo.java"/>
    //   </source>
    //   <source name="Hello.ice">
    //   </source>
    // </dependencies>
    def parseFreezeJBuildState(xml) {
        if(xml.name() != "build") {
            throw new GradleException("malformed XML: expected `dependencies'");
        }

		def state = new FreezeJBuildState()
        xml.children().each {
            if(it.name() == "source") {
                def source = it.attributes().get("name")
                def timestamp = it.attributes().get("timestamp")
                state.slice.put(source, timestamp)
            } else if(name() == "generated") {
                def source = it.attributes().get("name")
                state.generated.add(source)
            }
        }
        return state
    }

    private class Dependency {
        List<File> files;
        long timestamp;
    };

    def processJavaSet(Java java) {
    	java.args = java.args.stripIndent()
        def sourceFiles
        if (java.files == null) {
            sourceFiles = project.fileTree(dir: java.srcDir).include('**/*.ice')
        } else {
            sourceFiles = java.files
        }

        // Dictionary to A -> timestamp, where A is a slice file
        // we want to process.
        def timestamps = getTimestamps(sourceFiles)

        // Dictionary of A  -> [B] where A depends on B.
        def sliceDependencies = [:]
        if(!sourceFiles.isEmpty()) {
            getDependencies(java, sourceFiles)
        }

        // Dictionary of A -> timestamp, [B] where A is a slice file,
        // timestamp is the  modified time for A at last build and B is the
        // list of produced java source files.
        def dependencies = [:]
        def dependencyFile = new File(project.buildDir, java.name + ".d.xml")
        if(dependencyFile.isFile()) {
            try {
                dependencies = parseDependencies(new XmlSlurper().parse(dependencyFile));
            }
            catch(Exception ex) {
                LOGGER.info("Dependencies file {} not found, or invalid", dependencyFile);
                println ex
            }
        }

        def toBuild = new HashSet<>()
        timestamps.each {
            def d = dependencies[it.key]
            // If the dependency doesn't exist, or the timestamp
            // is older than the current timestamp then we need
            // to build the source file.
            if(d == null || d.timestamp < it.value) {
                toBuild.add(it.key);
            }
        }

        // Bail out if there is nothing to do (in theory this should not occur)
        if(toBuild.isEmpty()) {
            LOGGER.info("Nothing to do");
            return;
        }

        // Expand the toBuild list from the dependencies and
        // dependencies of dependencies.

        // First build a list of reverse dependencies.
        def revdep = buildReverseDependencies(sliceDependencies)

        // Add each of the dependencies. Use clone() as it changes the list.
        // Is there a better way?
        toBuild.clone().each {
            revdep[it].each {
                toBuild.add(it)
            }
        }

        LOGGER.info("Running slice2java on the following slice files")
        toBuild.each {
            LOGGER.info("    {}", it)
        }

        // Dictionary of A -> [B] where A is a slice file, and B is
        // the list of produced java source files.
        def generated = executeSlice2Java(java, toBuild)

        // Gather up the list of source files that we previously built for those
        // files which are building.
        def oldfiles = new HashSet<>()
        toBuild.each {
            Dependency d = dependencies[it]
            if(d != null) {
                d.files.each {
                    oldfiles.add(it)
                }
            }
        }

        // Add to the oldfiles list those slice files which no longer are in
        // the build list.
        dependencies.each {
            if(!sourceFiles.contains(it.key)){
                it.value.files.each {
                    oldfiles.add(it)
                }
            }
        }

        // Remove all source files that we have generated from the list of
        // sources file that we previously have built.
        generated.each {
            it.value.each {
                oldfiles.remove(it)
            }
        }

        LOGGER.info("The following generated java source files can be removed")
        oldfiles.each {
            LOGGER.info("    {}", it)
        }

        String buildDirPath = project.slice.output.getPath()
        oldfiles.each {
            String parent = it.getParent()
            if(!parent.startsWith(buildDirPath)) {
                LOGGER.info("Not removing {} as it is outside the build dir {}", it, buildDirPath)
            } else {
                it.delete()
            }
        }

        // Update the dependencies.
        toBuild.each {
            if(!dependencies.containsKey(it)) {
                dependencies[it] = new Dependency();
            }
            Dependency d = dependencies[it]
            d.timestamp = timestamps[it]
            d.files = generated[it]
        }

        // Write the new dependencies file.
        writeDependencies(dependencyFile, dependencies)
    }

    // Run slice2java. Returns a dictionary of A -> [B] where A is a slice file,
    // and B is the list of produced java source files.
    def executeSlice2Java(java, files) {
        def slice2java = getSlice2Java()
        def command = []
        command.add(slice2java);
        command.add("--list-generated")
        command.add("--output-dir=" + project.slice.output.getAbsolutePath())
        command.add('-I' + getIceSliceDir())
        java.include.each {
            command.add('-I' + it)
        }

		java.args.split().each {
            command.add(it)
        }

        files.each {
            command.add(it.getAbsolutePath() )
        }

        LOGGER.info("Processing slice:\n{}", command);

        def sout = new StringBuffer()
        def serr = new StringBuffer()

        def env = addLdLibraryPath()
        def p = command.execute(env, null)
        p.waitForProcessOutput(sout, serr)
        if (p.exitValue() != 0) {
            println serr.toString()
            throw new GradleException("${slice2java} command failed: " + p.exitValue())
        }
        return parseGeneratedXML(new XmlSlurper().parseText(sout.toString()))
    }

    // Executes slice2java to determine the slice file dependencies.
    // Returns a dictionary of A  -> [B] where A depends on B.
    def getDependencies(java, files) {
        def slice2java = getSlice2Java()
        def command = []
        command.add(slice2java);
        command.add("--depend-xml")
        command.add('-I' + getIceSliceDir())
        java.include.each {
            command.add('-I' + it)
        }
    	java.args.split().each {
            command.add(it)
    	}

        files.each {
            command.add(it.getAbsolutePath() )
        }

        LOGGER.info("Processing dependencies:\n{}", command);

        def sout = new StringBuffer()
        def serr = new StringBuffer()

        def env = addLdLibraryPath()
        def p = command.execute(env, null)
        p.waitForProcessOutput(sout, serr)
        if (p.exitValue() != 0) {
            println serr.toString()
            throw new GradleException("${slice2java} command failed: " + p.exitValue())
        }

        return parseSliceDependencyXML(new XmlSlurper().parseText(sout.toString()))
    }

    // Given a map of A->B,C,D where A depends on B, C, D produce a map
    // of B->A, C->A, D->A meaning that if B is touched A must be rebuilt.
    //
    // The sliceDependencies map is already transitive in that if A->B and B->C then
    // A already is A->B,C
    def buildReverseDependencies(sliceDependencies) {
        def revdep = [:]
        sliceDependencies.each {
            def source = it.key
            it.value.each {
                if(!revdep.containsKey(it)) {
                   revdep[it] = new HashSet()
                }
                revdep[it].add(source)
            }
        }
        return revdep;
    }

    // Return a dictionary of A->timestamp for each input file.
    def getTimestamps(files) {
        def timestamps = [:]
        files.each {
            if(!it.isFile()) {
                throw new GradleException("Cannot stat " + it)
            }
            timestamps[it] = it.lastModified()
        }
        return timestamps
    }

    def writeDependencies(dependencyFile, dependencies) {
        def writer = new StringWriter()
        def xml = new MarkupBuilder(writer)
        xml.dependencies {
            dependencies.each {
                def key = it.key
                def value = it.value
                xml.source("name": key, "timestamp": value.timestamp) {
                    value.files.each {
                        xml.file("name": it)
                    }
                }
            }
        }

        dependencyFile.write(writer.toString())
    }

    // Process the written slice file which is of the format:
    //
    // <dependencies>
    //   <source name="A.ice" timestamp="XXXX">
    //     <file name="Demo/Foo.java"/>
    //   </source>
    //   <source name="Hello.ice">
    //   </source>
    // </dependencies>
    def parseDependencies(xml) {
        if(xml.name() != "dependencies") {
            throw new GradleException("malformed XML: expected `dependencies'");
        }

        def dependencies =[:]
        xml.children().each {
            if(it.name() == "source") {
                def source = it.attributes().get("name")
                def timestamp = it.attributes().get("timestamp")
                def files = []
                it.children().each {
                    if(it.name() == "file") {
                        def file = new File(it.attributes().get("name"))
                        files.add(file)
                    }
                }
                Dependency d = new Dependency()
                d.files = files
                d.timestamp = timestamp.toLong()
                dependencies.put(new File(source), d)
            }
        }
        return dependencies
    }

    // Process the generated XML which is of the format:
    //
    // <generated>
    //   <source name="A.ice">
    //     <file name="Demo/_HelloOperations.java"/>
    //   </source>
    // </generated>
    def parseGeneratedXML(xml) {
        if(xml.name() != "generated") {
            throw new GradleException("malformed XML: expected `generated'");
        }

        def generated =[:]
        xml.children().each {
            if(it.name() == "source") {
                def source = it.attributes().get("name")
                def files = []
                it.children().each {
                    if(it.name() == "file") {
                        def name = new File(it.attributes().get("name"))
                        files.add(name)
                    }
                }
                generated.put(new File(source), files);

            }
        }
        return generated
    }

    // Parse the dependency XML which is of the format:
    //
    // <dependencies>
    //   <source name="A.ice">
    //     <dependsOn name="Hello.ice"/>
    //   </source>
    //   <source name="Hello.ice">
    //   </source>
    // </dependencies>
    def parseSliceDependencyXML(xml) {
        if(xml.name() != "dependencies") {
            throw new GradleException("malformed XML");
        }

        def dependencies =[:]
        xml.children().each {
            if(it.name() == "source") {
                def source = it.attributes().get("name")
                def files = []
                it.children().each {
                    if(it.name() == "dependsOn") {
                        def dependsOn = new File(it.attributes().get("name"))
                        files.add(dependsOn)
                    }
                }
                dependencies.put(new File(source), files);
            }
        }
        return dependencies
    }

    def getSlice2Java() {
        def slice2java = "slice2java"
        def iceHome = getIceHome()
        if (iceHome != null) {
            slice2java = iceHome + File.separator + "bin" + File.separator + "slice2java"
        }
        return slice2java
    }

    def getSlice2FreezeJ() {
        def slice2freezej = "slice2freezej"
        def iceHome = getIceHome()
        if (iceHome != null) {
            slice2freezej = iceHome + File.separator + "bin" + File.separator + "slice2freezej"
        }
        return slice2freezej
    }

    def getIceHome() {
        // Check if plugin property is set
        def iceHome = project.slice.iceHome
        if (iceHome != null) {
            return iceHome
        }

        // Check if environment variable is set
        if (iceHome == null) {
            def env = System.getenv()
            iceHome = env['ICE_HOME']
            if (iceHome != null) {
                return iceHome
            }
        }

        // Check default install locations
        if (iceHome == null) {
            def os = System.properties['os.name']
            if(os == "Mac OS X") {
                iceHome = "/usr/local"
            } else if(os.contains("Windows")) {
                def arch1 = env['PROCESSOR_ARCHITECTURE']
                def arch2 = env['PROCESSOR_ARCHITEW6432']
                if (arch1 == "AMD64" || arch1 == "IA64" || arch2 == "AMD64" || arch2 == "IA64") {
                    programFiles = System.getenv('ProgramFiles(x86)')
                } else {
                    programFiles = System.getenv('ProgramFiles')
                }
                iceHome = programFiles + File.separator + "ZeroC" + File.separator + "Ice-3.6.0"
            } else {
                iceHome = "/usr"
            }
        }

        if (!File(iceHome + File.separator + "bin" + File.separator + "slice2java").exists() ||
            !File(iceHome + File.separator + "bin" + File.separator + "slice2java.exe").exists())
        {
            throw new GradleException("Could not find Ice installation");
        }

        return iceHome
    }

    def getIceSliceDir() {
        def iceHome = getIceHome()
        if (project.slice.srcDist) {
            return iceHome + File.separator + ".." + File.separator + "slice"
        }

        def os = System.properties['os.name']
        if (os == "Mac OS X") {
            if (iceHome == "/usr/local") {
                return "/usr/local/share/slice"
            }
        } else if (iceHome == "/usr") {
            return "/usr/share/Ice-3.6.0/slice"
        }
        return iceHome + File.separator + "slice"
    }

    def addLdLibraryPath() {
        def iceInstall = getIceHome()
        def env = System.getenv()

        def ldLibPathEnv = null
        def ldLib64PathEnv = null
        def libPath = new File(iceInstall + File.separator + "lib").toString()
        def lib64Path = null

        def os = System.properties['os.name']
        if(os == "Mac OS X") {
            ldLibPathEnv = "DYLD_LIBRARY_PATH"
        } else if(os.contains("Windows")) {
            //
            // No need to change the PATH environment variable on Windows, the DLLs should be found
            // in the translator local directory.
            //
        } else {
            ldLibPathEnv = "LD_LIBRARY_PATH"
            ldLib64PathEnv = "LD_LIBRARY_PATH"
            lib64Path = new File(iceInstall + File.separator + "lib64").toString()

            if(new File(iceInstall + File.separator + "lib" + File.separator + "i386-linux-gnu").exists())
            {
                libPath = new File(iceInstall + File.separator +
                                   "lib" + File.separator +
                                   "i386-linux-gnu").toString()
            }

            if(new File(iceInstall + File.separator + "lib" + File.separator + "x86_64-linux-gnu").exists())
            {
                lib64Path = new File(iceInstall + File.separator +
                                     "lib" + File.separator +
                                     "x86_64-linux-gnu").toString();
            }
        }

        def newEnv = [:]
        if(ldLibPathEnv != null) {
            if(ldLibPathEnv.equals(ldLib64PathEnv)) {
                libPath = libPath + File.pathSeparator + lib64Path;
            }

            def envLibPath = env[ldLibPathEnv]
            if(envLibPath != null) {
                libPath = libPath + File.pathSeparator + envLibPath
            }
            newEnv[ldLibPathEnv] = libPath
        }

        if(ldLib64PathEnv != null && !ldLib64PathEnv.equals(ldLibPathEnv)) {
            def envLib64Path = env[ldLib64PathEnv]
            if(envLib64Path != null) {
                lib64Path = lib64Path + File.pathSeparator + envLib64Path
            }
            newEnv[ldLib64PathEnv] = lib64Path
        }

        return newEnv.collect { k, v -> "$k=$v" }
    }
}
