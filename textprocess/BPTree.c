#include <stdio.h>
#include <stdlib.h>

#define order 5

typedef struct key_value_pair {
	char *record_ptr; //leaf node only
	int record_Id;
} KVPair;

typedef struct BPTreeNode{
	struct BPTreeNode *next_leaf; //leaf node only
	char **record_ptr; //leaf node only
	struct BPTreeNode **child_node; //internal node only
	int *record_Id;
	int valid; //the number of valid entries
	char is_leaf; //leaf:1, internal:0
	struct BPTreeNode *parent;
} BPTreeNode;

typedef struct key_child_pair {
	BPTreeNode *child_ptr; //internal node only
	int record_Id;
} KCPair;

BPTreeNode *root;

BPTreeNode *new_leaf_node() {
	BPTreeNode *new_node;
	int i;

	new_node = (BPTreeNode *)malloc(sizeof(BPTreeNode));
		if(!new_node) {
			printf("memory error: insert_node\n");
			exit(1);
		}
		new_node->child_node = (BPTreeNode **)malloc(sizeof(BPTreeNode *)*order);
		if(!new_node) {
			printf("memory error: insert_node_childe\n");
			exit(1);
		}
		new_node->record_Id = (int *)malloc(sizeof(int)*(order-1));
		if(!new_node) {
			printf("memory error: insert_node_record_Id\n");
			exit(1);
		}
		new_node->record_ptr = (char **)malloc(sizeof(char *)*(order-1));
		if(!new_node) {
			printf("memory error: insert_node_record_ptr\n");
			exit(1);
		}
		new_node->is_leaf = 1;
		new_node->valid = 0;
		new_node->next_leaf = NULL;
		new_node->parent = NULL;

		for(i=0; i<order-1; i++) {
			new_node->child_node[i] = NULL;
			new_node->record_Id[i] = -1;
			new_node->record_ptr[i] = NULL;
		}
		new_node->child_node[order] = NULL;

	return new_node;
}

BPTreeNode * find_leaf_contain_key(int AM_fd, int recId) {
	int i;
	BPTreeNode *find_node;

	find_node = root;

	if (find_node == NULL) {
		return find_node;
	}
	while (find_node->is_leaf == 0) {
		i = 0;
		while (i < find_node->valid) {
			if (recId >= find_node->record_Id[i]) {
				i++;
			} else {
				break;
			}
		}
		find_node = find_node->child_node[i];
	}

	return find_node;
}

void insert_in_leaf(BPTreeNode *leaf_node, int recId, char *value) {
	int i;
	int j;

	j = 0;
	while (j < leaf_node->valid && leaf_node->record_Id[j] < recId) {
		j++;
	}

	for (i = leaf_node->valid; i > j; i--) {
		leaf_node->record_Id[i] = leaf_node->record_Id[i - 1];
		leaf_node->record_ptr[i] = leaf_node->record_ptr[i - 1];
	}
	leaf_node->record_Id[j] = recId;
	leaf_node->record_ptr[j] = value;
	leaf_node->valid++;
}

