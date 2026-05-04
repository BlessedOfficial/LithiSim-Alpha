#include <iostream>
#include "mine_graph.h"

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    MineGraph mine_graph;
    mine_graph.add_edge(PIT, CRUSHER, 10);
    mine_graph.add_edge(CRUSHER, PARKING, 10);
    mine_graph.add_edge(PARKING, MAINTENANCE, 10);
    mine_graph.add_edge(MAINTENANCE, PIT, 10);
    mine_graph.print_graph();
    return 0;
}
