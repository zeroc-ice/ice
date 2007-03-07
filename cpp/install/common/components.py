#
# Defines Ice components
#

import ConfigParser, sys, os, logging, fnmatch, os.path, shutil, re, pprint

def listFileLists():
    """Information routine for getting lists of file lists from a components file"""
    cfg = ConfigParser.SafeConfigParser()
    cfg.read("./components/components.ini")
    for f in cfg.sections():
        try:
            if cfg.getint(f, "active") == 1:
                for item, value in cfg.items(f):
                    if item.startswith("filelist"):
                        print value
        except ConfigParser.NoOptionError:
            continue

class StageFileError:
    """Thrown when there is a problem with the stage configuration file."""
    def __init__(self, msg = None):
        self.msg = msg

    def __str__(self):
        return repr(self.msg)

class ComponentDefError:
    """Indicates a component definition file has an error that provides
    proper interpretation"""

    def __init__(self, msg = None):
        self.msg = msg

    def __str__(self):
        return repr(self.msg)

class FileSpecError(ComponentDefError):
    """Indicates a filespec component definition file has a syntactical
    error"""
    def __init__(self, msg = None):
        ComponentDefError.__init__(self, msg)

def recursiveListing(path):
    """Provides a recursive directory listing based in path""" 
    result = []
    files = os.listdir(path)
    for x in files:
        fullpath = os.path.join(path, x);
        if os.path.isdir(fullpath) and not os.path.islink(fullpath):
            result.append(fullpath)
            result.extend(recursiveListing(fullpath))
        else:
            result.append(fullpath)
    return result

def fixVersion(file, argsHash):
    if not argsHash.has_key('version'):
        print "fixVersion unable to execute, no version information available"
        return

    version = argsHash['version']
    libversion = argsHash['dllversion']

    origfile = file + ".orig"
    os.rename(file, origfile)
    oldFile = open(origfile, "r")
    newFile = open(file, "w")
    line = oldFile.read();
    line = re.sub("@ver@", version, line)
    line = re.sub("@libver@", libversion, line)
    newFile.write(line)
    newFile.close()
    oldFile.close()
    os.remove(origfile)

