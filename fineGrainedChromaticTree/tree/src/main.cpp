#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <random>
#include <thread>
#include <chrono>
#include <iostream> 
#include "tree.h"

using namespace std;
using namespace std::chrono; 

typedef vector<int> vi;

int n;
int num_processes;

// Check if tree satisfies balance conditions
void check_balance(tr root, vi& heights, vi& leaf_wts, int level);

void insert_nodes(vi *inKeys, int low, int high) {
	printf("INSERT (%d,%d)\n", low, high);
	for (int i = low; i < high; ++i)
		insert(GLOBAL_ROOT, (*inKeys)[i]);
}

void delete_nodes(vi *delKeys, int low, int high) {
	printf("DELETE (%d,%d)\n", low, high);
	for (int i = low; i < high; ++i)
		remove(GLOBAL_ROOT, (*delKeys)[i]);
}

int main(int argc, char const *argv[])
{
	if(argc == 1) {
		printf("Enter the number of processes and number of nodes\n");
		return 1;
	}

	num_processes = atoi(argv[1]);
	n = atoi(argv[2]);
	printf("Number of processes = %d\n", num_processes);
	printf("Number of nodes = %d\n", n);

	vi keys(n);
	vi del(n/10);

	for (int i = 0; i < n; ++i)
		keys[i] = i;

	srand(42);
	random_shuffle(keys.begin(), keys.end());
	printf("KEYS\n");
	for (int i = 0; i < n; ++i)
		printf("%d,", keys[i]);

	printf("\n");

	GLOBAL_ROOT = NULL;

	// Insert the keys
	vector<thread *> runthreads;
	int chunksize = n / num_processes;
	printf("Commence insertion\n");
	auto start = high_resolution_clock::now();
	for (int i = 0; i < num_processes; ++i) {
		int low = i * chunksize;
		int high = (i+1) * chunksize;
		if(i == num_processes - 1)
			high = n;
		thread *tmp = new thread(insert_nodes, &keys, low, high);
		runthreads.push_back(tmp);
	}

	for (int i = 0; i < num_processes; ++i)
		runthreads[i]->join();
	auto stop = high_resolution_clock::now(); 
	auto duration = duration_cast<microseconds>(stop - start); 
	printf("End insertion\n");
	cout << "Time taken by insertion: "
         << duration.count() << " microseconds" << endl; 
	

	// Can we find all keys?
	for (int i = 0; i < n; ++i) {
		if(search(GLOBAL_ROOT, keys[i]) == NULL)
			printf("Key (%d, %d) not found\n", keys[i], i);
	}

	// Important nvariant - 

	vi heights;
	vi leaf_wts;
	// Rebalance
	rebalance(GLOBAL_ROOT, NULL);
	check_balance(GLOBAL_ROOT, heights, leaf_wts, 0);

	// Check if all the elements are same
	printf("Number of height elements = %d\n", heights.size());
	int ht = heights[0];
	// for (int i = 1; i < heights.size(); ++i) {
	// 	if(heights[i] != ht)
	// 		printf("Height %d different from %d\n", heights[i], ht);
	// }

	// Can we find all keys?
	for (int i = 0; i < n; ++i) {
		if(search(GLOBAL_ROOT, keys[i]) == NULL)
			printf("Key %d not found after initial rebalance\n", keys[i]);
	}	


	// Delete random keys
	vi del_keys(n/10);
	for (int i = 0; i < n/10; ++i)
		del_keys[i] = keys[i*10];
	vector<thread *> delthreads;
	int delchunk = (n/10) / num_processes;
	printf("Commence deletion\n");
	start = high_resolution_clock::now();
	for (int i = 0; i < num_processes; ++i)
	{
		int low = i * delchunk;
		int high = (i+1) * delchunk;
		if(i == num_processes - 1)
			high = n/10;
		thread *tmp = new thread(delete_nodes, &del_keys, low, high);
		delthreads.push_back(tmp);
	}
	
	for (int i = 0; i < num_processes; ++i)
		delthreads[i]->join();
	stop = high_resolution_clock::now(); 
	duration = duration_cast<microseconds>(stop - start); 
	printf("End deletion\n");

	cout << "Time taken by deletion: "
         << duration.count() << " microseconds" << endl; 

	// for (int i = 0; i < n/10; ++i)
	// 	remove(GLOBAL_ROOT, keys[i*10]);

	// Can we find all the keys?
	for (int i = 0; i < n; ++i) {
		auto found_or_not = search(GLOBAL_ROOT, keys[i]); 
		if(i % 10 == 0) {
			if(found_or_not != NULL)
				printf("Key %d FOUND after deleting keys\n", keys[i]);
		}
		else {
			if(found_or_not == NULL)
				printf("Key %d not found after deleting keys\n", keys[i]);
		}		
	}

	vi heights_new;
	vi leaf_wts_new;
	rebalance(GLOBAL_ROOT, NULL);
	check_balance(GLOBAL_ROOT, heights_new, leaf_wts_new, 0);

	ht = heights_new[0];
	// for (int i = 1; i < heights_new.size(); ++i) {
	// 	if(heights_new[i] != ht)
	// 		printf("Height(new) %d different from %d\n", heights_new[i], ht);
	// }

	// Can we still find all the keys?
	for (int i = 0; i < n; ++i) {
		if(n % 10 == 0)
			continue;
		if(search(GLOBAL_ROOT, keys[i]) == NULL)
			printf("Key %d not found after deleting keys and rebalancing\n", keys[i]);
	}

	return 0;
}

void check_balance(tr node, vi& heights, vi& leaf_wts, int level) {
	
	if(node == NULL)
		return;

	if(isLeaf(node)) {
		heights.push_back(level + node->weight);
		leaf_wts.push_back(node->weight);
		return;
	}
	// else
	check_balance(node->left, heights, leaf_wts, level+node->weight);
	check_balance(node->right, heights, leaf_wts, level+node->weight);
}