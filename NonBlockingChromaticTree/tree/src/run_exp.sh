# ./test <n> <num_proc> <sleep>
# rebal on
echo "10000 1 50000 rebal"
time ./test 10000 1 50000 1 > log/rebal/res_10k_1_50k.txt
echo "10000 2 50000 rebal"
time ./test 10000 2 50000 1 > log/rebal/res_10k_2_50k.txt
echo "10000 4 50000 rebal"
time ./test 10000 4 50000 1 > log/rebal/res_10k_4_50k.txt
echo "10000 6 50000 rebal"
time ./test 10000 6 50000 1 > log/rebal/res_10k_6_50k.txt
echo "10000 8 50000 rebal"
time ./test 10000 8 50000 1 > log/rebal/res_10k_8_50k.txt

echo "20000 1 50000 rebal"
time ./test 20000 1 50000 1 > log/rebal/res_20k_1_50k.txt
echo "20000 2 50000 rebal"
time ./test 20000 2 50000 1 > log/rebal/res_20k_2_50k.txt
echo "20000 4 50000 rebal"
time ./test 20000 4 50000 1 > log/rebal/res_20k_4_50k.txt
echo "20000 6 50000 rebal"
time ./test 20000 6 50000 1 > log/rebal/res_20k_6_50k.txt
echo "20000 8 50000 rebal"
time ./test 20000 8 50000 1 > log/rebal/res_20k_8_50k.txt

echo "40000 1 50000 rebal"
time ./test 40000 1 50000 1 > log/rebal/res_40k_1_50k.txt
echo "40000 2 50000 rebal"
time ./test 40000 2 50000 1 > log/rebal/res_40k_2_50k.txt
echo "40000 4 50000 rebal"
time ./test 40000 4 50000 1 > log/rebal/res_40k_4_50k.txt
echo "40000 6 50000 rebal"
time ./test 40000 6 50000 1 > log/rebal/res_40k_6_50k.txt
echo "40000 8 50000 rebal"
time ./test 40000 8 50000 1 > log/rebal/res_40k_8_50k.txt

echo "60000 1 50000 rebal"
time ./test 60000 1 50000 1 > log/rebal/res_60k_1_50k.txt
echo "60000 2 50000 rebal"
time ./test 60000 2 50000 1 > log/rebal/res_60k_2_50k.txt
echo "60000 4 50000 rebal"
time ./test 60000 4 50000 1 > log/rebal/res_60k_4_50k.txt
echo "60000 6 50000 rebal"
time ./test 60000 6 50000 1 > log/rebal/res_60k_6_50k.txt
echo "60000 8 50000 rebal"
time ./test 60000 8 50000 1 > log/rebal/res_60k_8_50k.txt

echo "80000 1 50000 rebal"
time ./test 80000 1 50000 1 > log/rebal/res_80k_1_50k.txt
echo "80000 2 50000 rebal"
time ./test 80000 2 50000 1 > log/rebal/res_80k_2_50k.txt
echo "80000 4 50000 rebal"
time ./test 80000 4 50000 1 > log/rebal/res_80k_4_50k.txt
echo "80000 6 50000 rebal"
time ./test 80000 6 50000 1 > log/rebal/res_80k_6_50k.txt
echo "80000 8 50000 rebal"
time ./test 80000 8 50000 1 > log/rebal/res_80k_8_50k.txt

echo "100000 1 50000 rebal"
time ./test 100000 1 50000 1 > log/rebal/res_100k_1_50k.txt
echo "100000 2 50000 rebal"
time ./test 100000 2 50000 1 > log/rebal/res_100k_2_50k.txt
echo "100000 4 50000 rebal"
time ./test 100000 4 50000 1 > log/rebal/res_100k_4_50k.txt
echo "100000 6 50000 rebal"
time ./test 100000 6 50000 1 > log/rebal/res_100k_6_50k.txt
echo "100000 8 50000 rebal"
time ./test 100000 8 50000 1 > log/rebal/res_100k_8_50k.txt

echo "---------------------------------------------"

echo "10000 1 0 norebal"
time ./test 10000 1 0 0 > log/norebal/res_10k_1_50k.txt
echo "10000 2 0 norebal"
time ./test 10000 2 0 0 > log/norebal/res_10k_2_50k.txt
echo "10000 4 0 norebal"
time ./test 10000 4 0 0 > log/norebal/res_10k_4_50k.txt
echo "10000 6 0 norebal"
time ./test 10000 6 0 0 > log/norebal/res_10k_6_50k.txt
echo "10000 8 0 norebal"
time ./test 10000 8 0 0 > log/norebal/res_10k_8_50k.txt

