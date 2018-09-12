# Makefile for test
# AIX version

.SUFFIXES:
.SUFFIXES: .cpp .obj .o

CPP = g++



	
INC= -I../Utility 


CPPFLAGS = -g -Wall  -O2 
CPPDEFS =  -DLINUX -DUNIX  -DNDEBUG -D_THREAD_SAFE -D_MT -DSA_USE_PTHREAD -D_AIX32_THREADS 

DLLLIB  = -lutility -lpthread


.cpp.o:
	$(CPP) $(CPPFLAGS)  -c   $(CPPDEFS)   $(INC)  $< -o $@

OBJL = NmCpMgr.o NetworkMon.o


#ALL:  nmudp   clean
ALL:  nmudp

nmudp: $(OBJL)
	$(CPP) $(CPPFLAGS) $(CPPDEFS) -o $@ $(OBJL) $(INC) -L$(HOME)/lib $(DLLLIB) 
	mv $@ $(HOME)/bin
testm: $(OBJLT)
	$(CPP) $(CPPFLAGS) $(CPPDEFS) -o $@ $(OBJLT) $(INC) -L$(HOME)/lib   $(DLLLIB) 
	mv $@ $(HOME)/bin		


clean:
	rm -f *.o
