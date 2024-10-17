#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

$(project)_programs = dsnode

$(project)_dependencies := DataStorm Ice
$(project)_targetdir    := $(bindir)

projects += $(project)
