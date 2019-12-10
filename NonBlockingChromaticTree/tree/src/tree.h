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
SCXProvider<node, 100> * GLOBAL_SCX;
std::atomic<int> numRebalOps;

pair<int, bool> tryDelete(int key, int tid);


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
	GLOBAL_SCX = new SCXProvider<node, 100>(num_processes);


	GLOBAL_SCX->initNode(GLOBAL_ROOT);
	GLOBAL_SCX->initNode(GLOBAL_ROOT->left);
	GLOBAL_SCX->initNode(GLOBAL_ROOT->right);
}


// NOTE - the input to this can NOT be null!!
bool isLeaf(tr node) {
	return node->left == NULL || node->right == NULL;
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
		return make_pair(-1, false);
	}
	auto s0 = GLOBAL_SCX->llx(tid, n0);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(s0) || s0 == GLOBAL_SCX->FINALIZED) return make_pair(-1, false);
	GLOBAL_SCX->scxAddNode(tid, n0, false, s0);

	tr n1 = key < n0->key ? n0->left : n0->right;
	if(n1 == NULL) {
		return make_pair(-1, false);
	}
	auto s1 = GLOBAL_SCX->llx(tid, n1);
	if(!GLOBAL_SCX->isSuccessfulLLXResult(s1) || s1 == GLOBAL_SCX->FINALIZED) return make_pair(-1, false);
	GLOBAL_SCX->scxAddNode(tid, n1, true, s1);


	tr n2 = key < n1->key ? n1->left : n1->right;
	if(n2 == NULL) {
		return make_pair(-1, false);
	}
	if(n2->key != key) {
		return make_pair(0, false);
	}
	auto s2 = GLOBAL_SCX->llx(tid, n2);
	if(!GLOBAL_SCX->isSuccessfulLLXResult(s2) || s2 == GLOBAL_SCX->FINALIZED) return make_pair(-1, false);


	tr n3 = key < n1->key ? n1->right : n1->left;
	if(n3 == NULL) {
		return make_pair(-1, false);
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

	int weight = (n1->key == 0x7FFFFFFF || n0->key == 0x7FFFFFFF) ? 1 : n1->weight + n3->weight;
	auto temp = new struct node;

	initializeNode(temp, n3->key, weight, n3->left, n3->right);
	bool scxStatus = GLOBAL_SCX->scxExecute(tid, (void * volatile *) (key < n0->key ? &n0->left:&n0->right), n1, temp);
	if(scxStatus) return make_pair(0, weight > 1);
	else return make_pair(-1, false);
}

pair<int, bool> tryInsert(int key, int tid, tr leaf, tr parent) {
	// std::cout<<to_string(key) + "\n";

	if(parent == NULL) return make_pair(-1, false);
	GLOBAL_SCX->scxInit(tid);
	auto parent0 = GLOBAL_SCX->llx(tid, parent);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(parent0) || parent0 == GLOBAL_SCX->FINALIZED) return make_pair(-1, false);
	GLOBAL_SCX->scxAddNode(tid, parent, false, parent0);

	auto toChange = (void * volatile *) &(parent->left);
	if(leaf == NULL) return make_pair(-1, false);
	if(leaf == parent->left) {}
	else if(leaf == parent->right) toChange = (void * volatile *) &(parent->right);  
	else return make_pair(-1, false);

	auto leaf0 = GLOBAL_SCX->llx(tid, leaf);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(leaf0) || leaf0 == GLOBAL_SCX->FINALIZED) return make_pair(-1, false);
	GLOBAL_SCX->scxAddNode(tid, leaf, false, leaf0);

	auto new1 = new struct node;
	initializeNode(new1, key, 1, NULL, NULL);
	int weight = leaf->key == 0x7FFFFFFF ? 1 : leaf->weight - 1;

	auto new2 = new struct node;
	if(key < leaf->key) initializeNode(new2, leaf->key, weight, new1, leaf); 
	else initializeNode(new2, key, weight, leaf, new1);

	bool scxStatus = GLOBAL_SCX->scxExecute(tid, toChange, leaf, new2);
	if(scxStatus) return make_pair(0, (weight == parent->weight) && (weight == 0));
	else return make_pair(-1, false);
}