void insert_in_parent(BPTreeNode *node1, int recId, BPTreeNode *node2) {
	BPTreeNode *parent_node;
	KCPair *temp_node;
	BPTreeNode *split_node;
	BPTreeNode *child_node;
	BPTreeNode *new_root;

	int i;
	int j;
	int k;
	int half_order_key;

	printf("@@@insert in parent@@@\n");

	if(node1 == root) {
		printf("---insert new root---\n");

		new_root = new_leaf_node();
		new_root->is_leaf = 0;
		new_root->child_node[0] = node1;
		new_root->child_node[1] = node2;
		new_root->parent = NULL;
		new_root->record_Id[0] = recId;
		new_root->valid = 1;
		root = new_root;
		node2->parent = new_root;
		node1->parent = new_root;

		return;
	}

	parent_node = node1->parent;

	if(parent_node->valid < order-1) {
		printf("---insert in parent wo splitting---\n");

		j = 0;
		while(j <= parent_node->valid && parent_node->child_node[j] != node1) {
			j++;
		}

		for (i = parent_node->valid; i>j; i--) {
			parent_node->child_node[i+1] = parent_node->child_node[i];
			parent_node->record_Id[i] = parent_node->record_Id[i-1];
		}
		parent_node->child_node[j+1] = node2;
		parent_node->record_Id[j] = recId;
		parent_node->valid++;

	} else {
		printf("---insert in parent w splitting---\n");
		split_node = new_leaf_node();
		split_node->is_leaf = 0;
		split_node->parent = parent_node->parent;

		temp_node = (KCPair *)malloc(sizeof(KCPair)*(order+1));

		//insert recId and node2 in temp_node just after node1
		k = 0;
		while(k <= parent_node->valid && parent_node->child_node[k] != node1) {
			k++;
		}

		for (i=0, j=0; i<parent_node->valid+1; i++, j++) {
			if(j == k+1) j++;
			temp_node[j].child_ptr = parent_node->child_node[i];
		}

		for (i=0, j=0; i<parent_node->valid; i++, j++) {
			if(j == k) j++;
			temp_node[j].record_Id = parent_node->record_Id[i];
		}

		temp_node[k+1].child_ptr = node2;
		temp_node[k].record_Id = recId;

		for(i=0; i<parent_node->valid; i++) {
			parent_node->child_node[i] = NULL;
			parent_node->record_Id[i] = -1;
		}
		parent_node->child_node[parent_node->valid] = NULL;
		parent_node->valid = 0;

		for(i=0; i<order/2; i++) {
			parent_node->child_node[i] = temp_node[i].child_ptr;
			parent_node->record_Id[i] = temp_node[i].record_Id;
			parent_node->valid++;
		}
		parent_node->child_node[i] = temp_node[i].child_ptr;

		half_order_key = temp_node[order/2].record_Id;

		for(++i, j=0; i<order; i++, j++) {
			split_node->child_node[j] = temp_node[i].child_ptr;
			split_node->record_Id[j] = temp_node[i].record_Id;
			split_node->valid++;
		}
		split_node->child_node[j] = temp_node[i].child_ptr;
		split_node->child_node[order-1] = temp_node[order-1].child_ptr;

		free(temp_node);

		for(i=0; i<=split_node->valid; i++) {
			child_node = split_node->child_node[i];
			child_node->parent = split_node;
		}

		insert_in_parent(parent_node, half_order_key, split_node);
	}
}

//all the functions have to have the AM_fd, tree indicator
void AM_InsertEntry(int AM_fd, int recId, char *value) {
	BPTreeNode *insert_node;
	BPTreeNode *split_node;
	KVPair *temp_node;

	int i;
	int j;
	int k;
	int smallest_key;
	printf("@@@AM_InsertEntry@@@\n");

	if(!root) {
		insert_node = new_leaf_node();
		root = insert_node;
	} else {
		printf("---go to find function---\n");
		insert_node = find_leaf_contain_key(AM_fd, recId);
	}

	if(insert_node->valid < order-1) {
		printf("@@@insert in leaf@@@\n");
		insert_in_leaf(insert_node, recId, value);
	} else {
		printf("---insert in leaf w splitting---\n");
		split_node = new_leaf_node();

		temp_node = (KVPair *)malloc(sizeof(KVPair)*order);

		k = 0;
		while (k < order - 1 && insert_node->record_Id[k] < recId)
			k++;

		for (i = 0, j = 0; i < insert_node->valid; i++, j++) {
			if (j == k) j++;
			temp_node[j].record_Id = insert_node->record_Id[i];
			temp_node[j].record_ptr = insert_node->record_ptr[i];
		}

		temp_node[k].record_Id = recId;
		temp_node[k].record_ptr = value;

		for(i=0; i<insert_node->valid; i++) {
			insert_node->child_node[i] = NULL;
			insert_node->record_ptr[i] = NULL;
			insert_node->record_Id[i] = -1;
		}
		insert_node->valid = 0;


		for(i=0; i<(order-1)/2; i++) {
			insert_node->record_ptr[i] = temp_node[i].record_ptr;
			insert_node->record_Id[i] = temp_node[i].record_Id;
			insert_node->valid++;
		}

		for (i = (order-1)/2, j = 0; i < order; i++, j++) {
			split_node->record_ptr[j] = temp_node[i].record_ptr;
			split_node->record_Id[j] = temp_node[i].record_Id;
			split_node->valid++;
		}

		free(temp_node);

		split_node->next_leaf = insert_node->next_leaf;
		insert_node->next_leaf = split_node;
		split_node->parent = insert_node->parent;


		smallest_key = split_node->record_Id[0];
		insert_in_parent(insert_node, smallest_key, split_node);
	}
}


