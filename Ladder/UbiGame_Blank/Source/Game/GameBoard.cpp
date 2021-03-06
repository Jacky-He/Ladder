#include "GameBoard.h"
#include "GameEngine/GameEngineMain.h"
#include "Game/Components/PlayerMovementComponent.h"
#include "Game/Components/BackgroundMovementComponent.h"
#include "Game/Components/LinkedEntityComponent.h"
#include "GameEngine/EntitySystem/Components/SpriteRenderComponent.h"
#include "GameEngine/EntitySystem/Components/SoundComponent.h"
#include "GameEngine/Util/SoundManager.h"
#include "GameEngine/EntitySystem/Components/TextRenderComponent.h"
#include "Game/GameControls/ObstacleShower.h"
#include "GameEngine/EntitySystem/Components/CollidableComponent.h"
#include "GameEngine/Util/CollisionManager.h"
#include "GameEngine/EntitySystem/Components/TextRenderComponent.h"
#include "Game/Components/PauseMenuComponent.h"
#include "Game/Components/GodControlComponent.h"
#include <SFML/Window/Mouse.hpp>
#include "Game/Components/ThrowProjectileComponent.h"
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <set>

using namespace Game;

sf::Music GameBoard::BGMusic;
float GameBoard::PauseDuration = 0;

GameBoard::GameBoard()
	:m_player(nullptr)
	, m_score(nullptr)
{
	// Read the high scores
	std::ifstream scoreInput;
	scoreInput.open("scores.txt");
	if (scoreInput.is_open()) {
		int x;
		scores.clear();
		while (scoreInput >> x) {
			scores.push_back(x);
		}
		scoreInput.close();
	}

	// Create the entities
	CreatePlayer();
    CreateGod();
	CreateWall();
	CreateLadders();
	CreateFog();
    CreateShower();
	CreatePauseMenu();
	CreateRestartButtons();
}

GameBoard::~GameBoard()
{
    
}

void GameBoard::Restart() {
	// Reset game status
	GameEngine::GameEngineMain::GetInstance()->isPaused = false;
	GameEngine::GameEngineMain::GetInstance()->isRunning = true;

	// Reset restart button status
	restartPressed = false;
	restartButton->GetComponent<GameEngine::SpriteRenderComponent>()->SetZLevel(0);
	restartButtonPressed->GetComponent<GameEngine::SpriteRenderComponent>()->SetZLevel(0);

	// Undo end game
	pauseShade->GetComponent<GameEngine::RenderComponent>()->SetZLevel(0);
	m_score->GetComponent<GameEngine::TextRenderComponent>()->SetColor(sf::Color(222, 180, 33, 255));
	m_shower->EnableShower();
	BGMusic.play();

	// Hide restart button
	restartButton->GetComponent<GameEngine::SpriteRenderComponent>()->SetZLevel(0);

	// Get window size
	sf::RenderWindow* mainWindow = GameEngine::GameEngineMain::GetInstance()->GetRenderWindow();
	unsigned int winWidth = mainWindow->getSize().x;
	unsigned int winHeight = mainWindow->getSize().y;

	// Reset player
	m_player->SetPos(sf::Vector2f(winWidth / 2.0, 4.0 * winHeight / 5));
	m_player->SetSize(sf::Vector2f(72.f, 72.f));
	m_player->GetComponent<PlayerMovementComponent>()->jumpDuration = 0;

	// Reset background
	ladderHiddenCenter->SetPos(sf::Vector2f(winWidth / 2.0, 0));
	wallHiddenCenter->SetPos(sf::Vector2f(winWidth / 2.0, 0));

	// Reset score
	GameEngine::GameEngineMain::score = 0;

	// Reset the timers
	GameEngine::GameEngineMain::GetInstance()->ResetGameTime();

	// Reset obstacles and projectiles
	m_shower -> ClearObstacles();
    m_player -> GetComponent<GameEngine::ThrowProjectileComponent>() -> ClearProjectiles();

	// Update the high scores
	std::string str = "High Scores: \n";
	while (scores.size() > 5) scores.pop_back();
	for (int i = 0; i < scores.size(); i++) {
		str += std::to_string(i + 1);
		str += ") ";
		str += std::to_string(scores[i]);
		str += "\n";
	}
	m_highScores->GetComponent<GameEngine::TextRenderComponent>()->SetString(str);
}

void GameBoard::CreateGod()
{
    m_god = new GameEngine::Entity();
    m_god -> AddComponent<GameEngine::GodControlComponent>();
    GameEngine::GameEngineMain::GetInstance() -> AddEntity(m_god);
}