int insertKey(int key, int value, int tid) {
	std::vector<tr> searchAns = searchTree(key);
	std::pair<int, bool> answer = tryInsert(key, tid, searchAns[2], searchAns[1]);
	while(answer.first == -1) {
		searchAns = searchTree(key);
		answer = tryInsert(key, tid, searchAns[2], searchAns[1]);
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
// 			if(n3->weight > 1 || (n2->weight == 0 && n3->weight == 0)) {
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

bool tryRebalance1(tr parent, tr node, int tid) {

	if(parent == NULL) return false;
	GLOBAL_SCX->scxInit(tid);
	auto parent0 = GLOBAL_SCX->llx(tid, parent);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(parent0) || parent0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, parent, false, parent0);

	auto toChange = (void * volatile *) &(parent->left);
	if(node == NULL) return false;
	if(node == parent->left) {}
	else if(node == parent->right) toChange = (void * volatile *) &(parent->right);  
	else return false;

	auto node0 = GLOBAL_SCX->llx(tid, node);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(node0) || node0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, node, true, node0);

	if(node->weight <= 0 && parent != GLOBAL_ROOT->left) return false;
	if(node->left == NULL || node->right == NULL) {
		return false;
	}

	auto nodeLeft = node->left;
	auto nodeRight = node->right;

	auto nodeLeft0 = GLOBAL_SCX->llx(tid, nodeLeft);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(nodeLeft0) || nodeLeft0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, nodeLeft, true, nodeLeft0);

	if(nodeLeft->weight != 0) return false;

	auto nodeRight0 = GLOBAL_SCX->llx(tid, nodeRight);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(nodeRight0) || nodeRight0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, nodeRight, true, nodeRight0);	

	if(nodeRight->weight != 0) return false;


	auto newNode = new struct node;
	auto newLeft = new struct node;
	auto newRight = new struct node;
	
	initializeNode(newNode, node->key, node->weight - 1, newLeft, newRight);
	initializeNode(newLeft, nodeLeft->key, 1, nodeLeft->left, nodeLeft->right);
	initializeNode(newRight, nodeRight->key, 1, nodeRight->left, nodeRight->right);
	bool scxStatus = GLOBAL_SCX->scxExecute(tid, toChange, node, newNode);
	return scxStatus;
}

bool tryRebalance4(tr parent, tr node, int tid) {

	if(parent == NULL) return false;
	GLOBAL_SCX->scxInit(tid);
	auto parent0 = GLOBAL_SCX->llx(tid, parent);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(parent0) || parent0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, parent, false, parent0);

	auto toChange = (void * volatile *) &(parent->left);
	if(node == NULL) return false;
	if(node == parent->left) {}
	else if(node == parent->right) toChange = (void * volatile *) &(parent->right);  
	else return false;

	auto node0 = GLOBAL_SCX->llx(tid, node);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(node0) || node0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, node, true, node0);

	if(node->left == NULL || node->right == NULL) {
		return false;
	}

	auto nodeLeft = node->left;
	auto nodeRight = node->right;

	auto nodeLeft0 = GLOBAL_SCX->llx(tid, nodeLeft);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(nodeLeft0) || nodeLeft0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, nodeLeft, true, nodeLeft0);


	auto nodeRight0 = GLOBAL_SCX->llx(tid, nodeRight);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(nodeRight0) || nodeRight0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, nodeRight, true, nodeRight0);	

	if((nodeLeft->weight > 1 && nodeRight->weight > 0) || (nodeRight->weight > 1 && nodeLeft->weight > 0)) {}
	else return false;


	auto newNode = new struct node;
	auto newLeft = new struct node;
	auto newRight = new struct node;
	
	initializeNode(newNode, node->key, node->weight + 1, newLeft, newRight);
	initializeNode(newLeft, nodeLeft->key, nodeLeft->weight - 1, nodeLeft->left, nodeLeft->right);
	initializeNode(newRight, nodeRight->key, nodeRight->weight - 1, nodeRight->left, nodeRight->right);
	bool scxStatus = GLOBAL_SCX->scxExecute(tid, toChange, node, newNode);
	return scxStatus;
}

