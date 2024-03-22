#include "include/asset_loader.h"
#include "include/json.h"

void load_model(const char* file, Arena* arena)
{
    ObjectNode* root = parse_file(file, arena);
}
