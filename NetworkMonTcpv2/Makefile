# Makefile for test
# AIX version

.SUFFIXES:
.SUFFIXES: .cpp .obj .o

CPP = g++



	
INC= -I../Utility -I../CommFramwork 


CPPFLAGS = -g -Wall  -O2 
CPPDEFS =  -DLINUX -DUNIX  -DNDEBUG -D_THREAD_SAFE -D_MT -DSA_USE_PTHREAD -D_AIX32_THREADS 

DLLLIB  = -lcommframwork -lutility -lpthread


.cpp.o:
	$(CPP) $(CPPFLAGS)  -c   $(CPPDEFS)   $(INC)  $< -o $@

OBJL = ProcessInterfaceMons.o ProcessInterfaceMonc.o PktBinBlockMon.o NmService.o NmCpMgr.o NetworkMon.o


#ALL:  nmtcp   clean
ALL:  nmtcp

nmtcp: $(OBJL)
	$(CPP) $(CPPFLAGS) $(CPPDEFS) -o $@ $(OBJL) $(INC) -L$(HOME)/lib $(DLLLIB) 
	mv $@ $(HOME)/bin
testm: $(OBJLT)
	$(CPP) $(CPPFLAGS) $(CPPDEFS) -o $@ $(OBJLT) $(INC) -L$(HOME)/lib   $(DLLLIB) 
	mv $@ $(HOME)/bin		


clean:
	rm -f *.o