bool tryRebalance3A(tr parent, tr node, int tid) {

	if(parent == NULL) return false;
	GLOBAL_SCX->scxInit(tid);
	auto parent0 = GLOBAL_SCX->llx(tid, parent);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(parent0) || parent0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, parent, false, parent0);

	auto toChange = (void * volatile *) &(parent->left);
	if(node == NULL) return false;
	if(node == parent->left) {}
	else if(node == parent->right) toChange = (void * volatile *) &(parent->right);  
	else return false;

	auto node0 = GLOBAL_SCX->llx(tid, node);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(node0) || node0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, node, true, node0);

	if(node->left == NULL || node->right == NULL) {
		return false;
	}
	auto nodeLeft = node->left;
	auto nodeRight = node->right;
	if(nodeRight->weight <= 0) return false;

	auto nodeLeft0 = GLOBAL_SCX->llx(tid, nodeLeft);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(nodeLeft0) || nodeLeft0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, nodeLeft, true, nodeLeft0);

	if(nodeLeft->weight != 0) return false;
	if(nodeLeft->left == NULL || nodeLeft->right == NULL) return false;

	auto nodeLeftRight = nodeLeft->right;
	auto nodeLeftRight0 = GLOBAL_SCX->llx(tid, nodeLeftRight);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(nodeLeftRight0) || nodeLeftRight0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, nodeLeftRight, true, nodeLeftRight0);

	if(nodeLeftRight->weight != 0) return false;

	auto newNode = new struct node;
	auto newLeft = new struct node;
	auto newRight = new struct node;
	
	initializeNode(newNode, nodeLeftRight->key, node->weight, newLeft, newRight);
	initializeNode(newLeft, nodeLeft->key, 0, nodeLeft->left, nodeLeftRight->left);
	initializeNode(newRight, node->key, 0, nodeLeftRight->right, node->right);
	bool scxStatus = GLOBAL_SCX->scxExecute(tid, toChange, node, newNode);
	return scxStatus;
}


bool tryRebalance3B(tr parent, tr node, int tid) {

	if(parent == NULL) return false;
	GLOBAL_SCX->scxInit(tid);
	auto parent0 = GLOBAL_SCX->llx(tid, parent);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(parent0) || parent0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, parent, false, parent0);

	auto toChange = (void * volatile *) &(parent->right);
	if(node == NULL) return false;
	if(node == parent->right) {}
	else if(node == parent->left) toChange = (void * volatile *) &(parent->left);  
	else return false;

	auto node0 = GLOBAL_SCX->llx(tid, node);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(node0) || node0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, node, true, node0);

	if(node->right == NULL || node->left == NULL) {
		return false;
	}
	auto noderight = node->right;
	auto nodeleft = node->left;
	if(nodeleft->weight <= 0) return false;

	auto noderight0 = GLOBAL_SCX->llx(tid, noderight);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(noderight0) || noderight0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, noderight, true, noderight0);

	if(noderight->weight != 0) return false;
	if(noderight->right == NULL || noderight->left == NULL) return false;

	auto noderightleft = noderight->left;
	auto noderightleft0 = GLOBAL_SCX->llx(tid, noderightleft);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(noderightleft0) || noderightleft0 == GLOBAL_SCX->FINALIZED) return false;
	GLOBAL_SCX->scxAddNode(tid, noderightleft, true, noderightleft0);

	if(noderightleft->weight != 0) return false;

	auto newNode = new struct node;
	auto newright = new struct node;
	auto newleft = new struct node;
	
	initializeNode(newNode, noderightleft->key, node->weight, newleft, newright);
	initializeNode(newright, noderight->key, 0, noderightleft->right, noderight->right);
	initializeNode(newleft, node->key, 0, node->left, noderightleft->left);
	bool scxStatus = GLOBAL_SCX->scxExecute(tid, toChange, node, newNode);
	return scxStatus;
}

