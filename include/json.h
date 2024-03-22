#pragma once

#include "include/types.h"
#include "include/arena.h"

struct Node
{
    u32 type;
    u32 size;
    Str name;
};

struct StringNode
{
    Node info;
    Str value;
};

struct NumberNode
{
    Node info;
    float value;
};

struct BoolNode
{
    Node info;
    bool value;
};

struct ContainerNode
{
    Node info;
    u32 child_count;
    Node* child;
};

typedef ContainerNode ObjectNode;
typedef ContainerNode ArrayNode;

ObjectNode* parse_file(const char* file, Arena* arena);

Node* find_child(ObjectNode* node, const char* name);
ObjectNode* find_child_object(ObjectNode* node, const char* name);
ArrayNode* find_child_array(ObjectNode* node, const char* name);
StringNode* find_child_string(ObjectNode* node, const char* name);
BoolNode* find_child_bool(ObjectNode* node, const char* name);
