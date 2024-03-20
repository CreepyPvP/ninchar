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

    return buffer->ptr + curr;
}

void load_model(const char* file, Arena* arena)
{
    char* content = read_file(file, NULL, arena);
    assert(content);

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


    // DEBUG
    u32 offset = 0;
    while (offset < buffer.current) {
        u32* type = (u32*) (buffer.ptr + offset);
        u32 size = sizeof(SimpleToken);
        switch (*type) {
            case Token_BrackOpen: {
                printf("Token_BrackOpen\n");
            } break;
            case Token_BrackClose: {
                printf("Token_BrackClose\n");
            } break;
            case Token_ArrayOpen: {
                printf("Token_ArrayOpen\n");
            } break;
            case Token_ArrayClose: {
                printf("Token_ArrayClose\n");
            } break;
            case Token_Colon: {
                printf("Token_Colon\n");
            } break;
            case Token_Comma: {
                printf("Token_Comma\n");
            } break;
            case Token_EOF: {
                printf("EOF\n");
            } break;
            case Token_Error: {
                printf("Error\n");
            } break;
            case Token_String: {
                printf("Token_String\n");
                size = sizeof(StringToken);
            } break;
            case Token_Number: {
                printf("Token_Number\n");
                size = sizeof(NumberToken);
            } break;
            case Token_Bool: {
                printf("Token_Bool\n");
                size = sizeof(BoolToken);
            } break;
            default: {
                assert(false);
            }
        }
        offset += size;
    }
}


