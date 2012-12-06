#################################################
#
# Makefile progetto lso 2011
# (fram 1)(fram 2)(fram 3)
#################################################


# ***** DA COMPLETARE ******  con i file da consegnare *.c e *.h                                                                        
# primo frammento                                                                                                           
FILE_DA_CONSEGNARE1=./dgraph.c ./dgraph.h ./shortestpath.c ./shortestpath.h
# secondo frammento                                                                                                         
FILE_DA_CONSEGNARE2=./carstat

# terzo frammento                                                                                                          
FILE_DA_CONSEGNARE3=./common.h ./comsock.h ./comsock.c ./hashutenti.h ./hashutenti.c ./docars.c ./mgcars.c ./test-blackbox.c

# Compilatore
CC= gcc

CFLAGS = -Wall -pedantic-errors -g -std=gnu99 

# Librerie 
# Directory in cui si trovano le librerie
LIBDIR = ../lib
# Opzione di linking
LIBS = -L $(LIBDIR)

##################################################################
# Decommentare per eseguire il coverage-test con "make coverage" 
#LIBS += -lgcov
#CFLAGS += -fprofile-arcs -ftest-coverage
# Decommentare per abilitare l'output verboso 	                 
#CFLAGS += -DVERBOSE
# Decommentare per abilitare la hash alternativa 	         
#CFLAGS += -DALT_HASH
##################################################################

# Nome libreria primo frammento
LIBNAME1 = libcars.a
# Oggetti libreria $(LIBNAME1)
# DA COMPLETARE se si usano piu' file in aggiunta da dgraph.o
objects1 = dgraph.o shortestpath.o 

# Nome libreria terzo frammento
LIBNAME3 = libsock.a
# Oggetti libreria $(LIBNAME3)
# DA COMPLETARE se si usano piu' file in aggiunta da comsock.o
objects3 = comsock.o 

# Nome eseguibili primo frammento
exe1 = dgraph
exe2 = shpath

# Nome eseguibili terzo frammento
exeserv = mgcars
execli = docars

#consegna1 docu


.PHONY: clean lib1 test11 test12 
.PHONY: test21 
.PHONY: lib3 test31 test32 test33 
.PHONY: coverage 

# creazione libreria primo frammento
lib1:  $(objects1)
	-rm  -f $(LIBDIR)/$(LIBNAME1)
	ar -r $(LIBNAME1) $(objects1)
	cp $(LIBNAME1) $(LIBDIR)

# creazione libreria terzo frammento
lib3:  $(objects3)
	-rm  -f $(LIBDIR)/$(LIBNAME3)
	ar -r $(LIBNAME3) $(objects3)
	cp $(LIBNAME3) $(LIBDIR)

# eseguibili test primo frammento
$(exe1) : test-dgraph.o
	$(CC) -o $@ $^ $(LIBS) -lcars 

$(exe2) : test-shpath.o
	$(CC) -o $@ $^ $(LIBS) -lcars 

test-dgraph.o: test-dgraph.c dgraph.h 
	$(CC) $(CFLAGS) -c $< 

test-shpath.o: test-shpath.c dgraph.h shortestpath.h
	$(CC) $(CFLAGS) -c $<

# make rule per gli altri .o del primo frammento (***DA COMPLETARE***)

dgraph.o: dgraph.c common.h dgraph.h
	$(CC) $(CFLAGS) -c $<
	
shortestpath.o: shortestpath.c common.h dgraph.h shortestpath.h
	$(CC) $(CFLAGS) -c $<



######### target frammento 3 #%%%%%%%%%%%%
# make rule per gli eseguibili ed i .o del terzo frammento (***DA COMPLETARE***)
comsock.o: comsock.c common.h comsock.h
	$(CC) $(CFLAGS) -c $<

hashutenti.o: hashutenti.c common.h hashutenti.h
	$(CC) $(CFLAGS) -c $<

# eseguibile server
$(exeserv) : mgcars.o hashutenti.o
	   $(CC) $(CFLAGS) -o $@ $^ $(LIBS) -lcars -lsock -lpthread

mgcars.o : mgcars.c common.h hashutenti.h comsock.h
	   $(CC) $(CFLAGS) -c $<
	   
#eseguibile client
$(execli) : docars.o 
	  $(CC) $(CFLAGS) -o $@ $^ $(LIBS) -lcars -lsock -lpthread

docars.o: docars.c common.h dgraph.h comsock.h 
	  $(CC) $(CFLAGS) -c $<

all:
	@echo "\a\n\t *** MALLOC_TRACE è \"$(MALLOC_TRACE)\" deve essere \"./.mtrace.log\""
	@sleep 5
	make clean
	make test11
	make test12
	make test21
	make test31
	make test32
	make test33


########### NON MODIFICARE DA QUA IN POI ################
# genera la documentazione con doxygen
docu: ../doc/Doxyfile
	make -C ../doc/

#ripulisce  l'ambiente
clean:
	-rm -f *.o *~ ./core

# terget di test del primo frammento
F=./TUSCANY.map ./CITY.txt
test11: 
	make clean
	make lib1
	make $(exe1)
	cp ./DATA/CITY.txt ./DATA/TUSCANY.map .
	chmod 644 ./CITY.txt ./TUSCANY.map
	#@echo MALLOC_TRACE=$(MALLOC_TRACE)
	#@echo MALLOC_TRACE deve essere settata a \"./.mtrace.log\"
	-rm -f ./.mtrace.log
	./$(exe1) 
	mtrace ./$(exe1) ./.mtrace.log
	sort ./outcitta.txt > ./outcitta.sort
	sort ./outstrade.map > ./outstrade.sort
	sort ./CITY.txt > ./CITY.sort
	sort ./TUSCANY.map > ./TUSCANY.sort
	diff ./CITY.sort ./outcitta.sort
	diff ./TUSCANY.sort ./outstrade.sort
	@echo  "\a\n\t **** Test 11 superato! ****\n"

