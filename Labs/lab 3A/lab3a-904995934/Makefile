#NAME: Eric Chen, Julia Wang
#EMAIL: erchen3pro@gmail.com, julia.wang.ca@gmail.com
#ID: 

CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra

default:
	@$(CC) $(CFLAGS) -o lab3a lab3a.c 

clean:
	@rm -f *.tar.gz lab3a
dist:		
	@tar -czvf lab3a-904995934.tar.gz Makefile README lab3a.c ext2_fs.h
