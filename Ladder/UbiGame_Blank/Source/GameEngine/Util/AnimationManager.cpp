#include "AnimationManager.h"


using namespace GameEngine;

AnimationManager* AnimationManager::sm_instance = nullptr;

AnimationManager::AnimationManager()
{

}


AnimationManager::~AnimationManager()
{

}


void AnimationManager::InitStaticGameAnimations()
{
	/*
	//Template definition -> every new animation needs to have an ID and a definition that controls how animation is played
	m_animDefinitions.push_back
	(
	SAnimationDefinition(
	EAnimationId::BirdIdle,
	eTexture::Player,
	sf::Vector2i(0, 0),
	10,
	3)
	);
	*/
    m_animDefinitions.push_back(SAnimationDefinition(EAnimationId::RollingRock, eTexture::Rock, sf::Vector2i(0, 0), 32, 60));
    m_animDefinitions.push_back(SAnimationDefinition(EAnimationId::FiryBall, eTexture::Fireball, sf::Vector2i(0, 0), 6, 60));
    m_animDefinitions.push_back(SAnimationDefinition(EAnimationId::ShootingArrow, eTexture::Arrow, sf::Vector2i(0, 0), 1, 60));
}


void AnimationManager::ReleaseStaticGameAnimations()
{
	m_animDefinitions.clear();
}


const SAnimationDefinition* AnimationManager::GetAnimDefinition(EAnimationId::type animId) const
{
	for (int a = 0; a < m_animDefinitions.size(); ++a)
	{
		if (m_animDefinitions[a].m_animId == animId)
			return &m_animDefinitions[a];
	}

	return nullptr;
}