//all the functions have to have the AM_fd, tree indicator
void AM_DeleteEntry(int AM_fd, int record_Id, char *record_ptr) {
	BPTreeNode *delete_node;
	delete_node = find_leaf_contain_key(AM_fd, record_Id);
	printf("@@@AM_DeleteEntry@@@\n");

	delete_entry(AM_fd, delete_node, record_Id, record_ptr);


}

//all the functions have to have the AM_fd, tree indicator
void delete_entry(int AM_fd, BPTreeNode *delete_node, int record_Id, void *record_ptr) {
	int i;
	int j;
	int k;
	int m;
	int few_values;
	int delete_node_valid;
	int sibling_node_valid;
	int inter_record_Id;
	BPTreeNode *sibling_node;
	BPTreeNode *parent_node;
	BPTreeNode *temp_node;

	//find the index that has record_Id
	k = 0;
	while(delete_node->record_Id[k] != record_Id) {
		k++;
	}

	//delete record_Id
	for (++k; k < delete_node->valid; k++) {
		delete_node->record_Id[k-1] = delete_node->record_Id[k];
	}

	//find the index and delete record_ptr or child_node from delete_node
	if(delete_node->is_leaf == 1) {
		k = 0;
		while(delete_node->record_ptr[k] != (char *)record_ptr) {
			k++;
		}
		for (++k; k < delete_node->valid; k++) {
			delete_node->record_ptr[k-1] = delete_node->record_ptr[k];
		}
	} else {
		k = 0;
		while(delete_node->child_node[k] != (BPTreeNode *)record_ptr) {
			k++;
		}
		for (++k; k < delete_node->valid+1; k++) {
			delete_node->child_node[k-1] = delete_node->child_node[k];
		}
	}

	delete_node->valid--;

	if(delete_node->is_leaf == 1) {
		for (i = delete_node->valid; i<order-1; i++) {
			delete_node->record_ptr[i] = NULL;
		}
	} else {
		for (i = delete_node->valid+1; i<order; i++) {
			delete_node->child_node[i] = NULL;
		}
	}

	printf("###delete_entry###\n");

	if(delete_node->is_leaf == 0) {
		few_values = order%2 == 0 ? (order/2)-1 : order/2;
	} else {
		few_values = (order-1)%2 == 0 ? (order-1)/2 : ((order-1)/2)+1;
	}

	if(delete_node == root) {//delete_node is root
		if(delete_node->valid <= 0) { //delete_node has only one remaining child
			if(delete_node->is_leaf == 0) {
				//make the child of delete_node the new root of the tree
				root = delete_node->child_node[0];
			} else {
				root = NULL;
			}
			free(delete_node);
		}
	} else if(delete_node->valid < few_values) {//if delete_node has too few values. For nonleaf nodes, this criterion means less than order/2 pointers; for leaf nodes, it means less than (order−1)/2 values.
		printf("---node has too few values---\n");
		parent_node = delete_node->parent;

		//find the index i that has delete_node
		k = 0;
		while(parent_node->child_node[k] != delete_node) {
			k++;
		}

		//Let sibling_node be the previous or next child of parent(delete_node)& Let inter_record_Id be the value between pointers sibling_node and delete_node in parent(delete_node)
		if(k == 0) {
			//next: delete_node --> sibling_node
			sibling_node = parent_node->child_node[1];
			inter_record_Id = parent_node->record_Id[0];

		} else {
			//previous: sibling_node --> delete_node
			sibling_node = parent_node->child_node[k-1];
			inter_record_Id = parent_node->record_Id[k-1];
		}

		if(delete_node->is_leaf == 0) {
			few_values = order-1;
		} else {
			few_values = order;
		}

		if((sibling_node->valid + delete_node->valid) < few_values) {
			printf("---coalesce nodes---\n");

			//coalesce nodes
			if(k == 0) {//if delete_node is a predecessor of sibling_node
				//swap variables(N,N')
				temp_node = delete_node;
				delete_node = sibling_node;
				sibling_node = temp_node;
			}

			sibling_node_valid = sibling_node->valid;

			if(delete_node->is_leaf == 0) {//delete_node is not a leaf
				//append inter_record_Id

				sibling_node->record_Id[sibling_node_valid] = inter_record_Id;
				sibling_node->valid++;
				delete_node_valid = delete_node->valid;
				//append all pointer and values in delete_node to sibling_node
				for(i=sibling_node_valid+1, j=0; j<delete_node_valid; i++, j++) {
					sibling_node->record_Id[i] = delete_node->record_Id[j];
					sibling_node->child_node[i] = delete_node->child_node[j];
					sibling_node->valid++;
					delete_node->valid--;
				}
				sibling_node->child_node[i] = delete_node->child_node[j];

				for (i = 0; i<sibling_node->valid+1; i++) {
					temp_node = sibling_node->child_node[i];
					temp_node->parent = sibling_node;
				}
			} else { //delete_node is a leaf

				//append all key value pair in delete_node to sibling_node
				for(i=sibling_node_valid, j=0; j<delete_node->valid; i++, j++) {
					sibling_node->record_Id[i] = delete_node->record_Id[j];
					sibling_node->record_ptr[i] = delete_node->record_ptr[j];
					sibling_node->valid++;
				}

				//set sibling_node next_leaf to delete_node next_leaf
				sibling_node->next_leaf = delete_node->next_leaf;
			}
			printf("---coalesce nodes end---\n");

			delete_entry(AM_fd, parent_node, inter_record_Id, delete_node);

			free(delete_node);

		} else {
			printf("---redistribution---\n");
			//redistribution
			if(k != 0) {//if sibling_node is a predecessor of delete_node

				if(delete_node->is_leaf == 0) {
					//let m be such that sibling_node.Pm is the last pointer in sibling_node
					m = sibling_node->valid;

					//insert (N'.Pm, K') as the first pointer and value in N by shifting other pointers and values right
					delete_node->child_node[delete_node->valid + 1] = delete_node->child_node[delete_node->valid];
					for (i=delete_node->valid; i>0; i--) {
						delete_node->record_Id[i] = delete_node->record_Id[i-1];
						delete_node->child_node[i] = delete_node->child_node[i-1];
					}

					delete_node->child_node[0] = sibling_node->child_node[m];
					delete_node->record_Id[0] = inter_record_Id;

					temp_node = delete_node->child_node[0];
					temp_node->parent = delete_node;
					sibling_node->child_node[m] = NULL;

					//replace K' in parent(N) by N'.Km−1
					parent_node->record_Id[k-1] = sibling_node->record_Id[m-1];

					//remove (N'.Km−1, N'.Pm) from N'
					sibling_node->record_Id[m-1] = -1;
					sibling_node->child_node[m] = NULL;

				} else { //delete_node is leaf
					//let m be such that (sibling_node.Pm, sibling_node.Km) is the last pointer/value pair in sibling_node
					m = sibling_node->valid-1;

					//insert (N'.Pm, N'.Km) as the first pointer and value in N, by shifting other pointers and values right
					for (i=delete_node->valid; i>0; i--) {
						delete_node->record_Id[i] = delete_node->record_Id[i-1];
						delete_node->record_ptr[i] = delete_node->record_ptr[i-1];
					}
					delete_node->record_Id[0] = sibling_node->record_Id[m];
					delete_node->record_ptr[0] = sibling_node->record_ptr[m];

					//replace K' in parent(N) by N'.Km
					parent_node->record_Id[k-1] = sibling_node->record_Id[m];

					//remove (N'.Km, N'.Pm) from N'
					sibling_node->record_ptr[m] = NULL;
					sibling_node->record_Id[m] = -1;
				}
			} else {
				//symmetric, delete_node is a predecessor of sibling_node
				if (delete_node->is_leaf) {
					delete_node->record_Id[delete_node->valid] = sibling_node->record_Id[0];
					delete_node->record_ptr[delete_node->valid] = sibling_node->record_ptr[0];
					delete_node->parent->record_Id[k+1] = sibling_node->record_Id[1];

					for (i = 0; i < sibling_node->valid - 1; i++) {
						sibling_node->record_Id[i] = sibling_node->record_Id[i + 1];
						sibling_node->record_ptr[i] = sibling_node->record_ptr[i + 1];
					}
				} else {
					delete_node->record_Id[delete_node->valid] = inter_record_Id;
					delete_node->child_node[delete_node->valid + 1] = sibling_node->child_node[0];
					temp_node = delete_node->child_node[delete_node->valid + 1];
					temp_node->parent = delete_node;
					delete_node->parent->record_Id[k+1] = sibling_node->record_Id[0];

					for (i = 0; i < sibling_node->valid - 1; i++) {
						sibling_node->record_Id[i] = sibling_node->record_Id[i + 1];
						sibling_node->child_node[i] = sibling_node->child_node[i + 1];
					}
				}
				if (!delete_node->is_leaf)
					sibling_node->child_node[i] = sibling_node->child_node[i + 1];

			}

			delete_node->valid++;
			sibling_node->valid--;
			printf("---redistribution end---\n");

		}
	}
}

