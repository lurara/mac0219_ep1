OUTPUT=mandelbrot

IMAGE=.ppm

CC=gcc
CC_OPT=-std=c11

CC_OMP=-fopenmp
CC_PTH=-pthread

MPI_CC=mpicc

.PHONY: seq
seq: $(OUTPUT)_seq

$(OUTPUT)_seq: $(OUTPUT)_seq.c
	$(CC) -o $(OUTPUT)_seq $(CC_OPT) $(OUTPUT)_seq.c

.PHONY: pth
pth: $(OUTPUT)_pth
	
$(OUTPUT)_pth: $(OUTPUT)_pth.c
	$(CC) -o -pthread $(OUTPUT)_pth $(CC_OPT) $(CC_PTH) $(OUTPUT)_pth.c

.PHONY: omp
omp: $(OUTPUT)_omp

$(OUTPUT)_omp: $(OUTPUT)_omp.c
	$(CC) -o $(OUTPUT)_omp $(CC_OPT) $(CC_OMP) $(OUTPUT)_omp.c

# MPI 
.PHONY: mpi
mpi: $(OUTPUT)_mpi
	
$(OUTPUT)_mpi: $(OUTPUT)_mpi.c
	$(MPI_CC) -o $(OUTPUT)_mpi $(OUTPUT)_mpi.c

# MPI + OMP
.PHONY: mpi_omp
mpi_omp: $(OUTPUT)_mpi_omp
	
$(OUTPUT)_mpi_omp: $(OUTPUT)_mpi_omp.c
	$(MPI_CC) -o $(OUTPUT)_mpi_omp $(OUTPUT)_mpi_omp.c

# MPI + PTH
.PHONY: mpi_pth
mpi_pth: $(OUTPUT)_mpi_pth
	
$(OUTPUT)_mpi_pth: $(OUTPUT)_mpi_pth.c
	$(MPI_CC) -o $(OUTPUT)_mpi_pth $(OUTPUT)_mpi_pth.c

.PHONY: all
all: $(OUTPUT)_omp $(OUTPUT)_pth $(OUTPUT)_seq 

.PHONY: clean
clean:
	rm $(OUTPUT)_omp $(OUTPUT)_pth $(OUTPUT)_mpi $(OUTPUT)_mpi_omp $(OUTPUT)_mpi_pth $(OUTPUT)_seq *$(IMAGE)