test12: 
	make clean
	make lib1
	make $(exe2)
	cp ./DATA/out12.check .
	chmod 644 ./out12.check
	rm -f ./out12
	#@echo MALLOC_TRACE=$(MALLOC_TRACE)
	#@echo MALLOC_TRACE deve essere settata a \"./.mtrace.log\"
	-rm -f ./.mtrace.log
	./$(exe2) > ./out12
	diff ./out12 ./out12.check
	mtrace ./$(exe2) ./.mtrace.log
	@echo  "\a\n\t **** Test 12 superato! ****\n"

# test secondo frammento
test21:
	cp DATA/carstat.input? .            
	cp DATA/out.carstat.check .            
	chmod 644 ./carstat.input?
	chmod 644 ./out.carstat.check
	./testcarstat 
	diff ./out.carstat ./out.carstat.check  
	@echo "\a\n\t\t *** Test 2-1 superato! ***\n"


# primo test terzo frammento
test31: 
	-killall -w mgcars
	-killall -w docars
	make clean
	make lib1
	make lib3
	make $(exeserv)
	make $(execli)
	cp ./DATA/TUSCANY.map .
	cp ./DATA/CITY.txt .
	cp ./DATA/CITYBAD.txt .
	cp ./DATA/TUSCANY2.map .
	chmod 644 ./TUSCANY.map ./CITY.txt ./TUSCANY2.map ./CITYBAD.txt
	-rm -fr tmp/
	-mkdir tmp/
	./testparse
	@echo  "\a\n\t **** Test 31 superato! ****\n"

test32:
	-killall -w mgcars
	-killall -w docars
	make clean
	make lib1
	make lib3
	make $(exeserv)
	make $(execli)
	cp ./DATA/TUSCANY.map .
	cp ./DATA/CITY.txt .
	cp ./DATA/mgcars.log.1.check .
	cp ./DATA/mgcars.log.2.check .
	chmod 644 ./TUSCANY.map ./CITY.txt 
	chmod 644 mgcars.log.1.check mgcars.log.2.check
	-rm -fr tmp/
	-mkdir tmp/
	./testfunz
	@echo  "\a\n\t **** Test 32 superato! ****\n"

test33:
	-killall -w mgcars
	-killall -w docars
	make clean
	make lib1
	make lib3
	make $(exeserv)
	make $(execli)
	cp ./DATA/TUSCANY.map .
	cp ./DATA/CITY.txt .
	cp ./DATA/mgcars.log.2.check .
	chmod 644 ./TUSCANY.map ./CITY.txt 
	chmod 644 mgcars.log.2.check
	-rm -fr tmp/
	-mkdir tmp/
	./testpar
	@echo  "\a\n\t **** Test 33 superato! ****\n"

# target di consegna del primo frammento
SUBJECT1="lso11: consegna primo frammento"
ADDRESS="susanna@di.unipi.it"
#effettua il test e prepara il tar per la consegna
consegna1:
	make clean
	make test11
	make test12
	./gruppo-check.pl < gruppo.txt
	tar -cvf $(USER)-f1.tar ./gruppo.txt $(FILE_DA_CONSEGNARE1) ./Makefile
	@echo "*** PRIMO FRAMMENTO: TAR PRONTO $(USER)-f1.tar ***"
	@echo "*** inviare per email a $(ADDRESS) con subject\n \"$(SUBJECT1)\" ***"


# target di consegna del secondo frammento
SUBJECT2="lso11: consegna secondo frammento"

#effettua il test e prepara il tar per la consegna
consegna2:
	make clean
	make test21
	./gruppo-check.pl < gruppo.txt
	tar -cvf $(USER)-f2.tar ./gruppo.txt $(FILE_DA_CONSEGNARE2) 
	@echo "*** SECONDO FRAMMENTO: TAR PRONTO $(USER)-f2.tar ***"
	@echo "*** inviare per email a $(ADDRESS) con subject\n \"$(SUBJECT2)\" ***"


# target di consegna del terzo frammento
SUBJECT3="lso11: consegna terzo frammento"

# effettua il test e prepara il tar per la consegna
# ricordarsi di riconsegnare anche i file dei frammenti 1 e 2
consegna3:
	make clean
	make test11
	make test12
	make test21
	make test31
	make test32
	make test33
	./gruppo-check.pl < gruppo.txt
	tar -cvf $(USER)-f3.tar ./gruppo.txt $(FILE_DA_CONSEGNARE1) $(FILE_DA_CONSEGNARE2) $(FILE_DA_CONSEGNARE3) ./Makefile
	@echo "*** TERZO FRAMMENTO: TAR PRONTO $(USER)-f3.tar ***"
	@echo "*** inviare per email a $(ADDRESS) con subject\n \"$(SUBJECT3)\" ***"


#test-blackbox
test-blackbox: test-blackbox.o hashutenti.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) -lcars
	
test-blackbox.o: test-blackbox.c
	$(CC) $(CFLAGS) -c $<



coverage:
	@echo "\a\n\t *** CFLAGS=\"$(CFLAGS)\" ***\n"
	@echo "\a\n\t *** LIBS=\"$(LIBS)\"  ***\n"
	@echo "DEVONO contenere -fprofile-arcs -ftest-coverage e -lgcov\n"
	@echo "ALTRIMENTI decommenta le righe del Makefile!\n"
	@echo "\a\n\t *** MALLOC_TRACE è \"$(MALLOC_TRACE)\" deve essere \"./.mtrace.log\""
	@sleep 12
	make all
	make test-blackbox
	./test-blackbox
