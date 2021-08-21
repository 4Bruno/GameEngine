#include "game.h"

#if 0
animation
NewAnimation(u32 StepCount, r32 StepSize, b32 Interrumpible)
{
    Assert((StepCount > 0.0f && StepSize != 0.0f));

    animation A = {};

    A.Started       = false;                    // b32 Started;
    A.t             = 0.0f;                     // r32 t;
    A.StepCount     = (r32)StepCount;        // r32 StepCount;
    A.StepSize      = StepSize;                 // r32 StepSize;
    A._TimeStep     = 1.0f / StepCount;         // r32 _TimeStep;
    A.Interrumpible = Interrumpible;

    //Quaternion_setIdentity(&A.StartRotation); // RECORD StartRotation;
    //Quaternion_setIdentity(&A.EndRotation);   // RECORD EndRotation;

    return A;
}

void
BeginAnimateRotate(game_state * GameState,entity Entity, animation * Animation)
{
    Assert(Animation->_TimeStep > 0.0f);

    Quaternion rotateLeft;
    Quaternion_fromYRotation((90.0f / 180.0f * PI),&rotateLeft);
    Quaternion * R = &GetEntityTransform(GameState, Entity)->WorldR;
    Animation->StartRotation = *R;
    Quaternion_multiply(&rotateLeft,R,R);
    Quaternion_multiply(&rotateLeft,R,&Animation->EndRotation);
    Animation->t= 0.0f;
    Animation->Started= true;
}

void
AnimateRotate(game_state * GameState, entity Entity, animation * Animation)
{
    Assert(Animation->_TimeStep > 0.0f);

    if (Animation->t < 1.0f)
    {
        entity_transform * T = GetEntityTransform(GameState, Entity);
        Quaternion_slerp(&Animation->StartRotation, 
                &Animation->EndRotation, 
                Animation->t, 
                &T->LocalR);
        v3 StepDir = V3(Animation->StepSize, 0 ,0);
        v3 DesiredP = TestEntityMoveForward(T,&StepDir);
        T->LocalP = DesiredP;
    }
    else
    {
        Animation->Started = false;
    }
    Animation->t += Animation->_TimeStep;
}
#endif
