#NAME: Eric chen
#EMAIL: erchen3pro@gmail.com
#ID:


#build the program
default: lab0.c
	gcc -Wall -Wextra -o lab0 -g lab0.c
#smoke tests
check: default segFaultTest openFile createFile smoothRun sillyArg

segFaultTest:
	@./lab0 --segfault --catch &> /dev/null; \
	if [[ $$? -ne 4 ]]; then \
	echo "Testing failed, didn't catch your segfault."; \
	else \
	echo "Testing passed.segfault caught success."; \
	fi

#trying to open a file that doesn't exist
openFile:
	@echo "uwu" > uwu.txt; \
	chmod ugo-r uwu.txt; \
	./lab0 --input=uwu.txt &> /dev/null; \
	if [[ $$? -ne 2 ]]; then \
	echo "Testing failed. Incorrect exit status.."; \
	else \
	echo " Testing passed. right exit status when we failed to open file"; \
	fi
	@rm uwu.txt
createFile:
	@echo "hi" > nice.txt; \
	chmod u-w nice.txt; \
	./lab0 --output=nice.txt  &> /dev/null; \
	if [[ $$? -ne 3 ]]; then \
	echo "Testing failed. Not able to create file did not return correct exit status"; \
	else \
	echo "Testing passed. Correct exit status when we fail to create file."; \
	fi
	@rm -f nice.txt
smoothRun:
	@echo "UwU" > test.txt; \
	./lab0 --input=test.txt --output=out.txt; \
	if [[ $$? -ne 0 ]]; then \
	echo "Testing failed. Did not have a smooth run using input&output and couldn't end program with exit 0"; \
	else \
	echo " Testing passed. Correct exit status, everything went swimmingly.. "; \
	fi
	@rm test.txt
sillyArg:
	@./lab0 --silliness &> /dev/null; \
	if [[ $$? -ne 1 ]]; then \
	echo " Testing failed. The incorrect option did not return the correct exit status."; \
	else \
	echo " Testing passed. Correct exit status for the case where we have a bad option. "; \
	fi

#build tarball
dist:
	tar -cvzf lab0-305099495.tar.gz README breakpoint.png backtrace.png Makefile lab0.c
#reset and remove all
clean:
	rm -d *.txt *.tar.gz lab0

#Test cases
#1 sillyArg when you use incorrect option exit status should be 1
#2 segFaultTest make sure when we catch segfault the exit status is 4
#3 openFile  When you input redirect, if you cant open a file exit status 2
#4 createFile when you output redirect, cant create file with exit status 3
#5 smoothRun Succeed when we use input and output exit status 0
