#pragma once

#include "GameApplication.h"
#include "agentproc.h"

class GameApplication;
extern GameApplication* app;	// Defined in main.cpp

class Agent: public AgentProc
{
private:
	Ogre::SceneNode* mBodyNode;
	Ogre::Entity* mBodyEntity;

public:
		// all of the animations our character has, and a null ID
	// some of these affect separate body parts and will be blended together

	enum AnimID
	{
		ANIM_IDLE_BASE,
		ANIM_IDLE_TOP,
		ANIM_RUN_BASE,
		ANIM_RUN_TOP,
		ANIM_HANDS_CLOSED,
		ANIM_HANDS_RELAXED,
		ANIM_DRAW_SWORDS,
		ANIM_SLICE_VERTICAL,
		ANIM_SLICE_HORIZONTAL,
		ANIM_DANCE,
		ANIM_JUMP_START,
		ANIM_JUMP_LOOP,
		ANIM_JUMP_END,
		ANIM_NONE
	};

	Agent(std::string name, std::string filename);
	~Agent();

	void update(Ogre::Real deltaTime);		// update the agent

	Ogre::AnimationState* mAnims[13];		// master animation list
	AnimID mBaseAnimID;						// current base (full- or lower-body) animation
	AnimID mTopAnimID;						// current top (upper-body) animation
	bool mFadingIn[13];						// which animations are fading in
	bool mFadingOut[13];					// which animations are fading out
	Ogre::Real mTimer;						// general timer to see how long animations have been playing
	Ogre::Real mVerticalVelocity;			// for jumping

	void setupAnimations();										// Load the animations
	void setBaseAnimation(AnimID id, bool reset = false);		// Set which animation should play
	void setTopAnimation(AnimID id, bool reset = false);		// Set which animation should play on the upper body
	void fadeAnimations(Ogre::Real deltaTime);					// Blend in and out animations
	void updateAnimations(Ogre::Real deltaTime);				// Update the animation to the next frame

};