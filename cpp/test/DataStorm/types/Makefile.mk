# Copyright (c) ZeroC, Inc.

$(project)_programs        = reader writer
$(project)_dependencies    = DataStorm Ice TestCommon

$(project)_reader_sources  = Reader.cpp Test.ice
$(project)_writer_sources  = Writer.cpp Test.ice

tests += $(project)
