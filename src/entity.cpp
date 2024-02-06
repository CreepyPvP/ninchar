#include "include/entity.h"





u32 get_entity_type_index(EntityTypeId id, Game* game){
    if(game->types[id].id == id){
        return id;
    }
    for (u32 i=0; i<game->type_count; i++){
        if ( game->types[i].id == id){
            return i;
        }
    }
    return -1;
}

void collider_entity_standard_init(Entity* entity, Game* game, u32 x, u32 y)
{
        entity->pos = v3(x, y, 1);
        ((ColliderEntity*)entity)->radius = v3(0.5);
}
void objective_init(Entity* entity, Game* game, u32 x, u32 y){
    collider_entity_standard_init(entity, game, x, y);
    ((ObjectiveEntity*)entity)->broken = false;
}


void entity_standard_update(Entity* entity, Game* game) {}

void entity_texture_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg)
{
    push_cube(group, entity->pos, v3(0.5),  *(TextureHandle*)(entity->type->render_data), v3(1));
}

void entity_color_cube_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg){
    push_cube(group, entity->pos, v3(0.5), group->commands->white, ((ColorHandle*)entity->type->render_data)->color);
}


void objective_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg)
{
    if(!((ObjectiveEntity*)entity)->broken){
        entity_color_cube_render(entity, game, group, dbg);
    }
}