BPTreeNode *queue = NULL;

void enqueue(BPTreeNode *new_node) {
	BPTreeNode *c;

	if (queue == NULL) {
		queue = new_node;
		queue->next_leaf = NULL;
	} else {
		c = queue;

		while(c->next_leaf != NULL) {
			c = c->next_leaf;
		}

		c->next_leaf = new_node;
		new_node->next_leaf = NULL;
	}
}

BPTreeNode *dequeue() {
	BPTreeNode *n = queue;

	queue = queue->next_leaf;
	n->next_leaf = NULL;

	return n;
}

int path_to_root(BPTreeNode *root, BPTreeNode *child ) {
	int length = 0;
	BPTreeNode *c = child;

	while (c != root) {
		c = c->parent;
		length++;
	}

	return length;
}

void print_tree(BPTreeNode *root) {
	BPTreeNode * n = NULL;
	int i = 0;
	int rank = 0;
	int new_rank = 0;

	if (root == NULL) {
		printf("Empty tree\n");
		return;
	}

	queue = NULL;
	enqueue(root);

	while(queue != NULL) {
		n = dequeue();
		if(n->parent!=NULL && n==n->parent->child_node[0]) {
			new_rank = path_to_root(root, n);
			if (new_rank != rank) {
				rank = new_rank;
				printf("\n");
			}
		}

		for(i=0; i<n->valid; i++) {
			printf("%d ", n->record_Id[i]);
		}

		if(!n->is_leaf) {
			for(i = 0; i <= n->valid; i++) {
				enqueue(n->child_node[i]);
			}
		}
		printf("| ");
	}
	printf("\n");
}

