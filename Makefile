INC = -I/usr/local/include -I/usr/share/CUnit/include
LIB = -L/usr/local/lib -L/usr/share/CUnit/lib
SIMD = -msse -mmmx -msse2

all:  runtest asgn2c generateDataset

runtest:util.c runtest.c WjCryptLib/WjCryptLib_Rc4.c
	gcc $^ -o runtest $(INC) $(LIB) -lcunit -std=c99 -O3

generateDataset:util.c generateDataset.c WjCryptLib/WjCryptLib_Rc4.c
	gcc $^ -o generateDataset $(INC) $(LIB) -std=c99 -O3

asgn2c: util.c asgn2c.c WjCryptLib/WjCryptLib_Rc4.c
	mpicc $^ -o asgn2c $(INC) $(LIB) $(SIMD) -lcunit -pthread -fopenmp -std=c99 -O3

clean:
	rm -rf runtest generateDataset asgn2c