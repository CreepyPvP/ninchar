#include "include/asset_loader.h"
#include "include/json.h"

void process_nodes(ArrayNode* list, ObjectNode** nodes, Arena* arena) 
{
    NumberNode** indices = (NumberNode**) push_size(arena, sizeof(*indices) * list->child_count);
    u32 count = list_number(list, indices, list->child_count);

    for (u32 i = 0; i < count; ++i) {
        NumberNode* index = indices[i];
        ObjectNode* node = nodes[(u32) index->value];

        StringNode* name = get_string(node, "name");
        printf("%.*s\n", name->value.len, name->value.ptr);

        ArrayNode* children = get_array(node, "children");
        if (children) {
            process_nodes(children, nodes, arena);
        }
    }
}

void load_model(const char* file, Arena* arena)
{
    ObjectNode* root = parse_file(file, arena);

    NumberNode* main_scene_id = get_number(root, "scene");
    ArrayNode* scenes = get_array(root, "scenes");
    ArrayNode* node_list = get_array(root, "nodes");
    ObjectNode** nodes = (ObjectNode**) push_size(arena, sizeof(*nodes) * node_list->child_count);
    u32 node_count = list_object(node_list, nodes, node_list->child_count);

    ObjectNode* main_scene = at_object(scenes, main_scene_id->value);
    ArrayNode* root_nodes = get_array(main_scene, "nodes");

    process_nodes(root_nodes, nodes, arena);
}
