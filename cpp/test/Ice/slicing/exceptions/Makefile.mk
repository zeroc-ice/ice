# Copyright (c) ZeroC, Inc.

$(project)_client_sources          = $(test-client-sources)
$(project)_server_sources          = $(test-server-sources) ServerPrivate.ice
$(project)_serveramd_sources       = $(test-serveramd-sources) ServerPrivateAMD.ice

ifneq ($(linux_id),)
    $(project)_cppflags += $(if $(filter yes,$(OPTIMIZE)),-fno-var-tracking-assignments)
endif

tests += $(project)
