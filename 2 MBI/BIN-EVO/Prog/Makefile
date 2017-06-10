CC	= g++
CFLAGS	= -std=c++11
OBJ_FILES	= eca.o

all:	eca

eca:	$(OBJ_FILES)
		$(CC) $(OBJ_FILES) -o $@ $(CFLAGS)

eca.o:	eca.cpp
		$(CC) -o $@ -c $<  $(CFLAGS)

clean:
		rm -f eca.o eca *.tab

run:
		./eca -x pattern -p 20 -m 10 -g 500 -s 13 -r 200
		BiCAS2016/BiCAS.py experiment.tab experiment.cas
