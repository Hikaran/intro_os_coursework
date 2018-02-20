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
#include <sys/types.h>
#include <sys/wait.h>
#include "makeargv.h"

#define MAX_NODES 100
#define MAX_CHILDREN 10
#define MAX_NAME_LENGTH 1024
#define MAX_INPUT_LENGTH 100
#define MAX_PROGRAM_PATH_LENGTH 20

/** Return if the given line is a comment (starts with a '#') */
int lineIsComment(char* line) {
    return line[0] == '#';
}

/**
 * Parse "Line 2" of the input file creating the nodes and initializing their values
 * Args:
 *   'line' - Line to parse
 *   'nodes' - Pointer to nodes to be allocated
 *   'candidates' - array of candidates arguments to be passed used in node value initialization
 * Return:
 *   Number of nodes created
 * Initialized fields of nodes: id, name, output, candidates (copy of given string)
 * Defaulted fields of nodes: prog (to "leafcounter"), num_children (to 0), status (to 0),
 *   pid (to 0)
 */
int createNodes(char* line, node_t *nodes, char*** candidates, int num_candidate_words) {
    char*** argvp = (char***)malloc(MAX_NODES*MAX_NAME_LENGTH*sizeof(char));
    int num_nodes = makeargv(line, " ", argvp);
    int j;
    for (int i = 0; i < num_nodes; i++) {
        nodes[i].id = i + 1;
        strcpy(nodes[i].name, (*argvp)[i]);
        strcpy(nodes[i].output, (*argvp)[i]);
        prepend(nodes[i].output, "Output_");
        for (j = 0; j < num_candidate_words; j++) {
            strcpy(nodes[i].candidates[j], (*candidates)[j]);
        }
        strcpy(nodes[i].prog, "leafcounter");
        nodes[i].num_children = 0;
        nodes[i].status = 0;
        nodes[i].pid = 0;

        for (j = 0; j < num_candidate_words; j++) {
            printf("%s has candidate string |%s|\n", nodes[i].name, nodes[i].candidates[j]);
        }
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
    char*** candidates = (char***)malloc(MAX_NODES*MAX_NAME_LENGTH*sizeof(char));
    int num_candidate_words, num_candidates_given, num_candidates_found;

    int line_num = 0;
    int num_nodes_created = 0;
    while (read_line(buf, f)) {
        trimwhitespace(buf);
        // Ignore comments.
        if (lineIsComment(buf)) {  // TODO: Ignore empty lines
            continue;
        }
        line_num++;
        if (line_num <= 0) {
            printf("There was an error parsing the input file.\n");
            exit(1);
        } else if (line_num == 1) {
            // Split the candidate string.
            num_candidate_words = makeargv(buf, " ", candidates);
            num_candidates_found = num_candidate_words - 1;

            // Check the candidates for correct formatting.
            num_candidates_given = atoi((*candidates)[0]);
            printf("Number of candidates given: %d\n", num_candidates_given);
            printf("Number of candidates found: %d\n", num_candidates_found);
            if (num_candidates_given < 1) {
                // Consider any non-numeric string as an invalid number as well.
                printf("Invalid number of candidates given.\n");
                exit(1);
            } else if (num_candidates_given != num_candidates_found) {
                printf("Number of candidates given does not match number of candidate names.\n");
                exit(1);
            }
        } else if (line_num == 2) {
            num_nodes_created = createNodes(buf, nodes, candidates, num_candidate_words);
        } else {
            linkNodes(buf, nodes);
        }
    }
    free(buf);
    free(candidates);
    return num_nodes_created;
}

void callExec(node_t* node) {
    // Retrieve number of children.
    char str_num_children[MAX_CHILDREN];
    sprintf(str_num_children, "%d", node->num_children);

    // Retrieve number of candidate words.
    int num_candidate_words = atoi(node->candidates[0]) + 1;
    printf("Number of candidate words: %d in %s\n", num_candidate_words, node->name);

    int i, j = 0;
    char* input_words[MAX_INPUT_LENGTH];

    // Retrieve program name.
    input_words[i] = node->prog;
    i++;

    // Retrieve input file names.
    if (node->num_children > 0) {
        input_words[i] = str_num_children;
        i++;
        for (j = 0; j < num_candidate_words; j++) {
            input_words[i] = (node->input)[j];
            i++;
        }
    } else {
        input_words[i] = node->name;
        i++;
    }

    // Retrieve output file name.
    input_words[i] = node->output;
    i++;

    printf("Pre-candidates in %s\n", node->name);

    // Retrieve candidate number and names.
    // Add 1 to account for candidate number in candidate info array.
    for (j = 0 ; j < num_candidate_words; j++) {
        input_words[i] = node->candidates[j];
        i++;
    }

    printf("Post-candidates in %s\n", node->name);

    // NULL terminator
    input_words[i] = NULL;
    i++;

    if (i > MAX_INPUT_LENGTH) {
        printf("Exceeded MAX_INPUT_LENGTH.");
        exit(1);
    }

    // Construct relative file path for program and execute.
    char file_path[MAX_PROGRAM_PATH_LENGTH];
    sprintf(file_path, "./%s", node->prog);
    execvp(file_path, input_words);
}

/**Function : execNodes
 * Arguments: 'n' - Pointer to Nodes to be allocated by parsing
 * About execNodes:
 * If the node passed has children, fork and execute them first
 * Please note that processes which are independent of each other
 * can and should be running in a parallel fashion
 * */
void execNodes(node_t* allnodes, node_t* node) {
    int num_children = node->num_children;
    int i = 0;

    // Iteratively fork all children for each parent.
    while (i < num_children) {
        node->pid = fork();
        if (node->pid == 0) {  // Child branch
            // Find child node and treat as a new parent node.
            node = findNodeByID(allnodes, node->children[i]);
            if (node == NULL) {
                printf("Failed to find child node.\n");
                exit(1);
            }
            num_children = node->num_children;
            i = 0;
        } else if (node->pid > 0) {  // Parent branch
            // Move to next child.
            i++;
        } else {
            perror("Fork failed\n");
            exit(1);
        }
    }

    // Make parent wait for all children.
    if (num_children > 0) {
        while (wait(&(node->status)) > 0) {
            //printf("Parent %s waited on a child.\n", node->name);
        }
    }

    // All children, if any, have finished. Execute program!
    callExec(node);
}

int main(int argc, char **argv){
    // Allocate space for MAX_NODES to node pointer
    struct node* mainnodes=(struct node*)malloc(sizeof(struct node)*MAX_NODES);

    if (argc != 2){
        printf("Usage: %s Program\n", argv[0]);
        exit(1);
    }

    //call parseInput
    int num = parseInput(argv[1], mainnodes);

    // Check if there is a cycle in the graph.

    // Call execNodes on the root node and set prog of root node
    node_t* root = findnode(mainnodes, "Who_Won");
    if (root == NULL) {
        printf("Failed to find root node.\n");
        exit(1);
    }
    strcpy(root->prog, "find_winner");
    //printgraph(mainnodes, num);
    execNodes(mainnodes, root);

    return 0;
}