echo "20000 8 0 norebal"
time ./test 20000 8 0 0 > log/norebal/res_20k_8_50k.txt
echo "40000 8 0 norebal"
time ./test 40000 8 0 0 > log/norebal/res_40k_8_50k.txt
echo "60000 8 0 norebal"
time ./test 60000 8 0 0 > log/norebal/res_60k_8_50k.txt
echo "80000 8 0 norebal"
time ./test 80000 8 0 0 > log/norebal/res_80k_8_50k.txt
echo "100000 8 0 norebal"
time ./test 100000 8 0 0 > log/norebal/res_100k_8_50k.txt

echo "10000 1 50000 norebal"
time ./test 10000 1 0 0 > log/norebal/res_10k_1_0.txt
echo "10000 2 50000 norebal"
time ./test 10000 2 0 0 > log/norebal/res_10k_2_0.txt
echo "10000 4 50000 norebal"
time ./test 10000 4 0 0 > log/norebal/res_10k_4_0.txt
echo "10000 6 50000 norebal"
time ./test 10000 6 0 0 > log/norebal/res_10k_6_0.txt
echo "10000 8 50000 norebal"
time ./test 10000 8 0 0 > log/norebal/res_10k_8_0.txt

echo "20000 1 50000 norebal"
time ./test 20000 1 0 0 > log/norebal/res_20k_1_0.txt
echo "20000 2 50000 norebal"
time ./test 20000 2 0 0 > log/norebal/res_20k_2_0.txt
echo "20000 4 50000 norebal"
time ./test 20000 4 0 0 > log/norebal/res_20k_4_0.txt
echo "20000 6 50000 norebal"
time ./test 20000 6 0 0 > log/norebal/res_20k_6_0.txt
echo "20000 8 50000 norebal"
time ./test 20000 8 0 0 > log/norebal/res_20k_8_0.txt

echo "40000 1 50000 norebal"
time ./test 40000 1 0 0 > log/norebal/res_40k_1_0.txt
echo "40000 2 50000 norebal"
time ./test 40000 2 0 0 > log/norebal/res_40k_2_0.txt
echo "40000 4 50000 norebal"
time ./test 40000 4 0 0 > log/norebal/res_40k_4_0.txt
echo "40000 6 50000 norebal"
time ./test 40000 6 0 0 > log/norebal/res_40k_6_0.txt
echo "40000 8 50000 norebal"
time ./test 40000 8 0 0 > log/norebal/res_40k_8_0.txt

echo "60000 1 50000 norebal"
time ./test 60000 1 0 0 > log/norebal/res_60k_1_0.txt
echo "60000 2 50000 norebal"
time ./test 60000 2 0 0 > log/norebal/res_60k_2_0.txt
echo "60000 4 50000 norebal"
time ./test 60000 4 0 0 > log/norebal/res_60k_4_0.txt
echo "60000 6 50000 norebal"
time ./test 60000 6 0 0 > log/norebal/res_60k_6_0.txt
echo "60000 8 50000 norebal"
time ./test 60000 8 0 0 > log/norebal/res_60k_8_0.txt

echo "80000 1 50000 norebal"
time ./test 80000 1 0 0 > log/norebal/res_80k_1_0.txt
echo "80000 2 50000 norebal"
time ./test 80000 2 0 0 > log/norebal/res_80k_2_0.txt
echo "80000 4 50000 norebal"
time ./test 80000 4 0 0 > log/norebal/res_80k_4_0.txt
echo "80000 6 50000 norebal"
time ./test 80000 6 0 0 > log/norebal/res_80k_6_0.txt
echo "80000 8 50000 norebal"
time ./test 80000 8 0 0 > log/norebal/res_80k_8_0.txt

echo "100000 1 50000 norebal"
time ./test 100000 1 0 0 > log/norebal/res_100k_1_0.txt
echo "100000 2 50000 norebal"
time ./test 100000 2 0 0 > log/norebal/res_100k_2_0.txt
echo "100000 4 50000 norebal"
time ./test 100000 4 0 0 > log/norebal/res_100k_4_0.txt
echo "100000 6 50000 norebal"
time ./test 100000 6 0 0 > log/norebal/res_100k_6_0.txt
echo "100000 8 50000 norebal"
time ./test 100000 8 0 0 > log/norebal/res_100k_8_0.txt