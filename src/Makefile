
TOPDIR = ../

include $(TOPDIR)/Makefile.commands

SUB_DIRS := common \
			config/log \
			config/xml_manager \
			http \
			main

all:
	$(foreach dirs, $(SUB_DIRS), make -C $(dirs);)
	$(AR) rcs $(LIB_DIR)/libnixia.a $(OBJ_DIR)/*.o

