# Copyright (c) ZeroC, Inc.

$(test)_programs        = reader writer
$(test)_dependencies    = DataStorm Ice TestCommon

$(test)_reader_sources       = Reader.cpp Test.ice
$(test)_writer_sources      = Writer.cpp Test.ice

tests += $(test)
