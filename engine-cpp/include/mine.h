#pragma once

#include <iostream>

enum NodeType {
    PIT,
    CRUSHER,
    PARKING,
    MAINTENANCE,
    NUM_NODES 
};

inline const char* node_type_to_string(NodeType node) {
    switch (node) {
        case PIT: return "PIT";
        case CRUSHER: return "CRUSHER";
        case PARKING: return "PARKING";
        case MAINTENANCE: return "MAINTENANCE";
        default: return "UNKNOWN";
    }
}


struct MineGraph {
    int num_nodes;
    bool adjacency_matrix[NUM_NODES][NUM_NODES];
    int weights[NUM_NODES][NUM_NODES];
    NodeType nodes[NUM_NODES];

    MineGraph() {
        num_nodes = NUM_NODES;
        for (int i = 0; i < NUM_NODES; i++) {
            nodes[i] = static_cast<NodeType>(i);
            for (int j = 0; j < NUM_NODES; j++) {
                adjacency_matrix[i][j] = false;
                weights[i][j] = 0;
            }
        }
    }
    void add_edge(NodeType from, NodeType to, int weight){
        int from_index = static_cast<int>(from);
        int to_index = static_cast<int>(to);
        adjacency_matrix[from_index][to_index] = true;
        weights[from_index][to_index] = weight;
    }
    void remove_edge(NodeType from, NodeType to){
        int from_index = static_cast<int>(from);
        int to_index = static_cast<int>(to);
        adjacency_matrix[from_index][to_index] = false;
        weights[from_index][to_index] = 0;
    }
    void set_weight(NodeType from, NodeType to, int weight){
        int from_index = static_cast<int>(from);
        int to_index = static_cast<int>(to);
        weights[from_index][to_index] = weight;
    }

    void set_node_type(NodeType node, int index){
        nodes[index] = node;
    }

    void print_graph(){
        for(int i = 0; i < num_nodes; i++){
            for(int j = 0; j < num_nodes; j++){
                if(adjacency_matrix[i][j]){
                    std::cout << node_type_to_string(nodes[i]) << " -> "
                              << node_type_to_string(nodes[j]) << " : "
                              << weights[i][j] << std::endl;
                }
            }
        }
    }
};