#NAME: Eric Chen
#EMAIL: erchen3pro@gmail.com
#ID: 305099495


default:
	gcc -std=c11  -Wall -Wextra -lm -lmraa -g -o lab4c_tcp lab4c_tcp.c
	gcc  -std=c11 -Wall -Wextra -lm -lmraa -g -o lab4c_tls lab4c_tls.c  -lssl -lcrypto 

clean:
	rm -f *.tar.gz lab4c_tls lab4c_tcp
dist: default
	tar -cvzf lab4c-305099495.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README
