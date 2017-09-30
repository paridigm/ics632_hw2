#!/bin/bash
#SBATCH -n 1  # Number of tasks
#SBATCH -c 20 # Number of cores requested
#SBATCH -N 1  # Number of nodes requested
#SBATCH -t 300  # Runtime in minutes
#SBATCH -o on_a_single_node.out # Standard out goes to this file
#SBATCH -e on_a_single_node.err # Standard err goes to this file
#SBATCH --mail-type ALL
#SBATCH --mail-user tknappra@hawaii.edu
#SBATCH -p ics632.q
#SBATCH --account=ics_632

# setup vars

mult_n=2400			# array size for matrix mult (5 sec)
threads=20			# max number of threads to test (should be 20)
epochs=10			# tests per thread amount
heat_iters=15		# iterations to run the heat transfer problem (10 sec)
heat_tile_size=400	# tile size for heat transfer problem

mult_modes=(PARALLEL_NONE PARALLEL_I PARALLEL_K PARALLEL_J)

results_dir=results

mult_time_results_files=(      mult_seq_time.txt mult_i_time.txt mult_k_time.txt mult_j_time.txt)
mult_l1_misses_results_files=( mult_seq_l1.txt   mult_i_l1.txt   mult_k_l1.txt   mult_j_l1.txt)
mult_llc_misses_results_files=(mult_seq_llc.txt  mult_i_llc.txt  mult_k_llc.txt  mult_j_llc.txt)

heat_time_results_files=(      heat_seq_time.txt heat_parallel_time.txt heat_tile_time.txt)
heat_l1_misses_results_files=( heat_seq_l1.txt   heat_parallel_l1.txt   heat_tile_l1.txt  )
heat_llc_misses_results_files=(heat_seq_llc.txt  heat_parallel_llc.txt  heat_tile_llc.txt )


# make directory
mkdir -p $results_dir


# clear potential previous results
for((m=0; m<4; m++))
do
> $results_dir/${mult_time_results_files[m]}
> $results_dir/${mult_l1_misses_results_files[m]}
> $results_dir/${mult_llc_misses_results_files[m]}
done

for((m=0; m<3; m++))
do
> $results_dir/${heat_time_results_files[m]}
> $results_dir/${heat_l1_misses_results_files[m]}	
> $results_dir/${heat_llc_misses_results_files[m]}
done



###### matrix mult - sequential test ########

echo "matrix mult - ${mult_modes[0]} "

icc exercise1.c -o exercise1 -Ofast -fopenmp -D N=$mult_n -D NUM_THREADS=1

for ((j=0; j<epochs; j++))
do 
	# append stats to output file(s)
	perf stat -d -o perf.txt ./exercise1;
	str_t=$(cat perf.txt | grep "seconds time elapsed" | sed  "s/ *//" | sed "s/ .*seconds.*//" | sed "s/,//g")
	str_l1=$(cat perf.txt | grep "L1-dcache-load-misses" | sed "s/ *//" | sed "s/ .*L1.*//" | sed "s/,//g")
	str_llc=$(cat perf.txt | grep "LLC-load-misses" | sed "s/ *//" | sed "s/ .*LLC.*//" | sed "s/,//g")
	echo -n "$str_t, " >> $results_dir/${mult_time_results_files[0]}
	echo -n "$str_l1, " >> $results_dir/${mult_l1_misses_results_files[0]}
	echo -n "$str_llc, " >> $results_dir/${mult_llc_misses_results_files[0]}
	echo " $j"
done


###### matrix mult - parallel tests ########

echo "Matrix Multiplication - Parallel Tests"

for((m=1; m<4; m++))
do
	echo "matrix mult - ${mult_modes[m]} "

	for ((i=1; i<=threads; i++))
	do
		# compile for each thread
		icc exercise1.c -o exercise1 -Ofast -fopenmp -D N=$mult_n -D NUM_THREADS=$i -D ${mult_modes[m]}
		
		# add a line
		echo -n "$i, " >> $results_dir/${mult_time_results_files[m]}
		echo -n "$i, " >> $results_dir/${mult_l1_misses_results_files[m]}
		echo -n "$i, " >> $results_dir/${mult_llc_misses_results_files[m]}
		echo $i
		
		for ((j=0; j<epochs; j++))
		do 
			# append stats to output file(s)
			perf stat -d -o perf.txt ./exercise1;
			str_t=$(cat perf.txt | grep "seconds time elapsed" | sed  "s/ *//" | sed "s/ .*seconds.*//" | sed "s/,//g")
			str_l1=$(cat perf.txt | grep "L1-dcache-load-misses" | sed "s/ *//" | sed "s/ .*L1.*//" | sed "s/,//g")
			str_llc=$(cat perf.txt | grep "LLC-load-misses" | sed "s/ *//" | sed "s/ .*LLC.*//" | sed "s/,//g")
			echo -n "$str_t, " >> $results_dir/${mult_time_results_files[m]}
			echo -n "$str_l1, " >> $results_dir/${mult_l1_misses_results_files[m]}
			echo -n "$str_llc, " >> $results_dir/${mult_llc_misses_results_files[m]}
			echo " $j"
		done
		
		echo "" >> $results_dir/${mult_time_results_files[m]}
		echo "" >> $results_dir/${mult_l1_misses_results_files[m]}
		echo "" >> $results_dir/${mult_llc_misses_results_files[m]}
		
	done

	echo ""

