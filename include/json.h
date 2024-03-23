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

Node* get(ObjectNode* node, const char* name);
ObjectNode* get_object(ObjectNode* node, const char* name);
ArrayNode* get_array(ObjectNode* node, const char* name);
StringNode* get_string(ObjectNode* node, const char* name);
NumberNode* get_number(ObjectNode* node, const char* name);
BoolNode* get_bool(ObjectNode* node, const char* name);

Node* at(ObjectNode* node, u32 index);
ObjectNode* at_object(ObjectNode* node, u32 index);
ArrayNode* at_array(ObjectNode* node, u32 index);
StringNode* at_string(ObjectNode* node, u32 index);
NumberNode* at_number(ObjectNode* node, u32 index);
BoolNode* at_bool(ObjectNode* node, u32 index);

u32 list_object(ContainerNode* node, ObjectNode** arr, u32 max_nodes);
u32 list_number(ContainerNode* node, NumberNode** arr, u32 max_nodes);
