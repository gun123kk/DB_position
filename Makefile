CC=gcc
CFLAG=-pthread
INC=-I./ -I./include
CFLAG1=-lbcm2835
CFLAG2=-lmosquitto
app:
	$(CC) $(CFLAG) -o app ./*.c ./src/*.c  $(INC) -lm $(CFLAG1) $(CFLAG2)
 
clean:
	rm app;rm -rf *.o

