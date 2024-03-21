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

struct TokenBuffer
{
    u8* ptr; 
    u32 current; 
    u32 cap;
    u32 token_count;
};

struct Node
{
    u32 type;
    u32 size;
    Str name;

    union {
        float number;
        Str string;
        bool boolean;

        struct {
            u32 offset;
            u32 child_count;
        } container;

    };
};

bool is_number(char c) 
{
    return (c >= '0' && c <= '9') || c == '-' || c == '+';
}

u8* push_token(TokenBuffer* buffer, u32 size)
{
    u32 curr = buffer->current;
    assert(buffer->current + size < buffer->cap);
    buffer->current += size;

    buffer->token_count++;

    return buffer->ptr + curr;
}

TokenBuffer json_lexer(char* content, Arena* arena)
{
    TokenBuffer buffer = {};
    buffer.cap = 10000;
    buffer.ptr = (u8*) push_size(arena, buffer.cap);

    char* curr = content;
    while (*curr) {
        if (*curr == ' ' || *curr == '\n' || *curr == '\t' || *curr == '\r') {
            curr++;
            continue;
        }

        if (*curr == '"') {
            StringToken* token = (StringToken*) push_token(&buffer, sizeof(StringToken));

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
            BoolToken* token = (BoolToken*) push_token(&buffer, sizeof(BoolToken));
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
            NumberToken* token = (NumberToken*) push_token(&buffer, sizeof(NumberToken));
            token->type = Token_Number;
            token->value = atof(curr);
            while (is_number(*curr) || *curr == '.' || *curr == 'e') {
                curr++;
            }
            continue;
        }

        SimpleToken* token = (SimpleToken*) push_token(&buffer, sizeof(SimpleToken));

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

    SimpleToken* eof_token = (SimpleToken*) push_token(&buffer, sizeof(SimpleToken));
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

void parse_block(SimpleToken** curr, Node* nodes, u32* node_count, Node* node);

void parse_field(SimpleToken** curr, Node* nodes, u32* node_count)
{
    StringToken* str = (StringToken*) *curr;

    Node* node = nodes + *node_count;
    (*node_count)++;

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
            parse_block(curr, nodes, node_count, node);
        } break;

        default: {
            assert(false || "Arrays not implemented");
        }
    }
}

void parse_block(SimpleToken** curr, Node* nodes, u32* node_count, Node* node)
{
    expect(curr, Token_BrackOpen, sizeof(SimpleToken));

    node->type = Node_Object;
    node->container.offset = *node_count;
    node->container.child_count = 0;

    bool comma = true;
    while ((*curr)->type == Token_String && comma) {
        parse_field(curr, nodes, node_count);
        node->container.child_count++;

        if ((*curr)->type == Token_Comma) {
            (*curr)++;
        } else {
            comma = false;
        }
    }

    node->size = node->container.child_count + 1;

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

    TokenBuffer tokens = json_lexer(content, arena);
    u32 node_count = 1;
    Node* nodes = (Node*) push_size(arena, sizeof(Node) * tokens.token_count);

    SimpleToken* curr = (SimpleToken*) tokens.ptr;
    Node* root = nodes;
    *root = {};
    parse_block(&curr, nodes, &node_count, root);

    // ArrayNode* scene_nodes = (ArrayNode*) find_child(doc, "nodes", NODE_ARRAY);
    // scene_nodes->child_count;
    // ArrayNode* quat_node = (ArrayNode) find_child(obj_node, "rot", NODE_ARRAY);
    // assert(qut_node->child_count == 4);

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