bool tryRebalance2A(tr parent, tr v, int tid) {
	
	if(parent == NULL) {
		return false;

	}
	GLOBAL_SCX->scxInit(tid);
	auto parent0 = GLOBAL_SCX->llx(tid, parent);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(parent0) || parent0 == GLOBAL_SCX->FINALIZED) {
		return false;
	}
	GLOBAL_SCX->scxAddNode(tid, parent, false, parent0);	

	// Pointer to be changed
	auto toChange = (void * volatile *) &(parent->left);
	if(v == NULL){
		return false;
	}
	auto v0 = GLOBAL_SCX->llx(tid, v);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(v0) || v0 == GLOBAL_SCX->FINALIZED) 
	{
		return false;
	}
	GLOBAL_SCX->scxAddNode(tid, v, true, v0);
	if(v == parent->left) {}
	else if(v == parent->right) toChange = (void * volatile *) &(parent->right);  
	else return false;


	// First mirror case
	tr C = v->right;
	if((C == NULL) || (C != NULL && C->weight == 0)){
		return false;
	}

	tr u = v->left;
	if(u == NULL || u->weight != 0) {
		return false;
	}

	auto u0 = GLOBAL_SCX->llx(tid, u);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(u0) || u0 == GLOBAL_SCX->FINALIZED) 
		return false;
	GLOBAL_SCX->scxAddNode(tid, u, true, u0);

	tr A = u->left;
	tr B = u->right;

	if(A == NULL || A->weight != 0 || B == NULL)
		return false;	

	auto u2 = new struct node;
	auto v2 = new struct node;
	
	initializeNode(u2, u->key, v->weight, A, v2);
	initializeNode(v2, v->key, u->weight, B, C);

	bool scxStatus = GLOBAL_SCX->scxExecute(tid, toChange, v, u2);

	// if(!scxStatus) {
	// 	delete u2;
	// 	delete v2;
	// }

	return scxStatus;
}

bool tryRebalance2B(tr parent, tr v, int tid) {
	
	if(parent == NULL)
		return false;
	GLOBAL_SCX->scxInit(tid);
	auto parent0 = GLOBAL_SCX->llx(tid, parent);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(parent0) || parent0 == GLOBAL_SCX->FINALIZED)
		return false;
	GLOBAL_SCX->scxAddNode(tid, parent, false, parent0);	

	// Pointer to be changed
	auto toChange = (void * volatile *) &(parent->left);
	if(v == NULL)
		return false;
	if(v == parent->left) {}
	else if(v == parent->right) toChange = (void * volatile *) &(parent->right);  
	else return false;

	auto v0 = GLOBAL_SCX->llx(tid, v);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(v0) || v0 == GLOBAL_SCX->FINALIZED) 
		return false;
	GLOBAL_SCX->scxAddNode(tid, v, true, v0);

	// First mirror case
	tr C = v->left;
	if((C == NULL) || (C != NULL && C->weight == 0))
		return false;

	tr u = v->right;
	if(u == NULL || u->weight != 0)
		return false;

	auto u0 = GLOBAL_SCX->llx(tid, u);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(u0) || u0 == GLOBAL_SCX->FINALIZED) 
		return false;
	GLOBAL_SCX->scxAddNode(tid, u, true, u0);	

	tr A = u->right;
	tr B = u->left;

	if(A == NULL || A->weight != 0 || B == NULL)
		return false;	

	auto u2 = new struct node;
	auto v2 = new struct node;
	
	initializeNode(u2, u->key, v->weight, v2, A);
	initializeNode(v2, v->key, u->weight, C, B);

	bool scxStatus = GLOBAL_SCX->scxExecute(tid, toChange, v, u2);

	// if(!scxStatus) {
	// 	delete u2;
	// 	delete v2;
	// }

	return scxStatus;
}

