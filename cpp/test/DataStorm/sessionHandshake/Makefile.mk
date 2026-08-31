# Copyright (c) ZeroC, Inc.

$(project)_programs            = client writer
$(project)_dependencies        = TestCommon Ice

# The client speaks the session protocol directly, from its own restatement of it in Test.ice. It must not link the
# DataStorm library: the library carries its own copy of the generated contract, and the two copies would compete
# to supply the exception factories.
$(project)_client_sources      = Client.cpp Test.ice

$(project)_writer_sources      = Writer.cpp
$(project)_writer_dependencies = DataStorm Ice TestCommon

tests += $(project)
