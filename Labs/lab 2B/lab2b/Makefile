#Name: Eric Chen
#EMAIL: erchen3pro@gmail.com
#ID:


default:
	gcc -g -lprofiler -pthread -Wall -Wextra -o lab2_list SortedList.c lab2_list.c

profile: build
	rm -f profile.out
	LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text lab2_list ./raw.gperf > profile.out
	pprof --list=threadWork lab2_list ./raw.gperf >> profile.out

tests:
	./lab2_list --threads=1 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2	--iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4	--iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2b_list.csv

	-./lab2_list --threads=1 --iterations=1 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=1	--iterations=2 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=1 --iterations=4 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=1 --iterations=8 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=1 --iterations=16 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=4 --iterations=1 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=4 --iterations=2 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=4 --iterations=4 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=4 --iterations=8 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=4 --iterations=16 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=8 --iterations=1 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=8 --iterations=2 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=8 --iterations=4 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=8 --iterations=8 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=8 --iterations=16 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=12 --iterations=2 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=12 --iterations=4 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=12 --iterations=8 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=12 --iterations=16 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=16 --iterations=1 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=16 --iterations=2 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=16 --iterations=4 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=16 --iterations=8 --yield=id --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=16 --iterations=16 --yield=id --lists=4 >>lab2b_list.csv

	-./lab2_list --threads=1 --iterations=10 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=1 --iterations=20 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=1 --iterations=40 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=1 --iterations=80 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=4 --iterations=10 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	 -./lab2_list --threads=4 --iterations=20 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=4 --iterations=40 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=4 --iterations=80 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	  -./lab2_list --threads=8 --iterations=10 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=8 --iterations=20 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=8 --iterations=40 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=8 --iterations=80 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	  -./lab2_list --threads=12 --iterations=10 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=12 --iterations=20 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=12 --iterations=40 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=12 --iterations=80 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	  -./lab2_list --threads=16 --iterations=10 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=16 --iterations=20 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=16 --iterations=40 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=16 --iterations=80 --yield=id --sync=s --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=1 --iterations=10 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=1 --iterations=20 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=1 --iterations=40 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=1 --iterations=80 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=4 --iterations=10 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=4 --iterations=20 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=4 --iterations=40 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=4 --iterations=80 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=8 --iterations=10 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=8 --iterations=20 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=8 --iterations=40 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=8 --iterations=80 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=12 --iterations=10 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=12 --iterations=20 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=12 --iterations=40 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=12 --iterations=80 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=16 --iterations=10 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=16 --iterations=20 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=16 --iterations=40 --yield=id --sync=m --lists=4 >>lab2b_list.csv
	-./lab2_list --threads=16 --iterations=80 --yield=id --sync=m --lists=4 >>lab2b_list.csv
#no yield
	-./lab2_list --threads=1 --iterations=1000 --sync=s --lists=1 >>lab2b_list.csv
	-./lab2_list --threads=2 --iterations=1000 --sync=s --lists=1 >>lab2b_list.csv
	-./lab2_list --threads=4  --iterations=1000 --sync=m --lists=1 >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=1000 --sync=m --lists=1 >> lab2b_list.csv
	-./lab2_list --threads=12  --iterations=1000 --sync=m --lists=1 >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=2  --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12  --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	-./lab2_list --threads=2  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	-./lab2_list --threads=12  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
	-./lab2_list --threads=2  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
	-./lab2_list --threads=12  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=1000 --sync=s --lists=1 >> lab2b_list.csv
	-./lab2_list --threads=2  --iterations=1000 --sync=s --lists=1 >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=1000 --sync=s --lists=1 >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=1000 --sync=s --lists=1 >> lab2b_list.csv
	-./lab2_list --threads=12  --iterations=1000 --sync=s --lists=1 >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=2  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=12  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	-./lab2_list --threads=2  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	-./lab2_list --threads=12  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv
	-./lab2_list --threads=2  --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv
	-./lab2_list --threads=12  --iterations=1000 --sync=s --lists=16 >> lab2b_list.csv

graphs:
	gnuplot lab2_list.gp


#make sure its lab2b_list.csv 
dist:	default tests graphs
	tar -cvzf lab2b-305099495.tar.gz README Makefile lab2_list.c SortedList.h SortedList.c profile.out *.csv *.png *.gp
clean:
	rm -rf  lab2_list *.tar.gz 
