EXECUTABLES = mpi_solved1 mpi_solved2 mpi_solved3 mpi_solved4 mpi_solved5 # 
COMPILER = mpicc #mpicc-openmpi-mp
FLAGS = -O3 -Wall

# make all
all: $(EXECUTABLES)
	

# problem 1
mpi_solved1: mpi_solved1.c
	$(COMPILER) $(FLAGS) mpi_solved1.c -o mpi_solved1

1: mpi_solved1
	clear
	mpirun -np 4 ./mpi_solved1  


# problem 2
mpi_solved2: mpi_solved2.c
	$(COMPILER) $(FLAGS) mpi_solved2.c -o mpi_solved2

2: mpi_solved2
	clear
	mpirun -np 4 ./mpi_solved2  



# problem 3
mpi_solved3: mpi_solved3.c
	$(COMPILER) $(FLAGS) mpi_solved3.c -o mpi_solved3

3: mpi_solved3
	clear
	mpirun -np 4 ./mpi_solved3  



# problem 4
mpi_solved4: mpi_solved4.c
	$(COMPILER) $(FLAGS) mpi_solved4.c -o mpi_solved4

4: mpi_solved4
	clear
	mpirun -np 4 ./mpi_solved4  


# problem 5
mpi_solved5: mpi_solved5.c
	$(COMPILER) $(FLAGS) mpi_solved5.c -o mpi_solved5

5: mpi_solved5
	clear
	mpirun -np 4 -hosts box567,box571 ./mpi_solved5  

55: mpi_solved5
	mpirun -np 4 -hosts crunchy1,crunchy3 ./mpi_solved5  

# problem 6
mpi_solved6: mpi_solved6.c
	$(COMPILER) $(FLAGS) mpi_solved6.c -o mpi_solved6

6: mpi_solved6
	clear
	mpirun -np 4 ./mpi_solved6  




# clean up
clean:
	rm -rf $(EXECUTABLES)
	rm -vf *~