/*
 * VCforStudents.c
 *
 *  Created on: Feb 2, 2018
 *      Author: ayushi
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "makeargv.h"

#define MAX_NODES 100

//Function signatures

/**Function : parseInput
 * Arguments: 'filename' - name of the input file
 * 	      'n' - Pointer to Nodes to be allocated by parsing
 * Output: Number of Total Allocated Nodes
 * About parseInput: parseInput is supposed to
 * 1) Open the Input File [There is a utility function provided in utility handbook]
 * 2) Read it line by line : Ignore the empty lines [There is a utility function provided in utility handbook]
 * 3) Call parseInputLine(..) on each one of these lines
 ..After all lines are parsed(and the DAG created)
 4) Assign node->"prog" ie, the commands that each of the nodes has to execute
 For Leaf Nodes: ./leafcounter <arguments> is the command to be executed.
 Please refer to the utility handbook for more details.
 For Non-Leaf Nodes, that are not the root node(ie, the node which declares the winner):
 ./aggregate_votes <arguments> is the application to be executed. [Refer utility handbook]
 For the Node which declares the winner:
 This gets run only once, after all other nodes are done executing
 It uses: ./find_winner <arguments> [Refer utility handbook]
 */
int parseInput(char *filename, node_t *n) {

}

/**Function : parseInputLineCandidates
 * Arguments: 's' - Line to be parsed
 *            'n' - Pointer to node container
 * Output: ?
 * About parseInputLineCandidates: parseInputLineCandidates is supposed to
 * 1) Save the input line for later use.
 * */
int parseInputLineCandidates(char *s, node_t *n);

/**Function : parseInputLineNodes
 * Arguments: 's' - Line to be parsed
 *            'n' - Pointer to node container
 * Output: Number of Region Nodes allocated
 * About parseInputLineNodes: parseInputLineNodes is supposed to
 * 1) Split the input line.
 * 2) Create a new node from each name.
 * */
int parseInputLineNodes(char *s, node_t *n);

/**Function : parseInputLineStructure
 * Arguments: 's' - Line to be parsed
 *            'n' - Pointer to node container
 * Output: Number of Region Nodes allocated
 * About parseInputLineStructure: parseInputLineStructure is supposed to
 * 1) Split the parent from its children.
 * 2) Split the children.
 * 3) Link each child to parent.
 * */
int parseInputLineStructure(char *s, node_t *n);

/**Function : execNodes
 * Arguments: 'n' - Pointer to Nodes to be allocated by parsing
 * About execNodes: parseInputLine is supposed to
 * If the node passed has children, fork and execute them first
 * Please note that processes which are independent of each other
 * can and should be running in a parallel fashion
 * */
void execNodes(node_t *n);


int main(int argc, char **argv){

	//Allocate space for MAX_NODES to node pointer
	struct node* mainnodes=(struct node*)malloc(sizeof(struct node)*MAX_NODES);

	if (argc != 2){
		printf("Usage: %s Program\n", argv[0]);
		return -1;
	}

	//call parseInput
	int num = parseInput(argv[1], mainnodes);

	//Call execNodes on the root node


	return 0;
}