bool tryRebalance5A(tr parent, tr v, int tid) {

	if(parent == NULL)
		return false;
	GLOBAL_SCX->scxInit(tid);
	auto parent0 = GLOBAL_SCX->llx(tid, parent);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(parent0) || parent0 == GLOBAL_SCX->FINALIZED)
		return false;
	GLOBAL_SCX->scxAddNode(tid, parent, false, parent0);	

	// Pointer to be changed
	auto toChange = (void * volatile *) &(parent->left);
	if(v == NULL)
		return false;
	if(v == parent->left) {}
	else if(v == parent->right) toChange = (void * volatile *) &(parent->right);  
	else return false;

	auto v0 = GLOBAL_SCX->llx(tid, v);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(v0) || v0 == GLOBAL_SCX->FINALIZED) 
		return false;
	GLOBAL_SCX->scxAddNode(tid, v, true, v0);

	tr A = v->left;
	if(A == NULL || A->weight <= 1)
		return false;

	auto A0 = GLOBAL_SCX->llx(tid, A);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(A0) || A0 == GLOBAL_SCX->FINALIZED) 
		return false;
	GLOBAL_SCX->scxAddNode(tid, A, true, A0);

	tr Al = A->left, Ar = A->right;	

	tr u = v->right;
	if(u == NULL || u->weight != 0)
		return false;

	auto u0 = GLOBAL_SCX->llx(tid, u);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(u0) || u0 == GLOBAL_SCX->FINALIZED) 
		return false;
	GLOBAL_SCX->scxAddNode(tid, u, true, u0);

	tr B = u->left;
	tr C = u->right;
	if(B == NULL || B->weight <= 0 || C == NULL)
		return false;

	auto B0 = GLOBAL_SCX->llx(tid, B);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(B0) || B0 == GLOBAL_SCX->FINALIZED) 
		return false;
	GLOBAL_SCX->scxAddNode(tid, B, true, B0);

	tr Bl = B->left, Br = B->right;

	tr v2, u2, A2, B2;

	v2 = new struct node;
	u2 = new struct node;
	A2 = new struct node;
	B2 = new struct node;	

	initializeNode(A2, A->key, A->weight - 1, Al, Ar);
	initializeNode(B2, B->key, B->weight - 1, Bl, Br);
	initializeNode(v2, v->key, 1, A2, B2);
	initializeNode(u2, u->key, v->weight, v2, C);

	bool scxStatus = GLOBAL_SCX->scxExecute(tid, toChange, v, u2);

	return scxStatus;

}

bool tryRebalance5B(tr parent, tr v, int tid) {

	if(parent == NULL)
		return false;
	GLOBAL_SCX->scxInit(tid);
	auto parent0 = GLOBAL_SCX->llx(tid, parent);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(parent0) || parent0 == GLOBAL_SCX->FINALIZED)
		return false;
	GLOBAL_SCX->scxAddNode(tid, parent, false, parent0);	

	// Pointer to be changed
	auto toChange = (void * volatile *) &(parent->left);
	if(v == NULL)
		return false;
	if(v == parent->left) {}
	else if(v == parent->right) toChange = (void * volatile *) &(parent->right);  
	else return false;

	auto v0 = GLOBAL_SCX->llx(tid, v);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(v0) || v0 == GLOBAL_SCX->FINALIZED) 
		return false;
	GLOBAL_SCX->scxAddNode(tid, v, true, v0);

	tr A = v->right;
	if(A == NULL || A->weight <= 1)
		return false;

	auto A0 = GLOBAL_SCX->llx(tid, A);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(A0) || A0 == GLOBAL_SCX->FINALIZED) 
		return false;
	GLOBAL_SCX->scxAddNode(tid, A, true, A0);

	tr Al = A->left, Ar = A->right;	

	tr u = v->left;
	if(u == NULL || u->weight != 0)
		return false;

	auto u0 = GLOBAL_SCX->llx(tid, u);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(u0) || u0 == GLOBAL_SCX->FINALIZED) 
		return false;
	GLOBAL_SCX->scxAddNode(tid, u, true, u0);

	tr B = u->right;
	tr C = u->left;
	if(B == NULL || B->weight <= 0 || C == NULL)
		return false;

	auto B0 = GLOBAL_SCX->llx(tid, B);
	// Double Check although the first function checks its finalized or not
	if(!GLOBAL_SCX->isSuccessfulLLXResult(B0) || B0 == GLOBAL_SCX->FINALIZED) 
		return false;
	GLOBAL_SCX->scxAddNode(tid, B, true, B0);

	tr Bl = B->left, Br = B->right;

	tr v2, u2, A2, B2;

	v2 = new struct node;
	u2 = new struct node;
	A2 = new struct node;
	B2 = new struct node;	

	initializeNode(A2, A->key, A->weight - 1, Al, Ar);
	initializeNode(B2, B->key, B->weight - 1, Bl, Br);
	initializeNode(v2, v->key, 1, B2, A2);
	initializeNode(u2, u->key, v->weight, C, v2);

	bool scxStatus = GLOBAL_SCX->scxExecute(tid, toChange, v, u2);

	return scxStatus;

}

