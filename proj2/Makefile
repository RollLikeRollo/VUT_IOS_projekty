PROJ=proj2
CFLAGS  = -std=gnu99 -Wall -Wextra -Werror -pedantic 
RM=rm -f

$(PROJ): $(PROJ).c
	gcc $(CFLAGS) -o $(PROJ) $(PROJ).c -pthread -lrt

clean:
	$(RM) *.o $(PROJ)
