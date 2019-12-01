#include <stdlib.h>
#include <algorithm>    // std::swap
#include "common/scx_provider.h"
#include<bits/stdc++.h>

using namespace std;

typedef struct node {
	int key;
	int value;
	int weight;
	node *left;
	node *right;
	volatile int marked;
	volatile scx_handle_t scxPtr;

} * tr;

tr GLOBAL_ROOT;
SCXProvider<node, 10> * GLOBAL_SCX;

pair<int, bool> tryDelete(int key, int tid);



void rebalance(tr node, tr parent) {
	return;
}

void initializeNode(tr x, int key, int weight, tr left, tr right) {

	x->key = key;
	x->weight = weight;
	x->left = left;
	x->right = right;
	GLOBAL_SCX->initNode(x);
}

void init(int num_processes) {
	GLOBAL_ROOT = new struct node;
	GLOBAL_ROOT->key = 0x7FFFFFFF;
	GLOBAL_ROOT->weight = 1;
	GLOBAL_ROOT->left = new struct node;
	GLOBAL_ROOT->right = new struct node;
	GLOBAL_ROOT->left->key = 0x7FFFFFFF;
	GLOBAL_ROOT->right->key = 0x7FFFFFFF;
	GLOBAL_ROOT->left->left = NULL;
	GLOBAL_ROOT->left->right = NULL;
	GLOBAL_ROOT->right->left = NULL;
	GLOBAL_ROOT->right->right = NULL;
	GLOBAL_ROOT->left->weight = 1;
	GLOBAL_ROOT->right->weight = 1;
	GLOBAL_SCX = new SCXProvider<node, 10>(num_processes);


	GLOBAL_SCX->initNode(GLOBAL_ROOT);
	GLOBAL_SCX->initNode(GLOBAL_ROOT->left);
	GLOBAL_SCX->initNode(GLOBAL_ROOT->right);
}


// NOTE - the input to this can NOT be null!!
bool isLeaf(tr node) {
	return node->left == NULL && node->right == NULL;
}


std::vector <tr> searchTree(int key) {

	tr n0, n1, n2;
	n0 = NULL;
	n1 = GLOBAL_ROOT;
	n2 = n1->left;

	while(!isLeaf(n2)) {

		n0 = n1;
		n1 = n2;
		n2 = key < n1->key ? n1->left : n1->right;
	}

	std::vector<tr> ans;
	ans.push_back(n0);
	ans.push_back(n1);
	ans.push_back(n2);

	return ans;
}

tr getNode(int key) {

	std::vector<tr> ans = searchTree(key);
	return ans[2]->key == key ? ans[2] : NULL;
}

int deleteKey(int key, int tid) {

	std::pair<int, bool> answer = tryDelete(key, tid);
	while(answer.first == -1) {
		answer = tryDelete(key, tid);
	}

	if(answer.second) {
		// cleanUp(key, tid);
	}

	return answer.first;
}

