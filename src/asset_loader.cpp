#include "include/asset_loader.h"
#include "include/types.h"
#include "include/arena.h"
#include "include/util.h"

#include <assert.h>
#include <stdlib.h>


enum TokenType
{
    Token_BrackOpen,
    Token_BrackClose,
    Token_ArrayOpen,
    Token_ArrayClose,
    Token_Colon,
    Token_Comma,

    Token_String,
    Token_Number,
    Token_Bool,

    Token_EOF,
    Token_Error,
};

enum NodeType
{
    Node_Object,
    Node_Array,
    Node_Number,
    Node_String,
    Node_Bool,
};

struct SimpleToken
{
    u32 type;
};

struct StringToken
{
    u32 type;
    Str value;
};

struct NumberToken
{
    u32 type;
    float value;
};

struct BoolToken
{
    u32 type;
    bool value;
};

struct SimpleArena
{
    u8* ptr; 
    u32 current; 
    u32 cap;
};

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

struct ObjectNode
{
    Node info;
    u32 child_count;
    Node* child;
};

u8* alloc(SimpleArena* buffer, u32 size) 
{
    u32 curr = buffer->current;
    assert(buffer->current + size < buffer->cap);
    buffer->current += size;

    return buffer->ptr + curr;
}

bool is_number(char c) 
{
    return (c >= '0' && c <= '9') || c == '-' || c == '+';
}

SimpleArena json_lexer(char* content, Arena* arena)
{
    SimpleArena buffer = {};
    buffer.cap = 10000;
    buffer.ptr = (u8*) push_size(arena, buffer.cap);

    char* curr = content;
    while (*curr) {
        if (*curr == ' ' || *curr == '\n' || *curr == '\t' || *curr == '\r') {
            curr++;
            continue;
        }

        if (*curr == '"') {
            StringToken* token = (StringToken*) alloc(&buffer, sizeof(StringToken));

            curr++;
            token->type = Token_String;
            token->value.ptr = curr;
            token->value.len = 0;

            // TODO: Handle escape seqence
            while (*curr && *curr != '"') {
                curr++;
                token->value.len++;
            }

            curr++;
            continue;
        }

        if (*curr == 't' || *curr == 'T' || *curr == 'f' || *curr == 'F') {
            BoolToken* token = (BoolToken*) alloc(&buffer, sizeof(BoolToken));
            token->type = Token_Bool;
            token->value = *curr == 't' || *curr == 'T';
            // NOTE: "true" has 4 letters, "false"  has 5 letters
            if (token->value) {
                curr += 4;
            } else {
                curr += 5;
            }
            continue;
        }

        if (is_number(*curr)) {
            NumberToken* token = (NumberToken*) alloc(&buffer, sizeof(NumberToken));
            token->type = Token_Number;
            token->value = atof(curr);
            while (is_number(*curr) || *curr == '.' || *curr == 'e') {
                curr++;
            }
            continue;
        }

        SimpleToken* token = (SimpleToken*) alloc(&buffer, sizeof(SimpleToken));

        if (*curr == '{') {
            token->type = Token_BrackOpen;
        } else if (*curr == '}') {
            token->type = Token_BrackClose;
        } else if (*curr == ':') {
            token->type = Token_Colon;
        } else if (*curr == '[') {
            token->type = Token_ArrayOpen;
        } else if (*curr == ']') {
            token->type = Token_ArrayClose;
        } else if (*curr == ',') {
            token->type = Token_Comma;
        } else if (*curr == ':') {
            token->type = Token_Colon;
        } else {
            token->type = Token_Error;
        }

        curr++;
    }

    SimpleToken* eof_token = (SimpleToken*) alloc(&buffer, sizeof(SimpleToken));
    eof_token->type = Token_EOF;

    return buffer;
}

inline void advance(SimpleToken** curr, u32 size) 
{
    u8* ptr = (u8*) *curr;
    *curr = (SimpleToken*) (ptr + size);
}

inline void expect(SimpleToken** curr, u32 type, u32 size) 
{
    assert((*curr)->type == type);
    advance(curr, size);
}

void parse_block(SimpleToken** curr, SimpleArena* nodes, ObjectNode* node);

Node* parse_field(SimpleToken** curr, SimpleArena* nodes)
{
    StringToken* str = (StringToken*) *curr;

    // Node* node = nodes + *node_count;
    // TODO: Fix this function

    *node = {};
    node->name = str->value;
    advance(curr, sizeof(StringToken));

    expect(curr, Token_Colon, sizeof(SimpleToken));

    switch ((*curr)->type) {
        case Token_Number: {
            NumberToken* num = (NumberToken*) *curr;
            node->type = Node_Number;
            node->number = num->value;
            node->size = 1;
            advance(curr, sizeof(NumberToken));
        } break;

        case Token_String: {
            StringToken* str = (StringToken*) *curr;
            node->type = Node_String;
            node->string = str->value;
            node->size = 1;
            advance(curr, sizeof(StringToken));
        } break;

        case Token_Bool: {
            BoolToken* boolean = (BoolToken*) *curr;
            node->type = Node_Bool;
            node->boolean = boolean->value;
            node->size = 1;
            advance(curr, sizeof(BoolToken));
        } break;

        case Token_BrackOpen: {
            parse_block(curr, nodes, node);
        } break;

        default: {
            assert(false || "Arrays not implemented");
        }
    }

    return node;
}