bool checkCase4(tr node) {
	bool check1 = (node->left->weight > 1) && (node->right->weight > 0);
	bool check2 = (node->left->weight > 0) && (node->right->weight > 1);

	return check1 || check2; 
}
// 0 - no case
// 1  - on left
// 2 - on right
// Incoming is NOT leaf node
int checkCase2(tr node) {

	bool leftCheck = (!isLeaf(node->left)) && 
					 (node->left->weight == 0) && 
					 (node->left->left->weight == 0) && 
					 (node->right->weight > 0);

	bool rightCheck = (!isLeaf(node->right)) && 
					 (node->right->weight == 0) && 
					 (node->right->right->weight == 0) && 
					 (node->left->weight > 0);

	if(leftCheck)
		return 1;
	else if(rightCheck)
		return 2;
	else
		return 0;

}

void rebalance(tr node, tr parent, int tid) {
	int case2_leftright;
	bool rebalanceSucceeded = true;
	bool rebalanceRun = false;
	if(node == NULL)
		return;

	if(node->left == NULL && node->right == NULL) return; 

	bool isLeft = parent->left == node;

	if(node->left->weight == 0 && node->right->weight == 0 && (parent == GLOBAL_ROOT->left || node->weight > 0)) {
		rebalanceSucceeded = tryRebalance1(parent, node, tid);
		rebalanceRun = true;
	} else if((case2_leftright = checkCase2(node)) != 0) {
		if(case2_leftright == 1) {
			rebalanceSucceeded = tryRebalance2A(parent, node, tid);
		} else {
			rebalanceSucceeded = tryRebalance2B(parent, node, tid);
		}
		rebalanceRun = true;
	} else if((node->left->weight == 0 && node->left->right != NULL && node->left->right->weight == 0 && node->right->weight > 0) ) {
		rebalanceSucceeded = tryRebalance3A(parent, node, tid);
		rebalanceRun = true;
	} else if((node->right->weight == 0 && node->right->left != NULL && node->right->left->weight == 0 && node->left->weight > 0)) {
		rebalanceSucceeded = tryRebalance3B(parent, node, tid);
		rebalanceRun = true;
	} else if(checkCase4(node)) {
		rebalanceSucceeded = tryRebalance4(parent, node, tid);
		rebalanceRun = true;
	} else if(node->left->weight > 1 && node->right->weight == 0 && !isLeaf(node->right) && node->right->left->weight > 0) {
		rebalanceSucceeded = tryRebalance5A(parent, node, tid);	
		rebalanceRun = true;
	} else if(node->right->weight > 1 && node->left->weight == 0 && !isLeaf(node->left) && node->left->right->weight > 0) {
		rebalanceSucceeded = tryRebalance5B(parent, node, tid);
		rebalanceRun = true;
	}

	if(rebalanceRun) {
		if(rebalanceSucceeded) {
			// std::cout<<"Rebalance Succeeded"+to_string(x)+"\n";
			numRebalOps++;
		} //else {
		// 	// std::cout<<"Rebalance Failed \n"+to_string(x)+"\n";
		// }
	} else {
		if(isLeft) {
			rebalance(parent->left->left, parent->left, tid);
			rebalance(parent->left->right, parent->left, tid);
		} else {
			rebalance(parent->right->left, parent->right, tid);
			rebalance(parent->right->right, parent->right, tid);
		}
	}
}

void rebalancingThreadOperation(int tid) {
	rebalance(GLOBAL_ROOT->left->left, GLOBAL_ROOT->left, tid);
	rebalance(GLOBAL_ROOT->left->right, GLOBAL_ROOT->left, tid);

}