void GameBoard::CreateShower()
{
    m_shower = new GameEngine::ObstacleShower();
    m_shower -> EnableShower();
}

void GameBoard::CreatePlayer()
{
	sf::RenderWindow* mainWindow = GameEngine::GameEngineMain::GetInstance()->GetRenderWindow();
	unsigned int winWidth = mainWindow->getSize().x;
	unsigned int winHeight = mainWindow->getSize().y;

	m_player = new GameEngine::Entity();
	m_score = new GameEngine::Entity();
	m_highScores = new GameEngine::Entity();
	m_highScoresBack = new GameEngine::Entity();

	GameEngine::GameEngineMain::GetInstance()->AddEntity(m_player);
	GameEngine::GameEngineMain::GetInstance()->AddEntity(m_score);
	GameEngine::GameEngineMain::GetInstance()->AddEntity(m_highScores);
	GameEngine::GameEngineMain::GetInstance()->AddEntity(m_highScoresBack);
    
    m_player -> AddComponent<GameEngine::ThrowProjectileComponent>();
	
	m_score->SetPos(sf::Vector2f(winWidth / 2.0 - 20, 30.f));
	m_score->SetSize(sf::Vector2f(200.f, 72.f));
	m_highScores->SetPos(sf::Vector2f(winWidth - 250, 30.f));
	m_highScores->SetSize(sf::Vector2f(250.f, 720.f));
	m_highScoresBack->SetPos(sf::Vector2f(winWidth - 170, 100.f));
	m_highScoresBack->SetSize(sf::Vector2f(200.f, 300.f));
	m_player->SetPos(sf::Vector2f(winWidth/2.0, 4.0 * winHeight/5));
	m_player->SetSize(sf::Vector2f(72.f, 72.f));
	GameEngine::SpriteRenderComponent* spriteRender = static_cast<GameEngine::SpriteRenderComponent*>(m_player->AddComponent<GameEngine::SpriteRenderComponent>());
	GameEngine::TextRenderComponent* scoreRender = static_cast<GameEngine::TextRenderComponent*>(m_score->AddComponent<GameEngine::TextRenderComponent>());
	GameEngine::TextRenderComponent* hScoreRender = static_cast<GameEngine::TextRenderComponent*>(m_highScores->AddComponent<GameEngine::TextRenderComponent>());
	GameEngine::TextRenderComponent* hScoreRender1 = static_cast<GameEngine::TextRenderComponent*>(m_highScoresBack->AddComponent<GameEngine::TextRenderComponent>());
    
	BGMusic.openFromFile("Resources/snd/music.wav");
	BGMusic.play();
	BGMusic.setLoop(true);

	spriteRender->SetFillColor(sf::Color::Transparent);
	spriteRender->SetZLevel(9);
	spriteRender->SetTexture(GameEngine::eTexture::Player);
	spriteRender->SetTileIndex(2, 0);
	spriteRender->image = 2;
	m_player->AddComponent<PlayerMovementComponent>();
    m_player -> AddComponent<GameEngine::CollidableComponent>();

	scoreRender->SetString("0");
	scoreRender->SetCharacterSizePixels(69);
	scoreRender->SetZLevel(60);
	scoreRender->SetFont("arial.ttf");
	scoreRender->SetFillColor(sf::Color::Transparent);
	scoreRender->SetColor(sf::Color(222, 180, 33, 255));

	std::string str = "High Scores: \n";
	for (int i = 0; i < (int)scores.size(); i++) {
		str += std::to_string(i + 1);
		str += ") ";
		str += std::to_string(scores[i]);
		str += "\n";
	}

	hScoreRender->SetString(str);
	hScoreRender->SetCharacterSizePixels(21);
	hScoreRender->SetZLevel(60);
	hScoreRender->SetFont("ARCADEPI.ttf");
	hScoreRender->SetFillColor(sf::Color(0,0,0,0));
	hScoreRender->SetColor(sf::Color::White);
	
	hScoreRender1->SetString("");
	hScoreRender1->SetZLevel(59);
	hScoreRender1->SetFont("arial.ttf");
	hScoreRender1->SetFillColor(sf::Color(0, 0, 0, 200));
}


