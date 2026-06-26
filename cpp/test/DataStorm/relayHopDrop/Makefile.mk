# Copyright (c) ZeroC, Inc.

$(project)_programs        = writer
$(project)_dependencies    = DataStorm Ice TestCommon

$(project)_writer_sources  = Writer.cpp

tests += $(project)
