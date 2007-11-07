import fileinput

def commentOutLexYacc(target, makefile):
    f = fileinput.input("$makefile", True)
    inComment = False
    inClean = False
    for x in f:
        if not x.startswith("\t") and x.find("$base") != -1 and x.find("$base" + ".o") == -1:
            inComment = True 
        elif x.startswith("clean::"):
            inClean = True
        elif len(x.strip()) == 0:
            inClean = False
            inComment = False
        x = x.rstrip('\n')
        if (inComment or (inClean and x.find("$base") != -1)) and not x.startswith('#'):
                print '#',x
        else:
            print x
    f.close()