void GameBoard::CreateLadders()
{
	// Specify the size of the ladder image
	unsigned int ladderWidth = 76;
	unsigned int ladderHeight = 148;

	// Get the window dimensions
	sf::RenderWindow* mainWindow = GameEngine::GameEngineMain::GetInstance()->GetRenderWindow();
	unsigned int winWidth = mainWindow->getSize().x;
	unsigned int winHeight = mainWindow->getSize().y;
	int copiesStacked = (winHeight / ladderHeight + 1) * 2;

	// Create a hidden center that the ladders can follow
	ladderHiddenCenter = new GameEngine::Entity();
	GameEngine::GameEngineMain::GetInstance()->AddEntity(ladderHiddenCenter);
	ladderHiddenCenter->SetPos(sf::Vector2f(winWidth / 2.0, 0));
	BackgroundMovementComponent* bgComp = ladderHiddenCenter->AddComponent<BackgroundMovementComponent>();
	bgComp->SetSingleHeight(ladderHeight);

	float laddersX[5] = { 0.14, 0.31, 0.5, 0.65, 0.84 };

	for (int i = 0; i < 5; i++) {
		ladders[i] = new GameEngine::Entity*[copiesStacked];
		for (int j = 0; j < copiesStacked; j++) {
			ladders[i][j] = new GameEngine::Entity();
			GameEngine::GameEngineMain::GetInstance()->AddEntity(ladders[i][j]);
			ladders[i][j]->SetSize(sf::Vector2f(ladderWidth, ladderHeight));
			ladders[i][j]->SetPos(sf::Vector2f(winWidth *laddersX[i], winHeight - ladderHeight / 2.0 - ladderHeight * j));
			GameEngine::SpriteRenderComponent* render = ladders[i][j]->AddComponent<GameEngine::SpriteRenderComponent>();
			render->SetFillColor(sf::Color::Transparent);
			render->SetZLevel(2);
			render->SetTexture(GameEngine::eTexture::Ladder);
			LinkedEntityComponent* linkedComp = ladders[i][j]->AddComponent<LinkedEntityComponent>();
			linkedComp->SetFollowedEntity(ladderHiddenCenter);
			sf::Vector2f dif = ladders[i][j]->GetPos() - ladderHiddenCenter->GetPos();
			linkedComp->SetFollowOffset(dif);
		}
	}
}

void GameBoard::CreateWall()
{
	// Specify the size of the wall image
	unsigned int wallWidth = 320;
	unsigned int wallHeight = 320;

	// Get the window dimensions
	sf::RenderWindow* mainWindow = GameEngine::GameEngineMain::GetInstance()->GetRenderWindow();
	unsigned int winWidth = mainWindow->getSize().x;
	unsigned int winHeight = mainWindow->getSize().y;
	int copiesHor = winWidth / wallWidth + 1;
	int copiesVer = (winHeight / wallHeight + 1) * 2;

	// Create a hidden center that the wall images can follow
	wallHiddenCenter = new GameEngine::Entity();
	GameEngine::GameEngineMain::GetInstance()->AddEntity(wallHiddenCenter);
	wallHiddenCenter->SetPos(sf::Vector2f(winWidth / 2.0, 0));
	BackgroundMovementComponent* bgComp = wallHiddenCenter->AddComponent<BackgroundMovementComponent>();
	bgComp->SetSingleHeight(wallHeight);

	walls = new GameEngine::Entity**[copiesHor];
	for (int i = 0; i < copiesHor; i++) {
		walls[i] = new GameEngine::Entity*[copiesVer];
		for (int j = 0; j < copiesVer; j++) {
			walls[i][j] = new GameEngine::Entity();
			GameEngine::GameEngineMain::GetInstance()->AddEntity(walls[i][j]);
			walls[i][j]->SetSize(sf::Vector2f(wallWidth, wallHeight));
			walls[i][j]->SetPos(sf::Vector2f(wallWidth * (i + 0.5), winHeight - wallHeight * (j + 0.5)));
			GameEngine::SpriteRenderComponent* render = walls[i][j]->AddComponent<GameEngine::SpriteRenderComponent>();
			render->SetFillColor(sf::Color::Transparent);
			render->SetZLevel(1);
			render->SetTexture(GameEngine::eTexture::Wall);
			LinkedEntityComponent* linkedComp = walls[i][j]->AddComponent<LinkedEntityComponent>();
			linkedComp->SetFollowedEntity(wallHiddenCenter);
			sf::Vector2f dif = walls[i][j]->GetPos() - wallHiddenCenter->GetPos();
			linkedComp->SetFollowOffset(dif);
		}
	}
}

