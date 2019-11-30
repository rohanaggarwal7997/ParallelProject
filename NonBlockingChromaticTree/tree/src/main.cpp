#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <random>
#include "tree.h"

using namespace std;

typedef vector<int> vi;

const int n = 10000;

// Check if tree satisfies balance conditions
void check_balance(tr root, vi& heights, vi& leaf_wts, int level);

int main(int argc, char const *argv[])
{

	init(16);
	deleteKey(0, 0);
	deleteKey(0, 0);
	deleteKey(0, 0);
// 	vi keys(n);
// 	vi del(n/10);

// 	for (int i = 0; i < n; ++i)
// 		keys[i] = i;

// 	srand(42);
// 	random_shuffle(keys.begin(), keys.end());

// 	GLOBAL_ROOT = NULL;
// 	for (int i = 0; i < n; ++i)
// 		insert(GLOBAL_ROOT, keys[i]);

// 	// Can we find all keys?
// 	for (int i = 0; i < n; ++i) {
// 		if(search(GLOBAL_ROOT, keys[i]) == NULL)
// 			printf("Key (%d, %d) not found\n", keys[i], i);
// 	}

// 	// Important nvariant - 

// 	vi heights;
// 	vi leaf_wts;
// 	// Rebalance
// 	rebalance(GLOBAL_ROOT, NULL);
// 	check_balance(GLOBAL_ROOT, heights, leaf_wts, 0);

// 	// Check if all the elements are same
// 	printf("Number of height elements = %d\n", heights.size());
// 	int ht = heights[0];
// 	for (int i = 1; i < heights.size(); ++i) {
// 		if(heights[i] != ht)
// 			printf("Height %d different from %d\n", heights[i], ht);
// 	}

// 	// Can we find all keys?
// 	for (int i = 0; i < n; ++i) {
// 		if(search(GLOBAL_ROOT, keys[i]) == NULL)
// 			printf("Key %d not found after initial rebalance\n", keys[i]);
// 	}	


// 	// Delete random keys
// 	for (int i = 0; i < n/10; ++i)
// 		remove(GLOBAL_ROOT, keys[i*10]);

// 	// Can we find all the keys?
// 	for (int i = 0; i < n; ++i) {
// 		if(n % 10 == 0)
// 			continue;
// 		if(search(GLOBAL_ROOT, keys[i]) == NULL)
// 			printf("Key %d not found after deleting keys\n", keys[i]);
// 	}

// 	vi heights_new;
// 	vi leaf_wts_new;
// 	rebalance(GLOBAL_ROOT, NULL);
// 	check_balance(GLOBAL_ROOT, heights_new, leaf_wts_new, 0);

// 	ht = heights_new[0];
// 	for (int i = 1; i < heights_new.size(); ++i) {
// 		if(heights_new[i] != ht)
// 			printf("Height(new) %d different from %d\n", heights_new[i], ht);
// 	}

// 	// Can we still find all the keys?
// 	for (int i = 0; i < n; ++i) {
// 		if(n % 10 == 0)
// 			continue;
// 		if(search(GLOBAL_ROOT, keys[i]) == NULL)
// 			printf("Key %d not found after deleting keys and rebalancing\n", keys[i]);
// 	}

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