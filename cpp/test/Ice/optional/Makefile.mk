# For the include <CustomMap.h> in the generated code.
$(test)_cppflags        := -I$(project)

#
# Disable var tracking assignments for Linux with this test
#
ifneq ($(linux_id),)
    $(test)_cppflags += $(if $(filter yes,$(OPTIMIZE)),-fno-var-tracking-assignments)
endif

tests += $(test)