pair<int, bool> tryDelete(int key, int tid) {

	std::vector<tr> searchAns = searchTree(key);
	tr n0 = searchAns[0];

	GLOBAL_SCX->scxInit(tid);

	if(n0 == NULL) {
		return make_pair(0, false);
	}
	auto s0 = GLOBAL_SCX->llx(tid, n0);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(s0) || s0 == GLOBAL_SCX->FINALIZED) return make_pair(-1, false);
	GLOBAL_SCX->scxAddNode(tid, n0, false, s0);

	tr n1 = key < n0->key ? n0->left : n0->right;
	if(n1 == NULL) {
		return make_pair(0, false);
	}
	auto s1 = GLOBAL_SCX->llx(tid, n1);
	if(!GLOBAL_SCX->isSuccessfulLLXResult(s1) || s1 == GLOBAL_SCX->FINALIZED) return make_pair(-1, false);
	GLOBAL_SCX->scxAddNode(tid, n1, true, s1);


	tr n2 = key < n1->key ? n1->left : n1->right;
	if(n2 == NULL) {
		return make_pair(0, false);
	}
	if(n2->key != key) {
		return make_pair(0, false);
	}
	auto s2 = GLOBAL_SCX->llx(tid, n2);
	if(!GLOBAL_SCX->isSuccessfulLLXResult(s2) || s2 == GLOBAL_SCX->FINALIZED) return make_pair(-1, false);


	tr n3 = key < n1->key ? n1->right : n1->left;
	if(n3 == NULL) {
		return make_pair(0, false);
	}
	auto s3 = GLOBAL_SCX->llx(tid, n3);
	if(!GLOBAL_SCX->isSuccessfulLLXResult(s3) || s3 == GLOBAL_SCX->FINALIZED) return make_pair(-1, false);
	
	if(key < n1->key) {
		GLOBAL_SCX->scxAddNode(tid, n2, true, s2);
		GLOBAL_SCX->scxAddNode(tid, n3, true, s3);
	} else {
		GLOBAL_SCX->scxAddNode(tid, n3, true, s3);
		GLOBAL_SCX->scxAddNode(tid, n2, true, s2);
	}

	int weight = (n1->key == 0x7FFFFFFF or n0->key == 0x7FFFFFFF) ? 1 : n1->weight + n3->weight;
	auto temp = new struct node;
	initializeNode(temp, n3->key, weight, n3->left, n3->right);
	bool scxStatus = GLOBAL_SCX->scxExecute(tid, (void * volatile *) (key < n0->key ? &n0->left:&n0->right), n1, temp);
	if(scxStatus) return make_pair(0, weight > 1);
	else return make_pair(-1, false);
}

pair<int, bool> tryInsert(int key, int tid, tr leaf, tr parent) {

	GLOBAL_SCX->scxInit(tid);

	auto parent0 = GLOBAL_SCX->llx(tid, parent);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(parent0) || parent0 == GLOBAL_SCX->FINALIZED) return make_pair(-1, false);
	GLOBAL_SCX->scxAddNode(tid, parent, false, parent0);

	auto toChange = (void * volatile *) &parent->left;
	if(leaf == parent->left) {}
	else if(leaf == parent->right) toChange = (void * volatile *) &parent->right;  
	else return make_pair(-1, false);

	auto leaf0 = GLOBAL_SCX->llx(tid, leaf);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(leaf0) || leaf0 == GLOBAL_SCX->FINALIZED) return make_pair(-1, false);
	GLOBAL_SCX->scxAddNode(tid, leaf, true, leaf0);

	auto new1 = new struct node;
	initializeNode(new1, key, 1, NULL, NULL);
	int weight = leaf->key == 0x7FFFFFFF ? 1 : leaf->weight - 1;

	auto new2 = new struct node;
	if(key < leaf->key) initializeNode(new2, leaf->key, weight, new1, leaf); 
	else initializeNode(new2, key, weight, leaf, new1);

	bool scxStatus = GLOBAL_SCX->scxExecute(tid, toChange, leaf, new2);
	if(scxStatus) return make_pair(0, weight > 1);
	else return make_pair(-1, false);
}

int insertKey(int key, int value, int tid) {
	std::vector<tr> searchAns = searchTree(key);
	std::pair<int, bool> answer = tryInsert(key, value, searchAns[2], searchAns[1]);
	while(answer.first == -1) {
		searchAns = searchTree(key);
		answer = tryInsert(key, value, searchAns[2], searchAns[1]);
	}

	if(answer.second) {
		// cleanUp(key, tid);
	}

	return answer.first;
}

// void cleanUp(int key, int tid) {
// 	while(1) {
// 		tr n0, n1, n2, n3;
// 		n0 = NULL;
// 		n1 = NULL;
// 		n2 = GLOBAL_ROOT;
// 		n3 = GLOBAL_ROOT->left;

// 		while(1) {
// 			if(n3->weight > 1 || (n2->weight == 0 || n3->weight == 0)) {
// 				// Weight or Red Red Violation
// 				tryRebalance(n0, n1, n2, n3);
// 				break; /*Search Again from Top*/
// 			} else if(isLeaf(n3)) return;

// 			n0 = n1;
// 			n1 = n2;
// 			n2 = n3;
// 			n3 = key < n3->key ? n3->left : n3->right;
// 		}
// 	}
// }

