CC = gcc

DEPS = fs.h
OBJ = makeFileSystem.o fs.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

makeFileSystem: $(OBJ)
	gcc $(CFLAGS) -o $@ $^ -lm
.PHONY : clean
clean:
	-rm *.o $(objects)
