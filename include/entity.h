#include "include/game.h"


u32 get_entity_type_index(std::string name, Game* game);

void entity_standard_init(Entity* entity, Game* game, u32 x, u32 y);

void entity_standard_update(Entity* entity, Game* game);

void entity_standard_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg);

void wall_init(Entity* entity, Game* game, u32 x, u32 y);

void crate_init(Entity* entity, Game* game, u32 x, u32 y);

void objective_init(Entity* entity, Game* game, u32 x, u32 y);

void objective_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg);