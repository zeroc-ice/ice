#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys

def usage():
    print sys.argv[0] + " top_srcdir file1 file2 ..."

if len(sys.argv) < 2:
    usage()
    sys.exit(1)

assemblies = ['glacier2cs', 'icecs', 'icepatch2cs', 'icestormcs', 'iceboxcs', 'icegridcs', 'icesslcs']

top_srcdir = sys.argv[1]
for s in sys.argv[2:]:
    if s[len(s)-4:] != ".exe":
        continue
    f = file(s + ".config", 'w')
    f.write('<?xml version="1.0"?>\n');
    f.write('  <configuration>\n');
    f.write('    <runtime>\n');
    f.write('      <assemblyBinding xmlns="urn:schemas-microsoft-com:asm.v1">\n');
    for a in assemblies:
        f.write("        <dependentAssembly>\n");
        f.write('          <assemblyIdentity name="' + a + '" culture="neutral" publicKeyToken="1f998c50fec78381"/>\n');
        f.write('          <codeBase version="3.2.0.0" href="' + top_srcdir + '/../icecs/bin/' + a + '.dll"/>\n');
        f.write('        </dependentAssembly>\n');
    f.write('    </assemblyBinding>\n');
    f.write('  </runtime>\n');
    f.write('</configuration>\n');
    f.close();
