#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <random>
#include <thread>
#include "tree.h"

using namespace std;

typedef vector<int> vi;

const int n = 10000;
const int num_processes = 12;


void insert_nodes(int tid, vi *inKeys, int low, int high) {
	for (int i = low; i < high; ++i)
		insertKey((*inKeys)[i], 0, tid);
}

void delete_nodes(int tid, vi *delKeys, int low, int high) {
	for (int i = low; i < high; ++i)
		deleteKey((*delKeys)[i], tid);
}

int main(int argc, char const *argv[])
{
	vi keys(n);
	// Half to be deleted in first round, second half in second round
	vi del(n/5);

	for (int i = 0; i < n; ++i)
		keys[i] = i;	

	// Initialize tree
	init(num_processes);
	
	// Shuffle keys
	// Insert keys in parallel 
	srand(42);
	random_shuffle(keys.begin(), keys.end());

	vector<thread *> runthreads;

	int chunksize = n / num_processes;

	// PARALLEL
	printf("Commence insertion\n");
	for (int i = 0; i < num_processes; ++i) {
		int low = i * chunksize;
		int high = min((i+1) * chunksize, n);
		thread *tmp = new thread(insert_nodes, i, &keys, low, high);
		runthreads.push_back(tmp);
	}

	// Join on the threads
	for (int i = 0; i < num_processes; ++i)
		runthreads[i]->join();
	printf("End insertion\n");

	// Check - all can be reached?
	printf("Check round 1\n");
	for (int i = 0; i < n; ++i) {
		if(getNode(keys[i]) == NULL)
			printf("Key %d at posn %d not found", keys[i], i);
	}
	printf("Check round 1 Done\n");

	// Shuffle again
	random_shuffle(keys.begin(), keys.end());
	int num_del = n/5;

	vi remain_keys(n - n/5);

	for (int i = n/5; i < n; ++i)
		remain_keys[i - n/5] = keys[i];

	// Delete keys in parallel
	// PARALLEL
	printf("Commence deletion\n");
	int del_chunk = (n/5) / num_processes;
	vector<thread *> delthreads;
	for (int i = 0; i < num_processes; ++i) {
		int low = i * del_chunk;
		int high = min((i+1) * del_chunk, n/5);
		thread* tmp = new thread(delete_nodes, i, &keys, low, high);
		delthreads.push_back(tmp);
	}

	for (int i = 0; i < num_processes; ++i)
		delthreads[i]->join();
	printf("Deletion done\n");


	printf("Check round 2\n");
	for (int i = 0; i < n/5; ++i) {
		if(getNode(keys[i]) != NULL)
			printf("Key %d at posn %d FOUND\n", keys[i], i);
	}
	for (int i = 0; i < n - n/5; ++i) {
		if(getNode(remain_keys[i]) != NULL)
			printf("Key %d at posn %d not found\n", remain_keys[i], i);
	}
	printf("Check round 2 Done\n");

	// Shuffle remaining keys again
	random_shuffle(remain_keys.begin(), remain_keys.end());

	// New keys
	vi new_keys(n/5);
	for (int i = 0; i < n/5; ++i)
		new_keys[i] = i + n;

	random_shuffle(new_keys.begin(), new_keys.end());
	vector<thread *> delthreads_2;
	int new_chunk = del_chunk * 2;
	for (int i = 0; i < num_processes/2; ++i) {
		int low = i * new_chunk;
		int high = min((i+1) * new_chunk, n/5);
		thread *tmp = new thread(delete_nodes, i, &remain_keys, low, high);
		delthreads_2.push_back(tmp);
	}

	vector<thread *> inthreads;
	for (int idx = num_processes/2; idx < num_processes; ++idx) {
		int i = idx - num_processes/2;
		int low = i * new_chunk;
		int high = min((i+1) * new_chunk, n/5);
		thread *tmp = new thread(insert_nodes, idx, &new_keys, low, high);
		inthreads.push_back(tmp);
	}

	for (int i = 0; i < num_processes/2; ++i)
		delthreads_2[i]->join();
	for (int i = 0; i < num_processes/2; ++i)
		inthreads[i]->join();

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

	return 0;
}
