# OSAL files.
OSALSRC += ${CHIBIOS}/os/hal/osal/nil/osal.c

# Required include directories
OSALINC += ${CHIBIOS}/os/hal/osal/nil

# Shared variables
ALLCSRC += $(OSALSRC)
ALLINC  += $(OSALINC)
