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
        entity->collider.extra_data = NULL;
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
    u32 objective_index = get_entity_type_index("Objective", game);
    ObjectiveExtraData* data_array = (ObjectiveExtraData*) game->entity_types[objective_index].extra_data_list;
    printf("%d", entity->type->count);
    data_array[entity->type->count].broken = false;
    entity->collider.extra_data = &data_array[entity->type->count];
    entity->extra_data = &data_array[entity->type->count];
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

