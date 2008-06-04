#!/usr/bin/env python

import os, sys, shutil, fnmatch, re, glob
from stat import *

#
# Program usage.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options]"
    print
    print "Options:"
    print "-h    Show this message."
    print

#
# Returns the new copyright
#
def copyright(commentMark, patchIceE):
    result = [ ]
    result.append(commentMark + " **********************************************************************\n")
    result.append(commentMark + "\n")
    result.append(commentMark + " Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.\n")
    result.append(commentMark + "\n")
    if patchIceE == True:
        result.append(commentMark + " This copy of Ice-E is licensed to you under the terms described in the\n")
        result.append(commentMark + " ICEE_LICENSE file included in this distribution.\n")
    else:
        result.append(commentMark + " This copy of Ice is licensed to you under the terms described in the\n")
        result.append(commentMark + " ICE_LICENSE file included in this distribution.\n")
    result.append(commentMark + "\n")
    result.append(commentMark + " **********************************************************************\n")
    return result

#
# Replace one copyright
#
def replaceCopyright(file, commentMark, commentBegin, commentEnd, newCopyrightLines):
    oldFile = open(file, "r")
    oldLines = oldFile.readlines()

    done = 0
    commentFound = 0
    copyrightFound = 0

    beforeCopyrightLines = []
    oldCopyrightLines = []
    newLines = []

    justDone = 0

    if commentBegin == "":
        for x in oldLines:
            if not commentFound and (not x.startswith(commentMark) or x.startswith("#!/usr/bin/env")):
                beforeCopyrightLines.append(x)
            elif not done and x.startswith(commentMark):
                commentFound = 1
                if not copyrightFound and x.lower().find("copyright") != -1:
                    copyrightFound = 1
                # skip this comment line 
                oldCopyrightLines.append(x)            
            else:
                if not done:
                    done = 1
                    justDone = 1
                
                # Eliminate double blank lines after copyright (bug introduced by previous fixCopyright script)
                if justDone == 1:
                    newLines.append(x)
                    if x != "\n":
                        justDone = 0
                    else:
                        justDone = 2
                elif justDone == 2:
                    if x != "\n":
                         newLines.append(x)
                    justDone = 0                    
                else:
                    newLines.append(x)
    else:
         for x in oldLines:
            if not done:
                if x.startswith(commentBegin):
                    commentFound = 1
                if commentFound:
                   if not copyrightFound and x.find("Copyright") != -1:
                       copyrightFound = 1
                          
                   # skip this comment line 
                   if x.find(commentEnd) != -1:
                       done = 1
                       justDone = 1
                else:
                    beforeCopyrightLines.append(x)    
            else:
                # Eliminate double blank lines after copyright (bug introduced by previous fixCopyright script)
                if justDone == 1:
                    newLines.append(x)
                    if x != "\n":
                        justDone = 0
                    else:
                        justDone = 2
                elif justDone == 2:
                    if x != "\n":
                         newLines.append(x)
                    justDone = 0                    
                else:
                    newLines.append(x)


    oldFile.close()

    if copyrightFound and newCopyrightLines != oldCopyrightLines:

        mode = os.stat(file)[ST_MODE]

        origFile = file + ".orig"
        shutil.copy2(file, origFile)

        newFile = open(file, "w")
        newFile.writelines(beforeCopyrightLines)
        newFile.writelines(newCopyrightLines)

        #
        # Hack to keep the .err files
        #
        if fnmatch.fnmatch(file, "*test/Slice/errorDetection/*.ice") > 0: 
                newFile.write("\n")

        newFile.writelines(newLines)
        newFile.close()        

        os.chmod(file, S_IMODE(mode))
        print "------ Replaced copyright in " + file + " -------"

        #
        # Make sure Windows Makefiles are kept in DOS format.
        #
        if fnmatch.fnmatch(file, "*.mak*") or fnmatch.fnmatch(file, "*Make.rules.bcc") or fnmatch.fnmatch(file, "*Make.rules.msvc"):
            os.popen("unix2dos " + file);

    return copyrightFound

