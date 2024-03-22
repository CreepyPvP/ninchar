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

struct ContainerNode
{
    Node info;
    u32 child_count;
    Node* child;
};

typedef ContainerNode ObjectNode;
typedef ContainerNode ArrayNode;

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
    u32 read = (*curr)->type;
    if (read != type) {
        printf("Expected token %u, got token %u instead\n", type, read);
        assert(false);
    }
    advance(curr, size);
}

void parse_block(SimpleToken** curr, SimpleArena* nodes, ObjectNode* node);
void parse_array(SimpleToken** curr, SimpleArena* nodes, ObjectNode* node);

Node* parse_field(SimpleToken** curr, SimpleArena* nodes, bool expect_name)
{
    Node* header;
    StringToken* str = (StringToken*) *curr;

    if (expect_name) {
        advance(curr, sizeof(StringToken));
        expect(curr, Token_Colon, sizeof(SimpleToken));
    }

    switch ((*curr)->type) {
        case Token_Number: {
            NumberNode* node = (NumberNode*) alloc(nodes, sizeof(*node));
            *node = {};
            NumberToken* num = (NumberToken*) *curr;
            node->value = num->value;
            header = (Node*) node;
            header->type = Node_Number;
            header->size = sizeof(*node);
            advance(curr, sizeof(NumberToken));
        } break;

        case Token_String: {
            StringNode* node = (StringNode*) alloc(nodes, sizeof(*node));
            *node = {};
            StringToken* str = (StringToken*) *curr;
            node->value = str->value;
            header = (Node*) node;
            header->type = Node_String;
            header->size = sizeof(*node);
            advance(curr, sizeof(StringToken));
        } break;

        case Token_Bool: {
            BoolNode* node = (BoolNode*) alloc(nodes, sizeof(*node));
            *node = {};
            BoolToken* boolean = (BoolToken*) *curr;
            node->value = boolean->value;
            header = (Node*) node;
            header->type = Node_Bool;
            header->size = sizeof(*node);
            advance(curr, sizeof(BoolToken));
        } break;

        case Token_BrackOpen: {
            ObjectNode* node = (ObjectNode*) alloc(nodes, sizeof(*node));
            *node = {};
            header = (Node*) node;
            parse_block(curr, nodes, node);
        } break;

        case Token_ArrayOpen: {
            ArrayNode* node = (ArrayNode*) alloc(nodes, sizeof(*node));
            *node = {};
            header = (Node*) node;
            parse_array(curr, nodes, node);
        }

        default: {
            assert(false || "Unsupported token");
        }
    }

    if (expect_name) {
        header->name = str->value;
    }

    return header;
}

void parse_block(SimpleToken** curr, SimpleArena* nodes, ObjectNode* node)
{
    expect(curr, Token_BrackOpen, sizeof(SimpleToken));

    node->info.size = sizeof(*node);
    node->info.type = Node_Object;
    node->child_count = 0;

    bool comma = true;
    while ((*curr)->type == Token_String && comma) {
        Node* child = parse_field(curr, nodes, true);
        node->child_count++;
        node->info.size += child->size;

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

// TODO: merge this with parse block?
void parse_array(SimpleToken** curr, SimpleArena* nodes, ArrayNode* node)
{
    expect(curr, Token_ArrayOpen, sizeof(SimpleToken));

    node->info.size = sizeof(*node);
    node->info.type = Node_Array;
    node->child_count = 0;

    bool comma = true;
    while ((*curr)->type != Token_ArrayClose && comma) {
        Node* child = parse_field(curr, nodes, false);
        node->child_count++;
        node->info.size += child->size;

        if (!node->child) {
            node->child = child;
        }

        if ((*curr)->type == Token_Comma) {
            (*curr)++;
        } else {
            comma = false;
        }
    }

    expect(curr, Token_ArrayClose, sizeof(SimpleToken));
}

void print_node(Node* header)
{
    if (header->name.ptr) {
        printf("Got name: %.*s\n", header->name.len, header->name.ptr);
    }

    switch (header->type) {
        case Node_String: {
            StringNode* node = (StringNode*) header;
            printf("Value string: %.*s\n", node->value.len, node->value.ptr);
        } break;

        case Node_Number: {
            NumberNode* node = (NumberNode*) header;
            printf("Value number: %f\n", node->value);
        } break;

        case Node_Object: {
            ObjectNode* node = (ObjectNode*) header;
            printf("Object: child_count %u\n", node->child_count);
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

    SimpleToken* curr = (SimpleToken*) tokens.ptr;
    ObjectNode* root = (ObjectNode*) alloc(&nodes, sizeof(*root));
    *root = {};

    parse_block(&curr, &nodes, root);

    // ArrayNode* scene_nodes = (ArrayNode*) find_child(doc, "nodes", Node_Array);
    // scene_nodes->child_count;
    // ArrayNode* quat_node = (ArrayNode) find_child(obj_node, "rot", NODE_ARRAY);
    // assert(qut_node->child_count == 4);
    // get_children(scene_nodes, children);
    // for (u32 i = 0; i < scene_nodes->child_count; ++i) {
    //     do_smth(children[i]);
    // }

    print_node((Node*) root);

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