void GameBoard::CreateFog() {
	// Get the window dimensions
	sf::RenderWindow* mainWindow = GameEngine::GameEngineMain::GetInstance()->GetRenderWindow();
	int winWidth = (int)mainWindow->getSize().x;
	int winHeight = (int)mainWindow->getSize().y;

	// Set the fog dimensions
	int fogWidth = 460;
	int fogHeight = 270;
	int copiesHor = winWidth / fogWidth + 1;

	fog = new GameEngine::Entity * [copiesHor];
	for (int i = 0; i < copiesHor; i++) {
		fog[i] = new GameEngine::Entity();
		GameEngine::GameEngineMain::GetInstance()->AddEntity(fog[i]);
		fog[i]->SetSize(sf::Vector2f(fogWidth, fogHeight));
		fog[i]->SetPos(sf::Vector2f(fogWidth * (i + 0.5), fogHeight / 2.0));
		GameEngine::SpriteRenderComponent* render = fog[i]->AddComponent<GameEngine::SpriteRenderComponent>();
		render->SetFillColor(sf::Color::Transparent);
		render->SetZLevel(25);
		render->SetTexture(GameEngine::eTexture::Fog);
	}
}

void GameBoard::CreatePauseMenu() {
	// Get the window dimensions
	sf::RenderWindow* mainWindow = GameEngine::GameEngineMain::GetInstance()->GetRenderWindow();
	int winWidth = (int)mainWindow->getSize().x;
	int winHeight = (int)mainWindow->getSize().y;

	// Create a message that says "Paused"
	pauseText = new GameEngine::Entity();
	GameEngine::GameEngineMain::GetInstance()->AddEntity(pauseText);
	pauseText->SetPos(sf::Vector2f(winWidth / 2 - 70, winHeight / 2 - 30));
	pauseText->SetSize(sf::Vector2f(200, 100));
	GameEngine::TextRenderComponent* textRender = pauseText->AddComponent<GameEngine::TextRenderComponent>();
	textRender->SetFont("arial.ttf");
	textRender->SetString("Paused");
	textRender->SetZLevel(0);
	textRender->SetCharacterSizePixels(40);
	textRender->SetFillColor(sf::Color::Transparent);

	if (BGMusic.getStatus() != sf::SoundStream::Paused) {
		BGMusic.pause();
	}

	// Create a dark shade to block the game while the game is paused
	pauseShade = new GameEngine::Entity();
	GameEngine::GameEngineMain::GetInstance()->AddEntity(pauseShade);
	pauseShade->SetPos(sf::Vector2f(winWidth / 2.0, winHeight / 2.0));
	pauseShade->SetSize(sf::Vector2f(winWidth, winHeight));
	GameEngine::RenderComponent* shadeRender = pauseShade->AddComponent<GameEngine::RenderComponent>();
	shadeRender->SetZLevel(0);
	shadeRender->SetFillColor(sf::Color(0, 0, 0, 200));
}

void GameBoard::CreateRestartButtons() {
	// Get the window dimensions
	sf::RenderWindow* mainWindow = GameEngine::GameEngineMain::GetInstance()->GetRenderWindow();
	int winWidth = (int)mainWindow->getSize().x;
	int winHeight = (int)mainWindow->getSize().y;

	// Create the restart button
	restartButton = new GameEngine::Entity();
	GameEngine::GameEngineMain::GetInstance()->AddEntity(restartButton);
	restartButton->SetPos(sf::Vector2f(winWidth / 2, winHeight / 2));
	restartButton->SetSize(sf::Vector2f(64, 64));
	GameEngine::SpriteRenderComponent* render = restartButton->AddComponent<GameEngine::SpriteRenderComponent>();
	render->SetFillColor(sf::Color::Transparent);
	render->SetZLevel(0);
	render->SetTexture(GameEngine::eTexture::BackButton);

	// Create the restart button when pressed
	restartButtonPressed = new GameEngine::Entity();
	GameEngine::GameEngineMain::GetInstance()->AddEntity(restartButtonPressed);
	restartButtonPressed->SetPos(sf::Vector2f(winWidth / 2, winHeight / 2));
	restartButtonPressed->SetSize(sf::Vector2f(64, 64));
	GameEngine::SpriteRenderComponent* pressedRender = restartButtonPressed->AddComponent<GameEngine::SpriteRenderComponent>();
	pressedRender->SetFillColor(sf::Color::Transparent);
	pressedRender->SetZLevel(0);
	pressedRender->SetTexture(GameEngine::eTexture::BackButtonPressed);
}

