#include <stdlib.h>

typedef struct node {
	int key;
	int value;
	int weight;
	node *left;
	node *right;
} * tr;

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

	if(parent == NULL)
		return;

	if(parent->left == v)
		parent->left = u;
	else
		parent->right = u;
}


void rebalance(tr node, tr parent) {

	int case2_leftright;

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
	} else if((node->left->weight == 0 && node->left->right != NULL && node->left->right->weight == 0) ||
		(node->right->weight == 0 && node->right->left != NULL && node->right->left->weight == 0)) {
		
		if(node->left->weight == 0) {

			tr u = node->left;
			tr A = u->left;
			tr T = u->right;
			tr B = T->left;
			tr C = T->right;
			tr D = node->right;


			if(node == parent->left) parent->left = T;
			else parent->right = T;

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


			if(node == parent->left) parent->left = T;
			else parent->right = T;

			T->right = u;
			T->left = node;
			u->right = A;
			u->left = B;
			node->right = C;
			node->left = D;
		}

	} else if(checkCase4(node)) {
		balanceCase4(node, parent);
	} else if(1) {
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