class FileSpecWorker:
    def __init__(self, id, source, dest, processors):
        self.id = id
        self.source = source
        self.dest = dest
        self.include = []
        self.exclude = []
        self.explicit = []
        self.processors = []
        for p in processors:
            if globals().has_key(p.strip()):
                self.processors.append(p.strip())
            else:
                print "Possible component configuration error:"
                print "  Section %s is configured with non-existant processor %s." % (id, p.trim())
        
    def add(self, filename):
        parts = filename.split("=")
        if len(parts) < 2:
            #
            # This line doesn"t have a separator and so assume its an
            # explicit listing 
            #
            self.explicit.append(filename)
            return

        if parts[0].startswith("include"):
            self.include.append(parts[1].strip())
        elif parts[0].startswith("exclude"):
            self.exclude.append(parts[1].strip())
        elif parts[0].startswith("expect"):
            pass
        else:
            raise FileSpecError("Line \'%s\' does not match filespec schema." % filename)

    def execute(self, defaults, fake = False):
        """Copy all of the specified files."""
        recursiveIncludes = []
        recursiveExcludes = []
        
        midmatchIncludes = []
        midmatchExcludes = []

        localIncludes = []
        localExcludes = []

        for f in self.include:
            if f.startswith("**/"):
                if f.endswith("/**"):
                    midmatchIncludes.append(".*%s.*" %f[3:len(f) -3].replace('/', '.'))
                else:
                    recursiveIncludes.append("*%s" % f[3:])
            else:
                if f.endswith("/**"):
                    midmatchIncludes.append("%s.*" %f[0:len(f) -3].replace('/', '.'))
                else:
                    localIncludes.append(f)

        for f in self.exclude:
            if f.startswith("**/"):
                if f.endswith("/**"):
                    midmatchExcludes.append(".*%s.*" % f[3:len(f) -3].replace('/', '.'))
                else:
                    recursiveExcludes.append("*%s" % f[3:])
            else:
                if f.endswith("/**"):
                    midmatchExcludes.append("%s.*" %f[0:len(f) -3].replace('/', '.'))
                else:
                    localExcludes.append(f)

        logging.debug('localIncludes: ' +  str(localIncludes))
        logging.debug('localExcludes: ' + str(localExcludes))
        logging.debug('recursiveIncludes: ' + str(recursiveIncludes))
        logging.debug('recursiveExcludes: ' + str(recursiveExcludes))
        logging.debug('midmatchIncludes: ' + str(midmatchIncludes))
        logging.debug('midmatchExcludes: ' + str(midmatchExcludes))

        fullListing = []
        result = []
        files = os.listdir(self.source)

        for f in files:
            fullpath = os.path.join(self.source, f);
            if os.path.isdir(fullpath) and not os.path.islink(fullpath):
                fullListing.extend(recursiveListing(fullpath))
                continue

            for p in localIncludes + recursiveIncludes:
                if fnmatch.fnmatch(f, p):
                    found = False
                    for x in localExcludes + recursiveExcludes:
                        if fnmatch.fnmatch(f, x):
                            found = True
                            break
                    if not found: result.append(f)

        inmatches = []
        for p in recursiveIncludes:
            inmatches.extend(fnmatch.filter(fullListing, p))

        inSet = set(inmatches)

        for p in midmatchIncludes:
            r = re.compile(p)
            inmatches = []
            for f in fullListing(f):
                rel = f[len(self.source):].strip('\\/')
                if not r.match(rel) == None:
                    inmatches.append(f)
            inSet = inSet.union(set(inmatches))

        outmatches = []
        for x in recursiveExcludes:
            outmatches.extend(fnmatch.filter(fullListing, x))

        outSet = set(outmatches)

        for x in midmatchExcludes:
            r = re.compile(x)
            outmatches = []
            for f in fullListing:
                rel = f[len(self.source):].strip('\\/')
                if not r.match(rel) == None:
                    outmatches.append(f)
            outSet = outSet.union(set(outmatches))


        #
        # Using sets is the "easiest" way to do this. If Python's set
        # implementation is/gets buggy then this needs to be written
        # "longhand".
        #
        diff = inSet - outSet
        result.extend(list(diff))

        for i in range(0, len(result)):
            if result[i].startswith(self.source):
                result[i] = result[i][len(self.source):].strip('\\/')

        result.sort()
        result.extend(self.explicit)

        if fake: 
            for f in result:
                print "Copying %s from %s to %s" % (f, self.source, self.dest)
            return

        if logging.getLogger().getEffectiveLevel() == logging.DEBUG:
            logging.debug("Files to be copied:")
            for f in result:
                logging.debug(f)

        #
        # Detect changes in file list and confirm that changes are okay.
        #
        cachedResults = None
        cacheFilename = os.path.join(os.path.dirname(__file__), "%s.cache" % self.id)
        if os.path.exists(cacheFilename):
            cacheFile = open(cacheFilename, "r+b")
            cachedResults = cacheFile.readlines()
            cacheFile.close()

        if cachedResults != None:
            for i in range(0, len(cachedResults)):
                cachedResults[i] = cachedResults[i].rstrip()
            previous = set(cachedResults)
            current = set(result)
            added = current - previous
            removed = previous - current
            if len(added) > 0:
                print "Additions detected:"
                pprint.pprint(list(added))
                print "Accept (y/n):",
                while True:
                    answer = sys.stdin.read(1)
                    if answer in ["Y", "y"]:
                        break
                    elif answer in ["N", "n"]:
                        print "\nAborting..."
                        rejects = open("%s.rejects" % self.id, "w")
                        rejects.write("Added:\n")
                        rejects.writelines(pprint.pprint(list(added)))
                        rejects.write("Deleted:\n")
                        rejects.writelines(pprint.pprint(list(removed)))
                        rejects.close()
                        sys.exit(1)
                
            if len(removed) > 0:
                print "Deletions detected:"
                pprint.pprint(list(removed))
                print "Accept (y/n):",
                while True:
                    answer = sys.stdin.read(1)
                    if answer in ["Y", "y"]:
                        break
                    elif answer in ["N", "n"]:
                        print "\nAborting..."
                        rejects = open("%s.rejects" % self.id, "w")
                        rejects.write("Added:\n")
                        rejects.writelines(pprint.pprint(list(added)))
                        rejects.write("Deleted:\n")
                        rejects.writelines(pprint.pprint(list(removed)))
                        rejects.close()
                        sys.exit(1)

        cacheFile = open(cacheFilename, "w+b")
        for f in result:
            cacheFile.write(f)
            cacheFile.write("\n")
        cacheFile.close()

        #
        # Scan filename to see if matches one of our designated
        # 'convert to dos file format' name patterns. (These are
        # regex patterns, not patterns for filename globbing).
        #
        textFiles = [".*README.*", ".*Makefile.mak", ".*LICENSE.*"]
        textFileScanner = None
        expression = ""
        for p in textFiles:
            if expression != "":
                expression = expression + "|"
            expression = expression + p 
        textFileScanner = re.compile(expression)

        for f in result:
            #
            # an f, prefix means flatten.
            #
            flatten = False
            current = f

            if f.startswith('f,'):
                flatten = True
                current = current[2:]
            current = current % defaults
                
            targetDirectory = self.dest 
            targetFile = os.path.basename(current)
            if not flatten:
                targetDirectory = os.path.join(self.dest, os.path.dirname(current))

            if not os.path.exists(targetDirectory):
                os.makedirs(targetDirectory)

            s = os.path.join(self.source, current)
            d = os.path.join(targetDirectory, targetFile)
            try:
                if os.path.isdir(s):
                    os.mkdir(d)
                else:
                    shutil.copy2(s, d) 
                    isTextFile = (textFileScanner.search(d) != None)
                    if isTextFile:
                        # So how do I do the conversion.
                        tmp = open(d + ".bak", "w")
                        tmp.write(open(d, "rU").read())
                        tmp.close()
                        shutil.copy2(d + ".bak", d)
                        os.remove(d + ".bak")
                        
                    if self.processors != None and len(self.processors) > 0:
                        for p in self.processors:
                            e = globals()[p.strip()]
                            e(d, defaults)

            except IOError, e:
                logging.info('Copying %s to %s failed: %s' %  (s, d, str(e)))
                raise