void GameBoard::Update()
{
	if (!GameEngine::GameEngineMain::isRunning) {	// a game is finished
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {	// left click
			// Get game window
			sf::RenderWindow* mainWindow = GameEngine::GameEngineMain::GetInstance()->GetRenderWindow();

			// Find mouse position in window
			int mouseX = sf::Mouse::getPosition(*mainWindow).x;
			int mouseY = sf::Mouse::getPosition(*mainWindow).y;

			// Store the size and position of the restart button entity
			float halfRestartWidth = restartButton->GetSize().x / 2.0;
			float halfRestartHeight = restartButton->GetSize().y / 2.0;
			float restartX = restartButton->GetPos().x;
			float restartY = restartButton->GetPos().y;

			// Check if the mouse click is on the button
			if (mouseX <= restartX + halfRestartWidth && mouseX >= restartX - halfRestartWidth &&
				mouseY <= restartY + halfRestartHeight && mouseY >= restartY - halfRestartHeight && !restartPressed) {
				restartButtonPressed->GetComponent<GameEngine::SpriteRenderComponent>()->SetZLevel(71);
				restartButton->GetComponent<GameEngine::SpriteRenderComponent>()->SetZLevel(0);
				restartPressed = true;
			}
		}
		else if (restartPressed) {	// Restart on release
			Restart();
		}
		return;
	}

    m_shower -> Update();
    using namespace GameEngine;
    
    std::vector<CollidableComponent*>& collidables = CollisionManager::GetInstance()->GetCollidables();
    GameEngine::CollidableComponent* thiscol = m_player -> GetComponent<GameEngine::CollidableComponent>();
	GameEngine::TextRenderComponent* scoreRender = m_score->GetComponent<GameEngine::TextRenderComponent>();
    GameEngine::ThrowProjectileComponent* throwproj = m_player -> GetComponent<GameEngine::ThrowProjectileComponent>();
    for (int a = 0; a < collidables.size(); ++a)
    {
        CollidableComponent* colComponent = collidables[a];
        if (colComponent == thiscol || (throwproj -> GetProjectiles()).count(colComponent -> GetEntity()))
            continue;

        AABBRect intersection;
        AABBRect myBox = thiscol -> GetWorldAABB();
        AABBRect colideBox = colComponent -> GetWorldAABB();
        if (myBox.intersects(colideBox, intersection))
        {
            //end game
			GameEngine::GameEngineMain::GetInstance()->isRunning = false;
			m_shower->DisableShower();
			scoreRender->SetColor(sf::Color(224, 36, 0, 255));
			BGMusic.stop();
			pauseShade->GetComponent<GameEngine::RenderComponent>()->SetZLevel(59);

			//save high score
			std::ofstream scoreOutput("scores.txt");
			scores.push_back((int)GameEngine::GameEngineMain::GetInstance()->score);
			std::sort(scores.rbegin(), scores.rend());
			while (scores.size() > 5) scores.pop_back();
			for (int i = 0; i < scores.size(); i++) {
				scoreOutput << scores[i] << " ";
			}
			scoreOutput << std::endl;
			scoreOutput.close();

			// Show restart button
			restartButton->GetComponent<GameEngine::SpriteRenderComponent>()->SetZLevel(70);
        }
    }

	if (!GameEngine::GameEngineMain::GetInstance()->isRunning) return;

	float dt = GameEngine::GameEngineMain::GetTimeDelta();
	GameBoard::PauseDuration -= dt;
	if (GameBoard::PauseDuration <= 0) {
		GameBoard::PauseDuration = 0;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
			GameEngine::GameEngineMain::GetInstance()->isPaused ^= true;
			if (GameEngine::GameEngineMain::GetInstance()->isPaused) {
				pauseText->GetComponent<GameEngine::TextRenderComponent>()->SetZLevel(60);
				pauseShade->GetComponent<GameEngine::RenderComponent>()->SetZLevel(59);
				pauseClock.restart();
				m_shower->DisableShower();
			}
			else {
				pauseText->GetComponent<GameEngine::TextRenderComponent>()->SetZLevel(0);
				pauseShade->GetComponent<GameEngine::RenderComponent>()->SetZLevel(0);
				GameEngine::GameEngineMain::GetInstance()->sm_pauseTime += pauseClock.getElapsedTime();
				m_shower->EnableShower();
			}
			GameBoard::PauseDuration = 0.2;
		}
	}

	scoreRender->SetString(std::to_string((int)GameEngine::GameEngineMain::GetInstance()->score));

	if (BGMusic.getStatus() != sf::SoundStream::Playing && (GameEngine::GameEngineMain::GetInstance()->isRunning && !GameEngine::GameEngineMain::GetInstance()->isPaused)) {
		BGMusic.play();
	}
	else if (BGMusic.getStatus() != sf::SoundStream::Paused && (!GameEngine::GameEngineMain::GetInstance()->isRunning || GameEngine::GameEngineMain::GetInstance()->isPaused)) {
		BGMusic.pause();
	}
}
