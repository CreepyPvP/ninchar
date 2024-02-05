#include "include/entity.h"





u32 get_entity_type_index(std::string name, Game* game){
    for (u32 i=0; i<game->type_count; i++){
        if ( game->types[i].name.compare(name) == 0){
            return i;
        }
    }
    return -1;
}

void entity_standard_init(Entity* entity, Game* game, u32 x, u32 y)
{
        entity->pos = v3(x, y, 1);
        entity->radius = v3(0.5);
        if(entity->type->extra_data_size != 0){
            char* data_array = (char*)entity->type->extra_data_list;
            void* data_entry = (void*)(data_array + (entity->type->extra_data_size * entity->type->count));
            entity->extra_data = data_entry;
        }else{
            entity->extra_data = NULL;
        }
}
void objective_init(Entity* entity, Game* game, u32 x, u32 y){
    entity_standard_init(entity, game, x, y);
    ((ObjectiveExtraData*)entity->extra_data)->broken = false;
}


void entity_standard_update(Entity* entity, Game* game) {}

void entity_standard_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg)
{
    push_cube(group, entity->pos, v3(0.5), *entity->type->texture, entity->type->render_color);
}


void objective_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg)
{
    if(!((ObjectiveExtraData*)entity->extra_data)->broken){
        entity_standard_render(entity, game, group, dbg);
    }
}

