CC	= g++
CFLAGS	= -std=c++11
OBJ_FILES	= main.o gif2bmp.o

all:	main

main:	$(OBJ_FILES)
		$(CC) $(OBJ_FILES) -o $@ $(CFLAGS)

main.o:	main.cpp gif2bmp.h
		$(CC) -o $@ -c $<  $(CFLAGS)

gif2bmp.o:	gif2bmp.cpp gif2bmp.h
		$(CC) -o $@ -c $<  $(CFLAGS)

clean:
		rm -f *.o main out/*

tests:
		rm -f out/*
		./main -i in/easy.gif -o out/easy.bmp -l out/easy-log
		./main -i in/adam.gif -o out/adam.bmp -l out/adam-log
		./main -i in/android.gif -o out/android.bmp -l out/android-log
		./main -i in/fast.gif -o out/fast.bmp -l out/fast-log
		./main -i in/ff.gif -o out/ff.bmp -l out/ff-log
		./main -i in/fit.gif -o out/fit.bmp -l out/fit-log
		./main -i in/fit1.gif -o out/fit1.bmp -l out/fit1-log
		./main -i in/google.gif -o out/google.bmp -l out/google-log
		./main -i in/chem.gif -o out/chem.bmp -l out/chem-log
		./main -i in/jobs.gif -o out/jobs.bmp -l out/jobs-log
		./main -i in/lena.gif -o out/lena.bmp -l out/lena-log
		./main -i in/linux.gif -o out/linux.bmp -l out/linux-log
		./main -i in/quarter.gif -o out/quarter.bmp -l out/quarter-log
		./main -i in/quarter-int.gif -o out/quarter-int.bmp -l out/quarter-intlog
		./main -i in/ubuntu.gif -o out/ubuntu.bmp -l out/ubuntu-log
		./main -i in/sample.gif -o out/sample.bmp -l out/sample-log
		./main -i in/earth.gif -o out/earth.bmp -l out/earth-log

