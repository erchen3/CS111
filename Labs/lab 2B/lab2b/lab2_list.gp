#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2b_list.csv

#
# output:
#	lab2b_1.png ... throughput vs. threads single list
#	lab2b_2.png ... Wait For lock/Avg Time per Operation vs. Threads (Mutex)
#	lab2b_3.png ... threads and iterations that run (protected) w/o failure -multiple lists
#	lab2b_4.png ...Throughput vs. threads -mutex with Multiple lists
#	lab2b_5.png ...Throughput vs. threads -spinlock with Multiple Lists

#

# general plot parameters
set terminal png
set datafile separator ","


set title "List-1: Throughput vs. Threads"
set xlabel "Threads"
set xrange[0:25]
set ylabel "Throughput (operation/s)"
set logscale y 10
set output 'lab2b_1.png'

#grep the needed lines
plot \
     "< grep 'list-none-s,[0-9]*,1000,1' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin-Lock' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex' with linespoints lc rgb 'green'


set title "List-2: Wait For lock/Avg Time per Operation vs. Threads (Mutex)"
set xlabel "Threads"
set xrange [0:25]
set ylabel "Time Per Operation (ns)"
set logscale y 10
set output 'lab2b_2.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1' lab2b_list.csv" using ($2):($7) \
	title 'Per Operation' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,[0-9]*,1000,1' lab2b_list.csv" using ($2):($8) \
	title 'Wait For Lock' with linespoints lc rgb 'red', \
     
set title "List-3: Protected Iterations that run without failure -Multi Lists"
unset logscale x
set xrange [0:17]
set xlabel "Threads"
set ylabel "Successful iterations"
set logscale y 10
set output 'lab2b_3.png'
plot \
    "< grep 'list-id-none,[0-9]*,[0-9]*,4' lab2b_list.csv" using ($2):($3) \
	with points lc rgb "red" title "Unprotected", \
    "< grep 'list-id-m,[0-9]*,[0-9]*,4' lab2b_list.csv" using ($2):($3) \
	with points lc rgb "violet" title "Mutex", \
    "< grep 'list-id-s,[0-9]*,[0-9]*,4' lab2b_list.csv" using ($2):($3) \
	with points lc rgb "blue" title "Spin-Lock", \
#
# "no valid points" is possible if even a single iteration can't run
#

# unset the kinky x axis
unset xtics
set xtics

set title "List-4: Throughput vs. Threads -Mutex"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:13]
set ylabel "Throughput(Ops/s)"
set logscale y
set output 'lab2b_4.png'
set key right top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,1' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Lists=1' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-m,[0-9]*,1000,4' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Lists=4' with linespoints lc rgb 'green', \
     "< grep -e 'list-none-m,[0-9]*,1000,8' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title 'Lists=8' with linespoints lc rgb 'violet', \
     "< grep -e 'list-none-m,[0-9]*,1000,16' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title 'Lists=16' with linespoints lc rgb 'red'

set title "List-5: Throughput vs. Threads -Spin-Lock"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:13]
set ylabel "Throughput(Ops/s)"
set logscale y
set output 'lab2b_5.png'
set key right top
plot \
     "< grep -e 'list-none-s,[0-9]*,1000,1' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title 'Lists=1' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,4' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title 'Lists=4' with linespoints lc rgb 'green', \
     "< grep -e 'list-none-s,[0-9]*,1000,8' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title 'Lists=8' with linespoints lc rgb 'violet', \
     "< grep -e 'list-none-s,[0-9]*,1000,16' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title 'Lists=16' with linespoints lc rgb 'red'

