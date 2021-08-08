
void
EntityAddCollision(game_state * GameState, entity Entity)
{
#if 0
    Assert((GameState->TotalCollisionSpheres + 1) < ArrayCount(GameState->CollisionSpheres));
    RitterSphere(GameState->CollisionSpheres, Entity->Mesh->Vertices,Entity->Mesh->VertexSize);
#endif
    EntityAddFlag(GameState,Entity,component_collision);
}
