#include "include/asset_loader.h"
#include "include/json.h"

void load_model(const char* file, Arena* arena)
{
    ObjectNode* root = parse_file(file, arena);

    NumberNode* scene = find_child_number(root, "scene");
    ArrayNode* scenes = find_child_array(root, "scenes");
    ArrayNode* nodes = find_child_array(root, "nodes");
    ArrayNode* animations = find_child_array(root, "animations");
    ArrayNode* materials = find_child_array(root, "materials");
    ArrayNode* meshes = find_child_array(root, "meshes");
    ArrayNode* skins = find_child_array(root, "skins");
    ArrayNode* accessors = find_child_array(root, "accessors");
    ArrayNode* buffer_views = find_child_array(root, "bufferViews");
    ArrayNode* buffers = find_child_array(root, "buffers");
}
