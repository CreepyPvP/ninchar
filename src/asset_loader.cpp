#include "include/asset_loader.h"
#include "include/json.h"

void load_model(const char* file, Arena* arena)
{
    ObjectNode* root = parse_file(file, arena);

    ArrayNode* scenes = find_child_array(root, "scenes");
    ObjectNode* scene_0 = get_child_object(scenes, 0);
    StringNode* name = find_child_string(scene_0, "name");
}