done


###### heat transfer - sequential ########

echo "Heat Transfer - Sequential"

icc exercise2.c -o exercise2 -Ofast -fopenmp

for ((j=0; j<epochs; j++))
do 
	# append stats to output file(s)
	perf stat -d -o perf.txt ./exercise2 1 $heat_iters;
	str_t=$(cat perf.txt | grep "seconds time elapsed" | sed  "s/ *//" | sed "s/ .*seconds.*//" | sed "s/,//g")
	str_l1=$(cat perf.txt | grep "L1-dcache-load-misses" | sed "s/ *//" | sed "s/ .*L1.*//" | sed "s/,//g")
	str_llc=$(cat perf.txt | grep "LLC-load-misses" | sed "s/ *//" | sed "s/ .*LLC.*//" | sed "s/,//g")
	echo -n "$str_t, " >> $results_dir/${heat_time_results_files[0]}
	echo -n "$str_l1, " >> $results_dir/${heat_l1_misses_results_files[0]}
	echo -n "$str_llc, " >> $results_dir/${heat_llc_misses_results_files[0]}
	echo " $j"
done


###### heat transfer - faster versions ########

echo "Heat Transfer - Faster Versions"

# compile for each thread
icc exercise2.c -o exercise2 -Ofast -fopenmp -D PARALLEL
icc exercise2_fast.c -o exercise2_fast -Ofast -fopenmp -D TILE_SIZE=$heat_tile_size

for ((i=1; i<=threads; i++))
do
		
	# add a line
	echo -n "$i, " >> $results_dir/${heat_time_results_files[1]}		# parallel
	echo -n "$i, " >> $results_dir/${heat_l1_misses_results_files[1]}
	echo -n "$i, " >> $results_dir/${heat_llc_misses_results_files[1]}
	
	echo -n "$i, " >> $results_dir/${heat_time_results_files[2]}		# tile
	echo -n "$i, " >> $results_dir/${heat_l1_misses_results_files[2]}
	echo -n "$i, " >> $results_dir/${heat_llc_misses_results_files[2]}
	
	echo $i
	
	for ((j=0; j<epochs; j++))
	do 
		
		# parellel
		perf stat -d -o perf.txt ./exercise2 $i $heat_iters;
		str_t=$(cat perf.txt | grep "seconds time elapsed" | sed  "s/ *//" | sed "s/ .*seconds.*//" | sed "s/,//g")
		str_l1=$(cat perf.txt | grep "L1-dcache-load-misses" | sed "s/ *//" | sed "s/ .*L1.*//" | sed "s/,//g")
		str_llc=$(cat perf.txt | grep "LLC-load-misses" | sed "s/ *//" | sed "s/ .*LLC.*//" | sed "s/,//g")
		echo -n "$str_t, " >> $results_dir/${heat_time_results_files[1]}
		echo -n "$str_l1, " >> $results_dir/${heat_l1_misses_results_files[1]}
		echo -n "$str_llc, " >> $results_dir/${heat_llc_misses_results_files[1]}
		
		# tiled
		perf stat -d -o perf.txt ./exercise2_fast $i $heat_iters;
		str_t=$(cat perf.txt | grep "seconds time elapsed" | sed  "s/ *//" | sed "s/ .*seconds.*//" | sed "s/,//g")
		str_l1=$(cat perf.txt | grep "L1-dcache-load-misses" | sed "s/ *//" | sed "s/ .*L1.*//" | sed "s/,//g")
		str_llc=$(cat perf.txt | grep "LLC-load-misses" | sed "s/ *//" | sed "s/ .*LLC.*//" | sed "s/,//g")
		echo -n "$str_t, " >> $results_dir/${heat_time_results_files[2]}
		echo -n "$str_l1, " >> $results_dir/${heat_l1_misses_results_files[2]}
		echo -n "$str_llc, " >> $results_dir/${heat_llc_misses_results_files[2]}
		
		echo " $j"
	done
	
	echo "" >> $results_dir/${heat_time_results_files[1]}
	echo "" >> $results_dir/${heat_l1_misses_results_files[1]}
	echo "" >> $results_dir/${heat_llc_misses_results_files[1]}
	
	echo "" >> $results_dir/${heat_time_results_files[2]}
	echo "" >> $results_dir/${heat_l1_misses_results_files[2]}
	echo "" >> $results_dir/${heat_llc_misses_results_files[2]}
	
done

echo ""


# log everything
for((m=0; m<4; m++))
do
	cat $results_dir/${mult_time_results_files[m]}
	echo ""
	cat $results_dir/${mult_l1_misses_results_files[m]}
	echo ""
	cat $results_dir/${mult_llc_misses_results_files[m]}
	echo ""
done

for((m=0; m<3; m++))
do
	cat $results_dir/${heat_time_results_files[m]}
	echo ""
	cat $results_dir/${heat_l1_misses_results_files[m]}
	echo ""
	cat $results_dir/${heat_llc_misses_results_files[m]}
	echo ""
done

echo ""
echo ""
