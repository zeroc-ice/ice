# Copyright (c) ZeroC, Inc.

$(test)_programs        = writer
$(test)_dependencies    = DataStorm Ice TestCommon

$(test)_writer_sources       = Writer.cpp DuplicateSymbols.cpp Test.ice

tests += $(test)
