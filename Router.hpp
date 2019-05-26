#pragma once

#include <vector>

#include "Node.hpp"

using std::vector;

class Router
{
    private:
    vector<Node> *world;
    vector<uint8_t> route;

    public:
    enum MOVES { NORTH, SOUTH, EAST, WEST };
    Router(vector<Node> *world) : world(world) {};
    vector<uint8_t> *getRoute(Node *start, Node *end);
};

vector<uint8_t> *Router::getRoute(Node *start, Node *end)
{
    route.clear();

    route.push_back(Router::MOVES::SOUTH);
    route.push_back(Router::MOVES::EAST);
    route.push_back(Router::MOVES::SOUTH);
    route.push_back(Router::MOVES::WEST);

    return &route;
}