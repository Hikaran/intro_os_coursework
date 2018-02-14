/* TODO Fix header of this and README
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
#define MAX_CHILDREN 10
#define MAX_NAME_LENGTH 1024

/** Return if the given line is a comment (starts with a '#') */
int lineIsComment(char* line) {
    return line[0] == '#';
}

/**
 * Parse "Line 2" of the input file creating the nodes and initializing their values
 * Args:
 *   'line' - Line to parse
 *   'nodes' - Pointer to nodes to be allocated
 *   'candidates' - string of candidates to be passed used in node value initialization
 * Return:
 *   Number of nodes created
 * Initialized fields of nodes: id, name, output, candidates (copy of given string)
 * Defaulted fields of nodes: prog (to "leafcounter"), num_children (to 0), status (to 0),
 *   pid (to 0)
 */
int createNodes(char* line, node_t *nodes, char* candidates) {
    char*** argvp = (char***)malloc(MAX_NODES*MAX_NAME_LENGTH*sizeof(char));
    int num_nodes = makeargv(line, " ", argvp);
    for (int i = 0; i < num_nodes; i++) {
        nodes[i].id = i + 1;
        strcpy(nodes[i].name, (*argvp)[i]);
        strcpy(nodes[i].output, (*argvp)[i]);
        prepend(nodes[i].output, "Output_");
        strcpy(nodes[i].candidates, candidates);
        strcpy(nodes[i].prog, "leafcounter");
        nodes[i].num_children = 0;
        nodes[i].status = 0;
        nodes[i].pid = 0;
    }
    return num_nodes;
}

/* Parse "Line 3+" of the input file linking nodes together
 * Args:
 *   'line' - Line to parse
 *   'nodes' - Pointer to container of nodes
 * About linkNodes: linkNodes is supposed to...
 * 1) Identify parent node from line
 * 2) Update parent node prog to "aggregate_votes"
 * 3) Identify children of parent from line
 * 4) Update child count, child ids, and inputs of parent
 */
void linkNodes(char* line, node_t *nodes) {
    // Split parent name from rest of line.
    char*** link_info = (char***)malloc(MAX_NODES*MAX_NAME_LENGTH*sizeof(char));
    makeargv(line, ":", link_info);

    // Fetch parent node.
    node_t* parent = findnode(nodes, trimwhitespace((*link_info)[0]));

    // Change parent program since it is not a leaf node.
    strcpy(parent->prog, "aggregate_votes");

    // Split children and save quantity to parent.
    int num_children = makeargv(trimwhitespace((*link_info)[1]), " ", link_info);
    parent->num_children = num_children;

    // Copy child ids and input file names to parent node.
    for (int i = 0; i < num_children; i++) {
        node_t* child = findnode(nodes, trimwhitespace((*link_info)[i]));
        parent->children[i] = child->id;
        strcpy(parent->input[i], child->name);
        prepend(parent->input[i], "Output_");
    }    

    free(link_info);
}

/**Function : parseInput
 * Arguments: 'filename' - name of the input file
 * 			  'nodes' - Pointer to Nodes to be allocated by parsing
 * Output: Total number of allocated nodes
 * About parseInput: parseInput is supposed to...
 * 1) Open the Input File [There is a utility function provided in utility handbook]
 * 2) Read it line by line : Ignore the empty lines [There is a utility function provided in utility handbook]
 * 3) Exit program and print error message if any line is malformed
 * 4) Call appropriate function on each one of these lines
 */
int parseInput(char *filename, node_t *nodes) {
    FILE* f = file_open(filename);
    char* buf = (char*)malloc(MAX_NAME_LENGTH*sizeof(char));
    char* candidates = (char*)malloc(MAX_NAME_LENGTH*sizeof(char));
    int line_num = 0;
    int num_nodes_created = 0;
    while (buf = read_line(buf, f)) {
        buf = trimwhitespace(buf);
        if (lineIsComment(buf)) {  // TODO: Ignore empty lines
            continue;
        }
        line_num++;
        if (line_num <= 0) {
            printf("There was an error parsing the input file.\n");
            exit(1);
        } else if (line_num == 1) {
            strcpy(candidates, buf);
        } else if (line_num == 2) {
            num_nodes_created = createNodes(buf, nodes, candidates);
        } else {
            linkNodes(buf, nodes);
        }
    }
    free(buf);
    free(candidates);
    return num_nodes_created;
}

/**Function : execNodes
 * Arguments: 'n' - Pointer to Nodes to be allocated by parsing
 * About execNodes:
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
    node_t* root = findnode(mainnodes, "Who_Won");
    strcpy(root->prog, "find_winner");
    printgraph(mainnodes, num);
    execNodes(root);

    return 0;
}