int main(void) {
	printf("\n####BPTree_Start####\n");
	if(!root) {
		printf("root is null\n");
	}

	printf("\n####BPTree_Insert_Start####\n");
	AM_InsertEntry(1, 10, NULL);
	print_tree(root);

	AM_InsertEntry(1, 20, NULL);
	print_tree(root);

	AM_InsertEntry(1, 30, NULL);
	print_tree(root);

	AM_InsertEntry(1, 8, NULL);
	print_tree(root);

	AM_InsertEntry(1, 3, NULL);
	print_tree(root);

	AM_InsertEntry(1, 4, NULL);
	print_tree(root);

	AM_InsertEntry(1, 9, NULL);
	print_tree(root);

	AM_InsertEntry(1, 2, NULL);
	print_tree(root);

	AM_InsertEntry(1, 15, NULL);
	print_tree(root);

	AM_InsertEntry(1, 11, NULL);
	print_tree(root);

	AM_InsertEntry(1, 25, NULL);
	print_tree(root);

	AM_InsertEntry(1, 21, NULL);
	print_tree(root);

	AM_InsertEntry(1, 35, NULL);
	print_tree(root);

	AM_InsertEntry(1, 31, NULL);
	print_tree(root);

	printf("\n####BPTree_Delete_Start####\n");
	AM_DeleteEntry(1, 10, NULL);
	print_tree(root);

	AM_DeleteEntry(1, 20, NULL);
	print_tree(root);

	AM_DeleteEntry(1, 30, NULL);
	print_tree(root);

	AM_DeleteEntry(1, 8, NULL);
	print_tree(root);

	AM_DeleteEntry(1, 3, NULL);
	print_tree(root);

	AM_DeleteEntry(1, 4, NULL);
	print_tree(root);

	AM_DeleteEntry(1, 9, NULL);
	print_tree(root);

	AM_DeleteEntry(1, 2, NULL);
	print_tree(root);

	AM_DeleteEntry(1, 15, NULL);
	print_tree(root);

	AM_DeleteEntry(1, 11, NULL);
	print_tree(root);

	AM_DeleteEntry(1, 25, NULL);
	print_tree(root);

	AM_DeleteEntry(1, 21, NULL);
	print_tree(root);

	AM_DeleteEntry(1, 35, NULL);
	print_tree(root);

	AM_DeleteEntry(1, 31, NULL);
	print_tree(root);

}