void parse_block(SimpleToken** curr, SimpleArena* nodes, ObjectNode* node)
{
    expect(curr, Token_BrackOpen, sizeof(SimpleToken));

    node->size = sizeof(ObjectNode);
    node->type = Node_Object;
    node->child_count = 0;

    bool comma = true;
    while ((*curr)->type == Token_String && comma) {
        Node* child = parse_field(curr, nodes, node_count);
        node->child_count++;
        node->size += child->size;

        if (!node->child) {
            node->child = child;
        }

        if ((*curr)->type == Token_Comma) {
            (*curr)++;
        } else {
            comma = false;
        }
    }

    expect(curr, Token_BrackClose, sizeof(SimpleToken));
}

void print_node(Node* node, Node* nodes)
{
    if (node->name.ptr) {
        printf("Got name: %.*s\n", node->name.len, node->name.ptr);
    }

    switch (node->type) {
        case Node_String: {
            printf("Value string: %.*s\n", node->string.len, node->string.ptr);
        } break;

        case Node_Number: {
            printf("Value number: %f\n", node->number);
        } break;

        case Node_Object: {
            printf("Object: child_count %u {\n", node->container.child_count);
            u32 child_ptr = node->container.offset;
            for (u32 i = 0; i < node->container.child_count; ++i) {
                print_node(nodes + child_ptr, nodes);
                child_ptr += nodes[child_ptr].size;
            }
            printf("}\n");
        } break;
    }
}

void load_model(const char* file, Arena* arena)
{
    char* content = read_file(file, NULL, arena);
    assert(content);

    SimpleArena tokens = json_lexer(content, arena);
    SimpleArena nodes = {};
    nodes.cap = 10000;
    nodes.ptr = (u8*) push_size(arena, nodes.cap);
    Node* nodes = (Node*) push_size(arena, sizeof(Node) * tokens.token_count);

    SimpleToken* curr = (SimpleToken*) tokens.ptr;
    ObjectNode* root = (ObjectNode*) alloc(&nodes, *root);
    *root = {};

    parse_block(&curr, nodes, &node_count, root);

    // ArrayNode* scene_nodes = (ArrayNode*) find_child(doc, "nodes", Node_Array);
    // scene_nodes->child_count;
    // ArrayNode* quat_node = (ArrayNode) find_child(obj_node, "rot", NODE_ARRAY);
    // assert(qut_node->child_count == 4);
    // get_children(scene_nodes, children);
    // for (u32 i = 0; i < scene_nodes->child_count; ++i) {
    //     do_smth(children[i]);
    // }

    print_node(root, nodes);

    // DEBUG
    // u32 offset = 0;
    // while (offset < tokens.current) {
    //     u32* type = (u32*) (tokens.ptr + offset);
    //     u32 size = sizeof(SimpleToken);
    //     switch (*type) {
    //         case Token_BrackOpen: {
    //             printf("Token_BrackOpen\n");
    //         } break;
    //         case Token_BrackClose: {
    //             printf("Token_BrackClose\n");
    //         } break;
    //         case Token_ArrayOpen: {
    //             printf("Token_ArrayOpen\n");
    //         } break;
    //         case Token_ArrayClose: {
    //             printf("Token_ArrayClose\n");
    //         } break;
    //         case Token_Colon: {
    //             printf("Token_Colon\n");
    //         } break;
    //         case Token_Comma: {
    //             printf("Token_Comma\n");
    //         } break;
    //         case Token_EOF: {
    //             printf("EOF\n");
    //         } break;
    //         case Token_Error: {
    //             printf("Error\n");
    //         } break;
    //         case Token_String: {
    //             StringToken* token = (StringToken*) type;
    //             printf("Token_String: %.*s\n", token->value.len, token->value.ptr);
    //             size = sizeof(StringToken);
    //         } break;
    //         case Token_Number: {
    //             printf("Token_Number\n");
    //             size = sizeof(NumberToken);
    //         } break;
    //         case Token_Bool: {
    //             printf("Token_Bool\n");
    //             size = sizeof(BoolToken);
    //         } break;
    //         default: {
    //             assert(false);
    //         }
    //     }
    //     offset += size;
    // }
}


