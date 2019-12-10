#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <random>
#include <thread>
#include "tree.h"
#include <chrono> 
#include <map>
#include <utility>

using namespace std::chrono; 
using namespace std;

typedef vector<int> vi;

// const int n = 1000000;
// const int num_processes = 8;

int n;
int num_processes;
int sleep_time;
int rebal;
int num_rebal_threads;
int rebal_sleep;

void check_balance(tr node, vi& heights, vi& leaf_wts, vi &real_hts, int level, int true_level);

void insert_nodes(int tid, vi *inKeys, int low, int high) {
	for (int i = low; i < high; ++i)
		insertKey((*inKeys)[i], 0, tid);
}

void delete_nodes(int tid, vi *delKeys, int low, int high) {
	for (int i = low; i < high; ++i)
		deleteKey((*delKeys)[i], tid);
}
void rebalancingThreadFunction(int tid, std::atomic<bool> * continueRebalancing) {
	while((*continueRebalancing) == true){
		rebalancingThreadOperation(tid);
		std::this_thread::sleep_for(std::chrono::milliseconds(rebal_sleep));
	}
}

int main(int argc, char const *argv[])
{	

	if(argc != 7) {
		printf("Enter num_processes, n, sleep time, rebalance, rebal threads and rebal sleep\n");
		return 1;
	}

	n = atoi(argv[1]);
	num_processes = atoi(argv[2]);
	sleep_time = atoi(argv[3]);
	rebal = atoi(argv[4]);
	num_rebal_threads = atoi(argv[5]);
	rebal_sleep = atoi(argv[6]);

	if(rebal == 0)
		sleep_time = 0;

	std::atomic<bool> continueRebalancing(true);	

	vi keys(n);	

	for (int i = 0; i < n; ++i)
		keys[i] = i;	

	// Initialize tree
	init(num_processes+4);
	
	// Shuffle keys
	// Insert keys in parallel 
	srand(42);
	random_shuffle(keys.begin(), keys.end());

	vector<thread *> runthreads;

	int chunksize = n / num_processes;

	// PARALLEL
	printf("Commence insertion\n");
	auto start_insert = high_resolution_clock::now(); 
	for (int i = 0; i < num_processes; ++i) {
		int low = i * chunksize;
		int high = (i+1) * chunksize;
		if(i == num_processes - 1) {
			high = n;
		} 
		thread *tmp = new thread(insert_nodes, i, &keys, low, high);
		runthreads.push_back(tmp);
	}	

	vector<thread *> rebal_threads;

	thread *rebalancingThreadPointer;
	thread *rebalancingThreadPointer2;
	thread *rebalancingThreadPointer3;
	thread *rebalancingThreadPointer4;

	if(rebal == 1) {
		for (int i = 0; i < num_rebal_threads; ++i)
		{
			thread *tmp = new thread(rebalancingThreadFunction, num_processes+i, &continueRebalancing);
			rebal_threads.push_back(tmp);
		}		
	}	

	// Join on the threads
	for (int i = 0; i < num_processes; ++i)
		runthreads[i]->join();
	auto stop_insert = high_resolution_clock::now(); 
	auto duration_insert = duration_cast<microseconds>(stop_insert - start_insert); 
	printf("End insertion\n");

	cout << "Time taken by insertion: "
         << duration_insert.count() << " microseconds" << endl; 

	// Check - all can be reached?
	// printf("Check round 1\n");
	// for (int i = 0; i < n; ++i) {
	// 	if(getNode(keys[i]) == NULL)
	// 		printf("Key %d at posn %d not found", keys[i], i);
	// }
	// printf("Check round 1 Done\n");

	// Shuffle again
	random_shuffle(keys.begin(), keys.end());
	int num_del = n/5;

	vi remain_keys(n - n/5);

	for (int i = n/5; i < n; ++i)
		remain_keys[i - n/5] = keys[i];

	// Delete keys in parallel
	// PARALLEL
	printf("Commence deletion\n");
	auto start_del = high_resolution_clock::now(); 
	int del_chunk = (n/5) / num_processes;
	vector<thread *> delthreads;
	for (int i = 0; i < num_processes; ++i) {
		int low = i * del_chunk;
		int high = (i+1) * del_chunk;
		if(i == num_processes - 1) {
			high =  n/5;
		} 
		thread* tmp = new thread(delete_nodes, i, &keys, low, high);
		delthreads.push_back(tmp);
	}

	for (int i = 0; i < num_processes; ++i)
		delthreads[i]->join();
	auto stop_del = high_resolution_clock::now(); 
	auto duration_del = duration_cast<microseconds>(stop_del - start_del); 
	printf("Deletion done\n");

	cout << "Time taken by deletion: "
         << duration_del.count() << " microseconds" << endl; 

	// rebalancingThreadOperation(0);

	// printf("Check round 2\n");
	// for (int i = 0; i < n/5; ++i) {
	// 	tr x = getNode(keys[i]);
	// 	if(x != NULL)
	// 		printf("Key %d at posn %d FOUND\n", keys[i], i);
	// }
	// for (int i = 0; i < n - n/5; ++i) {
	// 	tr x = getNode(remain_keys[i]); 
	// 	if((x == NULL))
	// 		printf("Key %d at posn %d not found\n", remain_keys[i], i);
	// }
	// printf("Check round 2 Done\n");	

	// Shuffle remaining keys again
	random_shuffle(remain_keys.begin(), remain_keys.end());

	// New keys
	vi new_keys(n/5);
	for (int i = 0; i < n/5; ++i)
		new_keys[i] = i + n;

	random_shuffle(new_keys.begin(), new_keys.end());
	vector<thread *> delthreads_2;
	auto start_mix = high_resolution_clock::now(); 
	int new_chunk ,del_num_processes;
	if(num_processes == 1) {
		new_chunk = n/5;
		del_num_processes = 1;
	}
	else {
		new_chunk = (n/5) / (num_processes / 2);
		del_num_processes = num_processes / 2;
	}
	for (int i = 0; i < del_num_processes; ++i) {
		int low = i * new_chunk;
		int high = (i+1) * new_chunk;
		if(high == del_num_processes - 1)
			high = n/5;
		thread *tmp = new thread(delete_nodes, i, &remain_keys, low, high);
		delthreads_2.push_back(tmp);
	}

	if(num_processes == 1)
		delthreads_2[0]->join();

	vector<thread *> inthreads;
	int in_chunk = (n/5) / (num_processes - num_processes / 2);	
	if(num_processes == 1) {
		in_chunk = n/5;
	}
	else {
		in_chunk = (n/5) / (num_processes - num_processes / 2);		
	}
	for (int idx = num_processes/2; idx < num_processes; ++idx) {
		int i = idx - num_processes/2;
		int low = i * in_chunk;
		int high = (i+1) * in_chunk;
		if(idx == num_processes - 1)
			high = n/5;
		thread *tmp = new thread(insert_nodes, idx, &new_keys, low, high);
		inthreads.push_back(tmp);
	}

	if(num_processes != 1) {
		for (int i = 0; i < del_num_processes; ++i)
			delthreads_2[i]->join();
	}
	for (int i = num_processes/2; i < num_processes; ++i)
		inthreads[i - num_processes/2]->join();
	auto stop_mix = high_resolution_clock::now(); 
	auto duration_mix = duration_cast<microseconds>(stop_mix - start_mix); 
	cout << "Time taken by insertion and deletion: "
         << duration_mix.count() << " microseconds" << endl; 
	printf("Check round 3\n");	
	// Check - deleted threads should not be accessible
	for (int i = 0; i < n/5; ++i) {
		if(getNode(remain_keys[i]) != NULL)
			printf("Key %d at position %d FOUND\n", remain_keys[i], i);
	}
	for (int i = n/5; i < n - n/5 - n/5; ++i) {
		if(getNode(remain_keys[i]) == NULL)
			printf("Old key %d at position %d not found\n", remain_keys[i], i);
	}
	for (int i = 0; i < n/5; ++i) {
		if(getNode(new_keys[i]) == NULL)
			printf("New key %d at position %d not found\n", new_keys[i], i);		
	}
	printf("Check round 3 Done\n");	
	
	if(rebal == 1) {				
		continueRebalancing = false;
		printf("Sleeping To Let threads complete the Rebalancings\n");	
		numRebalOps = 0;
		std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
		printf("Woke up\n");
		for (int i = 0; i < num_rebal_threads; ++i)
			rebal_threads[i]->join();		
	}		
	
	// Check balance of tree

	vi weighted_heights, leaf_wts, real_hts;
	int level = 0;
	int true_level = 0;
	check_balance(GLOBAL_ROOT->left->left, weighted_heights, leaf_wts, real_hts, level, true_level);

	int ht = weighted_heights[0];

	if(rebal == 1) {
		for (int i = 1; i < weighted_heights.size(); ++i) {
		if(weighted_heights[i] != ht)
			printf("Height(new) %d different from %d\n", weighted_heights[i], ht);
		}
	}	

	// printf("%d\n", ht);

	// Print true heights
	printf("TRUE HEIGHTS\n");
	map<int, int> ht_dist;
	for (int i = 0; i < real_hts.size(); ++i) {
		if(ht_dist.find(real_hts[i]) == ht_dist.end())
			ht_dist.insert(make_pair(real_hts[i], 1));
		else
			ht_dist[real_hts[i]]++;
		// printf("%d\n", real_hts[i]);
	}

	for (auto itr = ht_dist.begin(); itr != ht_dist.end(); itr++) {
		cout<<itr->first<<": "<<itr->second<<endl;
	}
	cout << "num_rebal_ops: "<<numRebalOps<<endl;
	cout << "insert_time: " << duration_insert.count() << endl; 
	cout << "delete_time: " << duration_del.count() << endl; 
	cout << "mix_time: " << duration_mix.count() << endl; 
	// return 0;
}

void check_balance(tr node, vi& heights, vi& leaf_wts, vi &real_hts, int level, int true_level) {
	
	if(node == NULL)
		return;

	if(isLeaf(node)) {
		heights.push_back(level + node->weight);
		leaf_wts.push_back(node->weight);
		real_hts.push_back(true_level);
		return;
	}
	// else
	if(rebal == 1) {
		if(node->weight > 1)
			printf("Current node overweight\n");
		if(node->weight == 0 && node->left != NULL && node->left->weight == 0)
			printf("Current node left red-red\n");
		if(node->weight == 0 && node->right != NULL && node->right->weight == 0)
			printf("Current node right red-red\n");
	}	
	check_balance(node->left, heights, leaf_wts, real_hts, level+node->weight, true_level+1);
	check_balance(node->right, heights, leaf_wts, real_hts, level+node->weight, true_level+1);
}