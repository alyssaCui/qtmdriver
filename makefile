########################################################################
#                                                                      #
#   Makefile template file                                             #
#   COMPONENT:   Makefile for qtmdriver                           #
#   DATE:        2015/12/16                                            #
#   CREATOR:                                                           #
#                                                                      #
########################################################################
PLATFORM = LINUX
DBGFLAG = -g

CPP  = g++ $(DBGFLAG) -c -Wall -fPIC
LINK = g++ $(DBGFLAG)
LINK_SO = g++ $(DBGFLAG) -shared  -fPIC
	
CXXFLAGS = 
ifeq ($(PLATFORM), LINUX)
	CXXFLAGS += -DLINUX
endif

####################################################################
# definition for directory
PROJECTNAME = qtmdriver
PROJ_DIR =  .

INC_FLAG = -I$(PROJ_DIR)/inc
LDFLAGS = -L. -L$(PROJ_DIR)/lib -lpthread -lqtm
SRCS = $(wildcard $(PROJ_DIR)/src/*.cpp $(PROJ_DIR)/src/*.c) 

####################################################################
all:  clean compile $(PROJECTNAME)   
	rm -rf *.o

compile:
	$(CPP) $(CXXFLAGS) $(SRCS) $(INC_FLAG) 

$(PROJECTNAME): 
	$(LINK) -o $@ *.o $(LDFLAGS)
	
clean:
#	@echo $(PROJ_DIR)
	rm -f $(PROJECTNAME)
	rm -rf *.o
