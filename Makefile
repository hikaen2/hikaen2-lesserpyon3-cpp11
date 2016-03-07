CC = clang++
CFLAGS = -O3 -std=c++11 -Wall -pedantic-errors
objects = KomaMoves.o kyokumen.o main.o Te.o Joseki.o

all: shogi

shogi: $(objects)
	$(CC) $(CFLAGS) -o shogi $(objects)

KomaMoves.o: kyokumen.h KomaMoves.cpp
	$(CC) $(CFLAGS) -c KomaMoves.cpp

kyokumen.o: kyokumen.h kyokumen.cpp
	$(CC) $(CFLAGS) -c kyokumen.cpp

Te.o: kyokumen.h Te.cpp
	$(CC) $(CFLAGS) -c Te.cpp

main.o: main.h kyokumen.h main.cpp
	$(CC) $(CFLAGS) -c main.cpp

Joseki.o: Joseki.cpp
	$(CC) $(CFLAGS) -c Joseki.cpp

.PHONY: clean
clean:
	rm -f $(objects)

.PHONY: diff
diff:
	git diff --no-prefix 71c2c Joseki.cpp KomaMoves.cpp kyokumen.cpp kyokumen.h main.cpp main.h Te.cpp