def stage(filename, componentdir, stageDirectory, group, defaults):
    cfg = ConfigParser.SafeConfigParser() 
    cfg.read(filename)
    if not cfg.has_section(group):
        raise StageFileError("Section %s is missing from component list file." % group)

    sections = []
    for entry in cfg.options(group): 
        section = cfg.get(group, entry)
        if not cfg.has_section(section):
            raise StageFileError("Section %s is missing from component list file." % section)
        sections.append((entry, section))

    for stageLoc, section in sections:
        try:
            currentBase = stageDirectory
            for f in stageLoc.split('-'):
                currentBase = os.path.join(currentBase, f)

            if not os.path.exists(currentBase):
                os.makedirs(currentBase)

            elementCount = cfg.getint(section, "elements")
            if elementCount < 0:
                raise StageFileError("Section %s elements field is negative value" % section)
            
            for i in range(1, elementCount + 1):
                try:
                    if cfg.has_option(section, "targets"):
                        target = cfg.get(section, "targets", False, defaults).strip()

                        if "~" + defaults["target"] in target:
                            continue
                        elif not defaults["target"] in target:
                            continue
                        
                    source = cfg.get(section, "source%d" % i, False, defaults)
                    filelist = cfg.get(section, "filelist%d" % i, False, defaults)
                    dest = cfg.get(section, "dest%d" % i, False, defaults)
                    #
                    # Most configurations won"t have file templates. These are usually only used so that file lists can
                    # be reused but need to be slightly modified for each use case.
                    #
                    template = None
                    mapping = None
                    if cfg.has_option(section, "filetemplate%d" % i):
                        #
                        # We need to get the raw value!
                        #
                        template = cfg.get(section, "filetemplate%d" %i, True)
                        mapping = defaults 

                    processors = []
                    if cfg.has_option(section, "processor%d" % i):
                        processors = cfg.get(section, "processor%d" % i).split(',')

                    filename = os.path.join(componentdir, filelist)
                    if not (os.path.exists(filename) and os.path.isfile(filename)):
                        raise StageFileError("Component file %s does not exist or is not a file" % filename)

                    componentFile = file(filename, "r")
                    try:
                        worker = FileSpecWorker("%s.%s.%d" % (filename, section.replace(" ", "_"), i), source, os.path.join(currentBase, dest), processors)
                        for line in componentFile:
                            current = line.strip()
                            if line.startswith('#'):
                                continue

                            if len(current) == 0:
                                continue
                            if not template == None:
                                mapping['name'] = current
                                try:
                                    computedName = template % mapping
                                except:
                                    print "Mapping exception occurred with " + template + " and " + current  
                                    raise
                                    
                                logging.log(logging.DEBUG, 'Adding templatized name %s' % computedName)
                                worker.add(computedName)
                            else:
                                try:
                                    worker.add(current % defaults)
                                except Exception, e:
                                    print str(e) + ": occured while adding %s to worker in element %d in %s" % \
                                    (current, i, section)

                        if worker == None:      
                            msg = "Component file %s is empty." % filename
                            logging.warning(msg)
                        else:
                            #
                            # NOTE: set fake to true while debugging.
                            #
                            worker.execute(defaults, False)

                    finally:
                        componentFile.close()

                except ConfigParser.NoOptionError:
                    raise StageFileError("Section %s has invalid value for element %d" % (section, i))
                
        except ConfigParser.NoOptionError:
            raise StageFileError("Section %s has invalid or missing elements field" % section)

if __name__ == "__main__":
    print 'components'
