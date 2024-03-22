#include "include/json.h"
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
    buffer.cap = 100000;
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
            token->value.cap = token->value.len;

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

inline void advance(Node** curr, u32 size) 
{
    u8* ptr = (u8*) *curr;
    *curr = (Node*) (ptr + size);
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

void parse_container(SimpleToken** curr, SimpleArena* nodes, ContainerNode* node, bool is_block);

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
            parse_container(curr, nodes, node, true);
        } break;

        case Token_ArrayOpen: {
            ArrayNode* node = (ArrayNode*) alloc(nodes, sizeof(*node));
            *node = {};
            header = (Node*) node;
            parse_container(curr, nodes, node, false);
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

void parse_container(SimpleToken** curr, SimpleArena* nodes, ContainerNode* node, bool is_block)
{
    node->info.size = sizeof(*node);
    node->child_count = 0;

    TokenType open_token;
    TokenType close_token;

    if (is_block) {
        node->info.type = Node_Object;
        open_token = Token_BrackOpen;
        close_token = Token_BrackClose;
    } else {
        node->info.type = Node_Array;
        open_token = Token_ArrayOpen;
        close_token = Token_ArrayClose;
    }

    expect(curr, open_token, sizeof(SimpleToken));

    bool comma = true;
    while ((*curr)->type != close_token && comma) {
        Node* child = parse_field(curr, nodes, is_block);
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

    expect(curr, close_token, sizeof(SimpleToken));
}

Node* assert_type(Node* header, NodeType type) 
{
    if (header && header->type == Node_Object) {
        return header;
    }
    return NULL;
}

Node* find_child(ObjectNode* node, const char* name)
{
    Node* header = node->child;
    for (u32 i = 0; i < node->child_count; ++i) {
        if (str_equals(&header->name, name)) {
            return header;
        }
        advance(&header, header->size);
    }

    return NULL;
}

Node* find_child(ContainerNode* node, u32 index)
{
    if (index > node->child_count) {
        return NULL;
    }

    Node* header = node->child;
    for (u32 i = 0; i < index; ++i) {
        advance(&header, header->size);
    }

    return header;
}

ObjectNode* find_child_object(ObjectNode* node, const char* name) 
{
    return (ObjectNode*) assert_type(find_child(node, name), Node_Object); 
}

ArrayNode* find_child_array(ObjectNode* node, const char* name) 
{
    return (ArrayNode*) assert_type(find_child(node, name), Node_Array); 
}

StringNode* find_child_string(ObjectNode* node, const char* name) 
{
    return (StringNode*) assert_type(find_child(node, name), Node_String); 
}

NumberNode* find_child_number(ObjectNode* node, const char* name) 
{
    return (NumberNode*) assert_type(find_child(node, name), Node_Number); 
}

BoolNode* find_child_bool(ObjectNode* node, const char* name) 
{
    return (BoolNode*) assert_type(find_child(node, name), Node_Bool); 
}

ObjectNode* find_child_object(ObjectNode* node, u32 index) 
{
    return (ObjectNode*) assert_type(find_child(node, index), Node_Object); 
}

ArrayNode* find_child_array(ObjectNode* node, u32 index) 
{
    return (ArrayNode*) assert_type(find_child(node, index), Node_Array); 
}

StringNode* find_child_string(ObjectNode* node, u32 index) 
{
    return (StringNode*) assert_type(find_child(node, index), Node_String); 
}

NumberNode* find_child_number(ObjectNode* node, u32 index) 
{
    return (NumberNode*) assert_type(find_child(node, index), Node_Number); 
}

BoolNode* find_child_bool(ObjectNode* node, u32 index) 
{
    return (BoolNode*) assert_type(find_child(node, index), Node_Bool); 
}

ObjectNode* parse_file(const char* file, Arena* arena)
{
    char* content = read_file(file, NULL, arena);
    assert(content);

    SimpleArena tokens = json_lexer(content, arena);
    SimpleArena nodes = {};
    nodes.cap = 100000;
    nodes.ptr = (u8*) push_size(arena, nodes.cap);

    SimpleToken* curr = (SimpleToken*) tokens.ptr;
    ObjectNode* root = (ObjectNode*) alloc(&nodes, sizeof(*root));
    *root = {};

    parse_container(&curr, &nodes, root, true);
    // ArrayNode* arr = find_child_array(root, "an_array");
    // printf("Child count: %u\n", arr->child_count);
    return root;
}


