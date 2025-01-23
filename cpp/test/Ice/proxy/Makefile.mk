#
# Disable var tracking assignments for Linux with this test
#
ifneq ($(linux_id),)
    $(project)_cppflags += $(if $(filter yes,$(OPTIMIZE)),-fno-var-tracking-assignments)
endif
tests += $(project)
