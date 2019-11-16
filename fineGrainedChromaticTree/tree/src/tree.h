#include <stdlib.h>

typedef struct node {
	int key;
	int value;
	int weight;
	node *left;
	node *right;
} * tr;


void rebalance(tr node, tr parent) {

	if() {
		if(parent == NULL) {
			// We are at root
		} else {

		}
	} else if() {
		if(parent == NULL) {
			// We are at root
		} else {

		}
	} else if() {
		if(parent == NULL) {
			// We are at root
		} else {

		}
	} else if() {
		if(parent == NULL) {
			// We are at root
		} else {

		}
	} else if() {
		if(parent == NULL) {
			// We are at root
		} else {

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
	tr ptr = root;

	while(true) {
		if(isLeaf(ptr)) {
			if(ptr->key == key)
				return ptr;
			else
				return NULL;
		}

		// else
		if(key <= ptr->key)
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

		root->left = leafLeft;
		root->right = leafRight;
		
		leafLeft->key = root->key < key ? root->key : key;
		leafRight->key = root->key >= key ? root->key : key;

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
		root = new struct node;
		root->key = key;
		root->weight = 0;
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

