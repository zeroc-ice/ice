# Copyright (c) ZeroC, Inc.

$(project)_programs        = reader writer
$(project)_dependencies    = DataStorm Ice TestCommon

$(project)_reader_sources      = Reader.cpp
$(project)_writer_sources      = Writer.cpp

tests += $(project)
