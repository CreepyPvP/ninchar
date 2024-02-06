#include "include/game.h"


u32 get_entity_type_index(EntityTypeId id, Game* game);

void collider_entity_standard_init(Entity* entity, Game* game, u32 x, u32 y, u8 r, u8 g, u8 b);

void entity_standard_update(Entity* entity, Game* game, u8 inputs, float delta);

void entity_texture_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg);

void entity_color_cube_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg);

void objective_init(Entity* entity, Game* game, u32 x, u32 y, u8 r, u8 g, u8 b);

void objective_render(Entity* entity, Game* game, RenderGroup* group, RenderGroup* dbg);



void standard_collision_response(AABB a, AABB b, V2 dir, Game* game);

void moveable_collision_response(AABB a, AABB b, V2 dir, Game* game);

void objective_collision_response(AABB a, AABB b, V2 dir, Game* game);

V2 static_try_move_into(AABB a, AABB b, V2 dir, Game* game);

V2 moveable_try_move_into(AABB a, AABB b, V2 dir, Game* game);

V2 noclip_try_move_into(AABB a, AABB b, V2 dir, Game* game);
