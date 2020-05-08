#NAME: Eric Chen
#EMAIL: erchen3pro@gmail.com
#ID:305099495


default:
	gcc -std=c11 -lm -lmraa -Wall -Wextra -g -o lab4b lab4b.c

check: default
	chmod u+x test.sh
	./test.sh 305099495
clean:	
	rm -f lab4b *.tar.gz
dist:
	tar -czvf lab4b-305099495.tar.gz Makefile lab4b.c README test.sh
