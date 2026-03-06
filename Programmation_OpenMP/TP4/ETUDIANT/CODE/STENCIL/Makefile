CC=gcc
CFLAGS=-O3 
CFLAGS_OMP=-O3 -fopenmp

all:stencil_seq stencil_task stencil_taskdep

stencil_seq: stencil_seq.c
	$(CC) $(CFLAGS) -o $@ $<

stencil_task: stencil_task.c
	$(CC) $(CFLAGS_OMP) -o $@ $<

stencil_taskdep: stencil_taskdep.c
	$(CC) $(CFLAGS_OMP) -o $@ $<

clean:
	rm -f stencil_seq stencil_task stencil_taskdep

