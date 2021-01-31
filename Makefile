# Project Name
TARGET = arp

LIBDAISY_DIR = ../../libdaisy
DAISYSP_DIR = ../../DaisySP
FATFS_DIR = $(LIBDAISY_DIR)/Middlewares/Third_Party/FatFs/src
C_SOURCES = \
	$(FATFS_DIR)/diskio.c \
	$(FATFS_DIR)/ff.c \
	$(FATFS_DIR)/ff_gen_drv.c \
	$(FATFS_DIR)/option/ccsbcs.c \

# Sources
CPP_SOURCES = \
    main.cpp \
    arp.cpp \
    euclid.cpp \
    xfade.cpp \
    rand.cpp \
    tableenv.cpp \
    subharmonic.cpp \
    Menu.cpp \
    plugin.cpp


C_INCLUDES = -I$(FATFS_DIR) -I$(LIBDAISY_DIR)/src/sys

# Library Locations
LIBDAISY_DIR = ../../libdaisy
DAISYSP_DIR = ../../DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
