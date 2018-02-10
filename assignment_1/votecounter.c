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


/** Return if the given line is a comment (starts with a '#') */
int line_is_comment(char* line) {
    return line[0] == '#';  // TODO: Trim whitespace
}

/**
 * Parse "Line 2" of the input file creating the nodes and initalizing their values
 * Args:
 *   'line' - Line to parse
 *   'nodes' - Pointer to nodes to be allocated
 *   'candidates' - string of candidates to be passed used in node value initalization
 * */
void parseInputCreateNodes(char* line, node_t *nodes, char* candidates) {
    char** argvp = (char**)malloc(MAX_NODES*1024*sizeof(char));
    int num_tokens = makeargv(line, " ", &argvp);
    printf("num_tokens: %d\n", num_tokens);
}

/**Function : parseInput
 * Arguments: 'filename' - name of the input file
 * 			  'nodes' - Pointer to Nodes to be allocated by parsing
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
int parseInput(char *filename, node_t *nodes) {
    FILE* f = file_open(filename);
    char* buf = (char*)malloc(1024*sizeof(char));
    char* candidates = (char*)malloc(1024*sizeof(char));
    int line_num = 0;
    while (buf = read_line(buf, f)) {
        if (line_is_comment(buf)) {  // TODO: Ignore empty lines
            continue;
        }
        line_num++;
        if (line_num <= 0) {
            printf("There was an error parsing the input file.");
            exit(1);
        } else if (line_num == 1) {
            strcpy(candidates, buf);
        } else if (line_num == 2) {
            parseInputCreateNodes(buf, nodes, candidates);
        } else {
        }

        printf("line %d: %s", line_num, buf);
    }
    printf("candidates: %s", candidates);
    free(buf);
    free(candidates);
}

/**Function : parseInputLine
 * Arguments: 's' - Line to be parsed
 * 			  'n' - Pointer to Nodes to be allocated by parsing
 * Output: Number of Region Nodes allocated
 * About parseInputLine: parseInputLine is supposed to
 * 1) Split the Input file [Hint: Use makeargv(..)]
 * 2) Recognize the line containing information of
 * candidates(You can assume this will always be the first line containing data).
 * You may want to store the candidate's information
 * 3) Recognize the line containing "All Nodes"
 * (You can assume this will always be the second line containing data)
 * 4) All the other lines containing data, will show how to connect the nodes together
 * You can choose to do this by having a pointer to other nodes, or in a list etc-
 * */
int parseInputLine(char *s, node_t *n) {
    return 0;
}

/**Function : execNodes
 * Arguments: 'n' - Pointer to Nodes to be allocated by parsing
 * About execNodes: parseInputLine is supposed to
 * If the node passed has children, fork and execute them first
 * Please note that processes which are independent of each other
 * can and should be running in a parallel fashion
 * */
void execNodes(node_t *n) {
}


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