#
# Replace alls copyrights
#
def replaceAllCopyrights(path, patchIceE, recursive):

    cppCopyright = copyright("//", patchIceE)
    mcCopyright = copyright("; //", patchIceE)
    makefileCopyright = copyright("#", patchIceE)
    vbCopyright = copyright("'", patchIceE)
    pythonCopyright = makefileCopyright
    rubyCopyright = makefileCopyright
    xmlCopyright = []
    xmlCopyright.append("<!--\n");
    xmlCopyright.extend(copyright("", patchIceE))
    xmlCopyright.append("-->\n");
    
    files = os.listdir(path)
    for x in files:
        fullpath = os.path.join(path, x);
        if os.path.isdir(fullpath) and not os.path.islink(fullpath):
            if recursive:
                replaceAllCopyrights(fullpath, patchIceE, True)
        else:
            
            commentMark = ""
            commentBegin = ""
            commentEnd = ""
            copyrightLines = []
            skip = 0

            if x == "config" or x == ".depend" or x == ".dummy" or fnmatch.fnmatch(x, "*.dsp")   or fnmatch.fnmatch(x, "*.sln") or fnmatch.fnmatch(x, "*.vdproj") or fnmatch.fnmatch(x, "*.err") or fnmatch.fnmatch(x, "*.class") or fnmatch.fnmatch(x, "*.ico") or fnmatch.fnmatch(x, "*.gif") or fnmatch.fnmatch(x, "*.jpg") or fnmatch.fnmatch(x, "*.orig"):
                print "Skipping file " + fullpath + ": no copyright needed"
                skip = 1  
            elif fnmatch.fnmatch(x, "Make*") or fnmatch.fnmatch(x, "*.properties"):
                commentMark = "#"
                copyrightLines = makefileCopyright
            elif fnmatch.fnmatch(x, "*.h") or fnmatch.fnmatch(x, "*.cpp") or fnmatch.fnmatch(x, "*.cs") or fnmatch.fnmatch(x, "*.ice") or fnmatch.fnmatch(x, "*.java") or fnmatch.fnmatch(x, "*.l") or fnmatch.fnmatch(x, "*.y"):
                commentMark = "//"
                copyrightLines = cppCopyright
            elif fnmatch.fnmatch(x, "*.py"):
                commentMark = "#"
                copyrightLines = pythonCopyright
            elif fnmatch.fnmatch(x, "*.def"):
                commentMark = "#"
                copyrightLines = pythonCopyright
            elif fnmatch.fnmatch(x, "*.cnf"):
                commentMark = "#"
                copyrightLines = pythonCopyright
            elif fnmatch.fnmatch(x, "*.rb"):
                commentMark = "#"
                copyrightLines = rubyCopyright
            elif fnmatch.fnmatch(x, "*.mc"):
                commentMark = "; //"
                copyrightLines = mcCopyright
            elif fnmatch.fnmatch(x, "*.vb"):
                commentMark = "'"
                copyrightLines = vbCopyright
            elif fnmatch.fnmatch(x, "*.xml"):
                commentBegin = "<!--"
                commentEnd = "-->"
                copyrightLines = xmlCopyright
            else:
                print "***** Skipping file " + fullpath + ": unknown type" 
                skip = 1

            if not skip:
                if replaceCopyright(fullpath, commentMark, commentBegin, commentEnd, copyrightLines) == 0:
                    print "***** WARNING: Did not find copyright in " + fullpath                   

#   
# Find files matching a pattern.
#   
def find(path, patt):
    result = [ ]
    files = os.listdir(path)
    for x in files:
        fullpath = os.path.join(path, x);
        if os.path.isdir(fullpath) and not os.path.islink(fullpath):
            result.extend(find(fullpath, patt))
        elif fnmatch.fnmatch(x, patt):
            result.append(fullpath)
    return result

def fileMatchAndReplace(filename, matchAndReplaceExps, warn=True):

    mode = os.stat(filename)[ST_MODE]
    oldConfigFile = open(filename, "r")
    newConfigFile = open(filename + ".new", "w")

    #
    # Compile the regular expressions
    #
    regexps = [ ]
    for (regexp, replace) in matchAndReplaceExps:
        regexps.append((re.compile(regexp), replace))

    #
    # Search for the line with the given regular expressions and
    # replace the matching string
    #
    updated = False
    for line in oldConfigFile.readlines():
        for (regexp, replace) in regexps:
            match = regexp.search(line)
            if match != None:
                oldLine = line
                line = oldLine.replace(match.group(1), replace)
                updated = True
                break
        newConfigFile.write(line)

    newConfigFile.close()
    oldConfigFile.close()

    if updated:
        print "updated " + filename
        os.rename(filename + ".new", filename)
        os.chmod(filename, S_IMODE(mode))
    elif warn:
        print "warning: " + filename + " didn't contain any copyright"
        os.unlink(filename + ".new")

#
# Main
#

for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x.startswith("-"):
        print sys.argv[0] + ": unknown option `" + x + "'"
        print
        usage()
        sys.exit(1)

ice_dir = os.path.normpath(os.path.join(os.path.dirname(__file__)))

#
# Fix copyright header in files
#
replaceAllCopyrights(ice_dir, False, False)
for dir in ["slice", "cpp", "java", "cs", "vb", "php", "py", "rb", "sl", "demoscript", "distribution", "config", "certs"]:
    home = os.path.join(ice_dir, dir)
    if home:
        replaceAllCopyrights(home, False, True)

for dir in ["cppe", "javae"]:
    home = os.path.join(ice_dir, dir)
    if home:
        replaceAllCopyrights(home, True, True)

#
# Fix various other files that have copyright info in them that
# are not taken care of above.
#
vpatMatch = "20[0-9][0-9]-(20[0-9][0-9]) ZeroC"
copyright = "2008"

files = find(ice_dir, "*.rc")
files += find(ice_dir, "*LICENSE")
files += find(os.path.join(ice_dir, "cpp", "src"), "Gen.cpp")
files += find(os.path.join(ice_dir, "cpp", "src"), "Parser.cpp")
files += find(os.path.join(ice_dir, "cpp", "src", "Slice"), "*Util.cpp")
files += [os.path.join(ice_dir, "cpp", "src", "ca", "iceca")]
files += [os.path.join(ice_dir, "cpp", "doc", "symboltree.js")]
files += [os.path.join(ice_dir, "cpp", "demo", "Freeze", "backup", "backup")]
files += find(os.path.join(ice_dir, "cpp"), "*.bat")
files += [os.path.join(ice_dir, "cpp", "test", "IceSSL", "certs", "makecerts")]
files += [os.path.join(ice_dir, "java", "bin", "icegridgui.rpm")]
files += [os.path.join(ice_dir, "java", "src", "IceGridGUI", "Coordinator.java")]
files += find(os.path.join(ice_dir, "java", "resources", "IceGridAdmin"), "icegridadmin_content_*.html")
files += [os.path.join(ice_dir, "config", "makeprops.py")]
files += find(os.path.join(ice_dir), "AssemblyInfo.cs")
files += find(os.path.join(ice_dir, "sl"), "*.as*x")
files += find(os.path.join(ice_dir, "distribution", "src", "rpm"), "*")
files += find(os.path.join(ice_dir, "php"), "*.php")

for f in files:
    fileMatchAndReplace(f, [(vpatMatch, copyright)])
