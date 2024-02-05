#include "include/entity.h"





u32 get_entity_type_index(std::string name, Game* game){
    for (u32 i=0; i<game->entity_type_count; i++){
        if ( game->entity_types[i].name.compare(name) == 0){
            return i;
        }
    }
    return -1;
}

void entity_standard_init(Entity* entity, Game* game, u32 x, u32 y)
{
        entity->pos = v3(x, y, 1);
        entity->collider.radius = v3(0.5);
        if(entity->type->extra_data_size != 0){
            char* data_array = (char*)entity->type->extra_data_list;
            void* data_entry = (void*)(data_array + (entity->type->extra_data_size * entity->type->count));
            entity->extra_data = data_entry;
            entity->collider.extra_data = data_entry;
        }else{
            entity->collider.extra_data = NULL;
        }
}
void wall_init(Entity* entity, Game* game, u32 x, u32 y){
    entity_standard_init(entity, game, x, y);
    entity->collider.type = ColliderType_Static;
}
void crate_init(Entity* entity, Game* game, u32 x, u32 y){
    entity_standard_init(entity, game, x, y);
    entity->collider.type = ColliderType_Moveable;
}
void objective_init(Entity* entity, Game* game, u32 x, u32 y){
    entity_standard_init(entity, game, x, y);
    entity->collider.type = ColliderType_Objective;
    ((ObjectiveExtraData*)entity->extra_data)->broken = false;
}


void entity_standard_update(Entity* entity, Game* game) {}

void entity_standard_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg)
{
    push_cube(group, entity->pos, v3(0.5), *entity->type->texture, entity->type->render_color);
}


void objective_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg)
{
    ObjectiveExtraData* data = (ObjectiveExtraData*)entity->extra_data;
    if(!data->broken){
        entity_standard_render(entity, game, group, dbg);
    }
}

