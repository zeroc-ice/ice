#!/usr/bin/env python

import os, sys, shutil, fnmatch, re, glob

#
# Program usage.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options] [path]"
    print
    print "Options:"
    print "-e    Fix version for Ice-E instead of Ice."
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
        print "------ Replaced copyright in " + file + " -------"

    return copyrightFound

#
# Replace alls copyrights
#
def replaceAllCopyrights(path, patchIceE):

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
            replaceAllCopyrights(fullpath, patchIceE)
        else:
            
            commentMark = ""
            commentBegin = ""
            commentEnd = ""
            copyrightLines = []
            skip = 0

            if x == "config" or x == ".depend" or x == ".dummy" or fnmatch.fnmatch(x, "*.dsp")   or fnmatch.fnmatch(x, "*.sln") or fnmatch.fnmatch(x, "*.vdproj") or fnmatch.fnmatch(x, "*.err") or fnmatch.fnmatch(x, "*.class") or fnmatch.fnmatch(x, "*.ico") or fnmatch.fnmatch(x, "*.gif") or fnmatch.fnmatch(x, "*.jpg") or fnmatch.fnmatch(x, "*.orig"):
                print "Skipping file " + fullpath + ": no copyright needed"
                skip = 1  
            elif fnmatch.fnmatch(x, "*.h") or fnmatch.fnmatch(x, "*.cpp") or fnmatch.fnmatch(x, "*.cs") or fnmatch.fnmatch(x, "*.ice") or fnmatch.fnmatch(x, "*.java") or fnmatch.fnmatch(x, "*.l") or fnmatch.fnmatch(x, "*.y"):
                commentMark = "//"
                copyrightLines = cppCopyright
            elif fnmatch.fnmatch(x, "*.py"):
                commentMark = "#"
                copyrightLines = pythonCopyright
            elif fnmatch.fnmatch(x, "*.def"):
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
            elif fnmatch.fnmatch(x, "Make*") or fnmatch.fnmatch(x, "*.properties"):
                commentMark = "#"
                copyrightLines = makefileCopyright
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
# Main
#

patchIceE = False

for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x == "-e":
        patchIceE = True
    elif x.startswith("-"):
        print sys.argv[0] + ": unknown option `" + x + "'"
        print
        usage()
        sys.exit(1)
    else:
        path = x

ice_dir = os.path.normpath(os.path.join(os.path.dirname(__file__)))

if patchIceE:
    icee_home = os.path.join(ice_dir, "cppe")
    if icee_home:
        replaceAllCopyrights(icee_home, True)

    iceje_home = os.path.join(ice_dir, "javae")
    if iceje_home:
        replaceAllCopyrights(iceje_home, True)
else:
    slice_home = os.path.join(ice_dir, "slice")
    if slice_home:
        replaceAllCopyrights(slice_home, False)
            
    ice_home = os.path.join(ice_dir, "cpp")
    if ice_home:
        replaceAllCopyrights(ice_home, False)

    icej_home = os.path.join(ice_dir, "java")
    if icej_home:
        replaceAllCopyrights(icej_home, False)

    icecs_home = os.path.join(ice_dir, "cs")
    if icecs_home:
        replaceAllCopyrights(icecs_home, False)

    icevb_home = os.path.join(ice_dir, "vb")
    if icevb_home:
        replaceAllCopyrights(icevb_home, False)

    icephp_home = os.path.join(ice_dir, "php")
    if icephp_home:
        replaceAllCopyrights(icephp_home, False)

    icepy_home = os.path.join(ice_dir, "py")
    if icepy_home:
        replaceAllCopyrights(icepy_home, False)

    icerb_home = os.path.join(ice_dir, "rb")
    if icerb_home:
        replaceAllCopyrights(icerb_home, False)

    icedemo_home = os.path.join(ice_dir, "demoscript")
    if icedemo_home:
        replaceAllCopyrights(icedemo_home, False)

    icedist_home = os.path.join(ice_dir, "distribution")
    if icedist_home:
        replaceAllCopyrights(icedist_home, False)
        
    config_home = os.path.join(ice_dir, "config")
    if config_home:
        replaceAllCopyrights(config_home, False)
