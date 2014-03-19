#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, shutil, signal, time

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "demoscript")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(path[0])

from demoscript import Util

def cleandb():
    shutil.rmtree("db.save", True)
    Util.cleanDbDir("db/data")
    Util.cleanDbDir("db/logs")
    for filename in [ os.path.join("db", f) for f in os.listdir("db") if f.startswith("__db") ]:
        os.remove(filename)

sys.stdout.write("cleaning databases... ")
sys.stdout.flush()
cleandb()
for d in os.listdir('.'):
    if d.startswith('hotbackup'):
        shutil.rmtree(d)
print("ok")

client = Util.spawn('./client')

sys.stdout.write("populating map... ")
sys.stdout.flush()
client.expect('Updating map', timeout=60)
time.sleep(3) # Let the client do some work for a bit.
print("ok")

sys.stdout.write("performing full backup... ")
sys.stdout.flush()
if Util.isWin32():
    backup = Util.spawn('./backup.bat full')
else:
    backup = Util.spawn('./backup full')
backup.expect('hot backup started', timeout=30)
backup.waitTestSuccess(timeout=30)
print("ok")

sys.stdout.write("sleeping 5s... ")
sys.stdout.flush()
time.sleep(5)
print("ok")

sys.stdout.write("performing incremental backup... ")
sys.stdout.flush()
if Util.isWin32():
    backup = Util.spawn('./backup.bat incremental')
else:
    backup = Util.spawn('./backup incremental')
backup.expect('hot backup started', timeout=30)
backup.waitTestSuccess(timeout=30)
print("ok")

sys.stdout.write("sleeping 30s... ")
sys.stdout.flush()
time.sleep(30)
print("ok")

assert os.path.isdir('hotbackup')

sys.stdout.write("killing client with SIGTERM... ")
sys.stdout.flush()
client.kill(signal.SIGTERM)
client.waitTestSuccess(-signal.SIGTERM)
print("ok")

sys.stdout.write("Client output: ")
print("%s" % (client.before))

sys.stdout.write("restarting client... ")
sys.stdout.flush()
cleandb()
# Annoying. shutil.copytree cannot be used since db already exists
# (and cannot be removed since it contains .gitignore.
#os.system('cp -Rp hotbackup/* db')
for root, dirnames, filesnames in os.walk("hotbackup"):
    dbroot = os.path.join("db", root[len("hotbackup")+1:])
    for d in dirnames:
        try:
            os.mkdir(os.path.join("db", d))
        except OSError:
            pass
    for f in filesnames:
        shutil.copy2(os.path.join(root, f), os.path.join(dbroot, f))
sys.stdout.flush()

client = Util.spawn('./client')
client.expect('(.*)Updating map', timeout=60)
assert client.match.group(1).find('Creating new map') == -1
print("ok")

sys.stdout.write("sleeping 5s... ")
sys.stdout.flush()
time.sleep(5)
print("ok")

sys.stdout.write("killing client with SIGTERM... ")
client.kill(signal.SIGTERM)
client.waitTestSuccess(-signal.SIGTERM)
print("ok")

sys.stdout.write("Restarted client output: ")
print("%s" % (client.before))
