#include <stdlib.h>
#include <algorithm>    // std::swap


typedef struct node {
	int key;
	int value;
	int weight;
	node *left;
	node *right;
} * tr;

tr GLOBAL_ROOT;

void rebalance(tr node, tr parent) {
	return;
}

void init() {
	GLOBAL_ROOT = new struct node;
	GLOBAL_ROOT->key = 0x7FFFFFFF;
	GLOBAL_ROOT->weight = 1;
	GLOBAL_ROOT->left = new struct node;
	GLOBAL_ROOT->right = new struct node;
	GLOBAL_ROOT->left->key = 0x7FFFFFFE;
	GLOBAL_ROOT->right->key = 0x7FFFFFFF;
	GLOBAL_ROOT->left->left = NULL;
	GLOBAL_ROOT->left->right = NULL;
	GLOBAL_ROOT->right->left = NULL;
	GLOBAL_ROOT->right->right = NULL;
	GLOBAL_ROOT->left->weight = 1;
	GLOBAL_ROOT->right->weight = 1;

}





// NOTE - the input to this can NOT be null!!
bool isLeaf(tr node) {
	return node->left == NULL && node->right == NULL;
}

tr search(tr root, int key) {
	tr ptr = root;

	while(true) {
		if(isLeaf(ptr)) {
			if(ptr->key == key)
				return ptr;
			else
				return NULL;
		}

		// else
		if(key < ptr->key)
			ptr = ptr->left;
		else
			ptr = ptr->right;
	}
}

void recursiveInsert(tr root, int key) {

	if(root->left == NULL && root->right == NULL) {
		// Leaf Node Code
		if(root->key == key) return /*root*/;

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
		
		leafLeft->key = root->key < key ? root->key : key;
		leafRight->key = root->key != leafLeft->key ? root->key : key;
		root->key = root->right->key;

		root->weight -= 1;

		return /*root*/;

	} else {
		if(root->key > key) /*root->left = */recursiveInsert(root->left, key);
		else /*root->right = */recursiveInsert(root->right, key);
		return /*root*/;
	}
}

void insert(tr root, int key) {

	if(root == NULL) {
		GLOBAL_ROOT = new struct node;
		GLOBAL_ROOT->key = key;
		GLOBAL_ROOT->weight = 0;
		GLOBAL_ROOT->left = NULL;
		GLOBAL_ROOT->right = NULL;
	} else {
		recursiveInsert(root, key);
	}
}



tr recursiveRemove(tr root, int key) {

	if(root->left == NULL && root->right == NULL) {
		// Leaf Node Code
		if(root->key == key) {
			free(root);
			return NULL;
		} else {
			return root;
		}
		
	} else {
		if(root->key > key) root->left = recursiveRemove(root->left, key);
		else root->right = recursiveRemove(root->right, key);

		tr return_ptr = root;
		if(root->left == NULL && root->right != NULL) {		
			root->right->weight += root->weight;
			return_ptr = root->right;
		} else if(root->right == NULL && root->left != NULL) {			
			root->left->weight += root->weight;		
			return_ptr = root->left;
		}

		if(return_ptr != root)
			free(root);

		return return_ptr;
	}
}

void remove(tr root, int key) {

	if(root == NULL) {
		return;
	} else {
		recursiveRemove(root, key);
	}
}

