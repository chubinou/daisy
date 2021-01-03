# Project Name
TARGET = arp

# Sources
CPP_SOURCES = \
    main.cpp \
    arp.cpp \
    euclid.cpp \
    xfade.cpp \
    rand.cpp

# Library Locations
LIBDAISY_DIR = ../../libdaisy
DAISYSP_DIR = ../../DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
