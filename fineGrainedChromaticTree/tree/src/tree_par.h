#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <algorithm>    // std::swap
#include <thread>
#include <mutex>
#include <shared_mutex>

using namespace std;

std::shared_mutex tree_init_lock;

typedef struct node {
	int key;
	int value;
	int weight;
	node *left;
	node *right;

	std::shared_mutex rwlock;

} * tr;

tr GLOBAL_ROOT;

bool isLeaf(tr node);

bool checkCase4(tr node) {
	bool check1 = (node->left->weight > 1) && (node->right->weight > 0);
	bool check2 = (node->left->weight > 0) && (node->right->weight > 1);

	return check1 || check2; 
}

void balanceCase4(tr node, tr parent) {
	node->left->weight--;
	node->right->weight--;

	if(parent != NULL)
		node->weight++;
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

void balanceCase2(tr node, tr parent, int whichEdgeRed) {	
	tr v, u, B;
	// Left edge is red
	if(whichEdgeRed == 1) {
		v = node;
		u = node->left;
		B = node->left->right;

		v->left = B;
		u->right = v;
	}
	// Right edge is red
	else {
		v = node;
		u = node->right;
		B = node->right->left;

		v->right = B;
		u->left = v;
	}

	if(parent == NULL) GLOBAL_ROOT = u;
	else {
		if(parent->left == v)
			parent->left = u;
		else
			parent->right = u;
	}

	std::swap(u->weight, v->weight);

}


void rebalance(tr node, tr parent) {

	

	int case2_leftright;

	if(node == NULL)
		return;

	if(node->left == NULL && node->right == NULL) return; 

	if(node->left->weight == 0 && node->right->weight == 0 && (parent == NULL || node->weight > 0)) {
		
		node->left->weight = 1;
		node->right->weight = 1;

		if(parent == NULL) {
			// We are at root Do Nothing
		} else {
			node->weight -= 1;
		}
	} else if((case2_leftright = checkCase2(node)) != 0) {
		balanceCase2(node, parent, case2_leftright);
	} else if((node->left->weight == 0 && node->left->right != NULL && node->left->right->weight == 0 && node->right->weight > 0) ||
		(node->right->weight == 0 && node->right->left != NULL && node->right->left->weight == 0 && node->left->weight > 0)) {
		
		if(node->left->weight == 0) {
			tr u = node->left;
			tr A = u->left;
			tr T = u->right;
			tr B = T->left;
			tr C = T->right;
			tr D = node->right;

			// T is not leaf
				if(parent == NULL) GLOBAL_ROOT = T;
				else {
					if(node == parent->left) parent->left = T;
					else parent->right = T;
				}

				std::swap(T->weight, node->weight);

				T->left = u;
				T->right = node;
				u->left = A;
				u->right = B;
				node->left = C;
				node->right = D;
			

		} else if(node->right->weight == 0) {
			tr u = node->right;
			tr A = u->right;
			tr T = u->left;
			tr B = T->right;
			tr C = T->left;
			tr D = node->left;


				if(parent == NULL) GLOBAL_ROOT = T;
				else {
					if(node == parent->left) parent->left = T;
					else parent->right = T;
				}

				std::swap(T->weight, node->weight);

				T->right = u;
				T->left = node;
				u->right = A;
				u->left = B;
				node->right = C;
				node->left = D;
			
		}

	} else if(checkCase4(node)) {
		balanceCase4(node, parent);
	} else if((node->left->weight > 1 && node->right->weight == 0 && !isLeaf(node->right) && node->right->left->weight > 0) ||
		(node->right->weight > 1 && node->left->weight == 0 && !isLeaf(node->left) && node->left->right->weight > 0)) {

		if(node->right->weight == 0) {

			tr A = node->left;
			tr u = node->right;
			tr B = u->left;
			tr C = u->right;

			if(parent == NULL) GLOBAL_ROOT = u;
			else {
				if(node == parent->left) parent->left = u;
				else parent->right = u;
			}

			std::swap(u->weight, node->weight);
			u->left = node;
			u->right = C;
			node->left = A;
			node->right = B;

			node->left->weight--;
			node->right->weight--;
			node->weight++;

		} else if(node->left->weight == 0) {

			tr A = node->right;
			tr u = node->left;
			tr B = u->right;
			tr C = u->left;

			if(parent == NULL) GLOBAL_ROOT = u;
			else {
				if(node == parent->left) parent->left = u;
				else parent->right = u;
			}

			std::swap(u->weight, node->weight);
			u->right = node;
			u->left = C;
			node->right = A;
			node->left = B;


			node->right->weight--;
			node->left->weight--;
			node->weight++;
		}
		
	}	

	rebalance(node->left, node);
	rebalance(node->right, node);

}



// NOTE - the input to this can NOT be null!!
bool isLeaf(tr node) {
	return node->left == NULL && node->right == NULL;
}

tr search(tr root, int key) {
	// printf("SEARC\n");
	// std::cout<<"SEARC"<<endl;
	tr ptr = root;
	ptr->rwlock.lock_shared();
	while(true) {
		if(isLeaf(ptr)) {
			if(ptr->key == key) {
				// printf("Hound %d\n", key);
				ptr->rwlock.unlock_shared();
				return ptr;
			}
			else {
				// printf("Found %d instead of %d\n", ptr->key, key);
				ptr->rwlock.unlock_shared();
				return NULL;
			}
		}

		// Assuming key can ONLY be leaf coz of locking

		// else
		if(key <= ptr->key) {
			ptr->left->rwlock.lock_shared();
			ptr->rwlock.unlock_shared();
			ptr = ptr->left;
		}
		else {
			ptr->right->rwlock.lock_shared();
			ptr->rwlock.unlock_shared();
			ptr = ptr->right;
		}
	}
}

void recursiveInsert(tr parent, tr root, int key) {

	// Parent and root are supposed to be locked when you come in
	// root is never NULL	

	if(isLeaf(root)) {
		// Release parent
		if(parent != NULL)
			parent->rwlock.unlock();

		// Leaf Node Code
		if(root->key == key) {			
			root->rwlock.unlock();
			return /*root*/;
		}

		tr leafLeft = new struct node;
		tr leafRight = new struct node;

		leafLeft->weight = 1;
		leafRight->weight = 1;
		leafLeft->left = NULL;
		leafLeft->right = NULL;
		leafRight->left = NULL;
		leafRight->right = NULL;

		root->left = leafLeft;
		root->right = leafRight;
		
		// leafLeft->key = root->key < key ? root->key : key;
		// leafRight->key = root->key != leafLeft->key ? root->key : key;

		if(key <= root->key) {
			leafLeft->key = key;
			leafRight->key = root->key;
			root->key = leafLeft->key;
		}
		else {
			leafLeft->key = root->key;
			leafRight->key = key;
		}

		// root->key = root->right->key;

		root->weight -= 1;

		// Release root
		root->rwlock.unlock();

		return /*root*/;

	} else {
		// root left and right are NOT NULL!!		
		if(key <= root->key) {			
			root->left->rwlock.lock();
			// Release parent lock
			if(parent != NULL)
				parent->rwlock.unlock();
			recursiveInsert(root, root->left, key);
		}
		else{			
			root->right->rwlock.lock();
			// Release parent lock
			if(parent != NULL)
				parent->rwlock.unlock();
			recursiveInsert(root, root->right, key);
		}
		return /*root*/;
	}
}

void insert(tr root, int key) {

	if(root == NULL) {
		tree_init_lock.lock();		
		GLOBAL_ROOT = new struct node;
		GLOBAL_ROOT->key = key;
		GLOBAL_ROOT->weight = 1;
		GLOBAL_ROOT->left = NULL;
		GLOBAL_ROOT->right = NULL;
		tree_init_lock.unlock();
	} else {
		root->rwlock.lock();
		recursiveInsert(NULL, root, key);
	}
}



void recursiveRemove(tr parent, tr root, int key) {

	// Assume parent is already locked
	// root->rwlock.lock();

	if(isLeaf(root)) {
		// Leaf Node Code
		if(root->key == key) { // Remove
			tr sibling = NULL;
			if(parent->left == root) {
				parent->right->rwlock.lock();
				sibling = parent->right;

			}
			else { // CAN ASSUME right child and no other condition coz of locking
				parent->left->rwlock.lock();
				sibling = parent->left;
			}

			parent->weight += sibling->weight;
			parent->key = sibling->key;
			parent->left = sibling->left;
			parent->right = sibling->right;


			// release siblings
			root->rwlock.unlock();
			sibling->rwlock.unlock();					
			free(root);
			free(sibling);
			// Unlock parent
			parent->rwlock.unlock();
			return;			
		} else {
			root->rwlock.unlock();
			parent->rwlock.unlock();
			return;
		}
		
	} else {
		if(key <= root->key) {
			// Lock the child
			root->left->rwlock.lock();
			if(parent != NULL)
				parent->rwlock.unlock();
			recursiveRemove(root, root->left, key);
		}
		else {
			// Lock the child
			root->right->rwlock.lock();
			if(parent != NULL)
				parent->rwlock.unlock();
			recursiveRemove(root, root->right, key);
		}

	}
}

void remove(tr root, int key) {

	if(root == NULL) {
		return;
	} else {
		root->rwlock.lock();
		recursiveRemove(NULL, root, key);
	}
}

