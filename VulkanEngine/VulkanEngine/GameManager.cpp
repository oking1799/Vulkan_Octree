#include "pch.h"
#include "GameManager.h"

#include "DebugManager.h"
#include "EntityManager.h"
#include "InputManager.h"
#include "Camera.h"
#include "QuadTree.h"
#include "Octree.h"

#define MshMngr MeshManager::GetInstance()

#pragma region Singleton

GameManager* GameManager::instance = nullptr;

GameManager* GameManager::GetInstance()
{
    if (instance == nullptr) {
        instance = new GameManager();
    }

    return instance;
}

#pragma endregion

#pragma region Accessors

std::vector<std::shared_ptr<Light>> GameManager::GetLights()
{
    return lights;
}

std::shared_ptr<GameObject> GameManager::GetObjectByName(std::string name)
{
    for (int i = 0; i < gameObjects.size(); i++) {
        if (gameObjects[i]->GetName().compare(name) == 0) {
            return gameObjects[i];
        }
    }

    std::cout << "Could not find object with name: " << name << std::endl;
    return nullptr;
}

#pragma endregion

#pragma region Game Loop

void GameManager::Init()
{
    //Setup Lights
    lights.push_back(std::make_shared<Light>(glm::vec3(1.5f, 1.1f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 5.0f));
    lights.push_back(std::make_shared<Light>(glm::vec3(0.0f, 2.0f, -1.5f), glm::vec3(1.0f, 0.988f, 0.769f), 3.0f, 4.0f));
    
	float KDrange = 2.0f;
	int entitycount = 50; 
	std::vector<glm::vec2> linePoints;

	//QuadTree* quadTree = new QuadTree();
	OctTreeManager::InitOctTree(-2.0f, 2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 4, 2);

	//seed the random
	srand(time(NULL));
	//create cubes to be used by the KD tree.
	//Number of objects is set in Fixed_Data.h
	for (int i = 0; i < entitycount; i++) {

		//create random x and y coordinates for the cubes
		//range is between -2.0f and 2.0f for now
		float randX = (float)rand() / (float)RAND_MAX;
		float randRange = KDrange - -KDrange;
		float finalRandX = (randX * randRange) + -KDrange;
		float randY = (float)rand() / (float)RAND_MAX;
		float finalRandY = (randY * randRange) + -KDrange;
		float randZ = (float)rand() / (float)RAND_MAX;
		float finalRandZ = (randZ * randRange) + -KDrange;

		//add a cube to the gameobject vector
		gameObjects.push_back(std::make_shared<GameObject>(EntityManager::GetInstance()->GetMeshes()[MeshTypes::Cube]));

		//set data, place position at random coords
		gameObjects[i]->SetTransform(std::make_shared<Transform>(glm::vec3()));
		gameObjects[i]->GetTransform()->SetPosition(glm::vec3(finalRandX, finalRandY, finalRandZ));
		gameObjects[i]->GetTransform()->SetOrientation(glm::vec3(0.0f, 0.0f, 0.0f));
		gameObjects[i]->GetTransform()->SetScale(glm::vec3(0.05f, 0.05f, 0.05f));
		gameObjects[i]->SetPhysicsObject(std::make_shared<PhysicsObject>(gameObjects[i]->GetTransform(), PhysicsLayers::Trigger, 1.0f, false, false));
		gameObjects[i]->SetName("Cube");

		OctTreeManager::AddShape(gameObjects[i]);

		
		//quadTree->Insert(gameObjects[i]->GetTransform()->GetPosition(), linePoints);
	}

	gameObjects.push_back(std::make_shared<GameObject>(EntityManager::GetInstance()->GetMeshes()[MeshTypes::Skybox]));
	int lastIndex = gameObjects.size() - 1;
	// setup skybox to enhnace the scene

	gameObjects[lastIndex]->SetTransform(std::make_shared<Transform>(glm::vec3(0)));
	gameObjects[lastIndex]->SetPhysicsObject(std::make_shared<PhysicsObject>(gameObjects[lastIndex]->GetTransform(), PhysicsLayers::Trigger, 1.0f, false, false));
	gameObjects[lastIndex]->SetName("Skybox");

    for (size_t i = 0; i < gameObjects.size(); i++) {
        gameObjects[i]->Spawn();
    }
}

void GameManager::Update()
{

	OctTreeManager::UpdateOctTree();
    // MshMngr->ClearRenderList();
    // MeshManager::GetInstance()->DrawWireCube(glm::vec3(1.0f, 2.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	
	
	//hold and drag the screen to move the camera
    if (InputManager::GetInstance()->GetKey(Controls::RightClick)) {
        lockCamera = !lockCamera; 
	}
	else {
		lockCamera = true;
	}
    if (InputManager::GetInstance()->GetKey(Controls::LeftClick)) {
        lockCamera = !lockCamera;
	}
	else {
		lockCamera = true;
	}

    //  Rotate camera if not locked
    if (!lockCamera) {
        glm::vec2 deltaMouse = InputManager::GetInstance()->GetDeltaMouse();
        if (deltaMouse.x != 0 || deltaMouse.y != 0) {
            deltaMouse = glm::normalize(deltaMouse) / 4.0f;
        }

        glm::quat orientation = Camera::GetMainCamera()->GetTransform()->GetOrientation();
        glm::vec3 rotation = orientation * glm::vec3(deltaMouse.y, 0.0f, 0.0f) + glm::vec3(0.0f, -deltaMouse.x, 0.0f);

        Camera::GetMainCamera()->GetTransform()->Rotate(rotation);
    }

    //Move Cube
    glm::vec3 moveDirection = glm::vec3(0.0f, 0.0f, 0.0f);

	//switch to up and down for now
	//Using wasd will move the cube in the corresponding directions
	//w-up, s-down, a-left, d-right
    if (InputManager::GetInstance()->GetKey(Controls::Forward)) {
        moveDirection += glm::vec3(0.0f, 0.0f, 1.0f);
    }
    if (InputManager::GetInstance()->GetKey(Controls::Back)) {
        moveDirection += glm::vec3(0.0f, 0.0f, -1.0f);
    }
	//Q and E inputs, disabled for now
    if (InputManager::GetInstance()->GetKey(Controls::Up)) {
        moveDirection += glm::vec3(0.0f, 1.0f, 0.0f);
    }
    if (InputManager::GetInstance()->GetKey(Controls::Down)) {
        moveDirection += glm::vec3(0.0f, -1.0f, 0.0f);
    }
    if (InputManager::GetInstance()->GetKey(Controls::Left)) {
        moveDirection += glm::vec3(-1.0f, 0.0f, 0.0f);
    }
    if (InputManager::GetInstance()->GetKey(Controls::Right)) {
        moveDirection += glm::vec3(1.0f, 0.0f, 0.0f);
    }

    if (moveDirection.x != 0 || moveDirection.y != 0 || moveDirection.z != 0) {
        moveDirection = glm::normalize(moveDirection);
    }

    Camera::GetMainCamera()->GetTransform()->Translate(moveDirection * cameraSpeed * Time::GetDeltaTime(), true);
    //Update Lights
    float scaledTime = Time::GetTotalTime() * 2.0f;
    lights[0]->position = glm::vec3(0.0f, 1.1f, 0.0f) + glm::vec3(cos(scaledTime), 0.0f, sin(scaledTime)) * 1.5f;
    
    //Update Game Objects
    for (size_t i = 0; i < gameObjects.size(); i++) {
        gameObjects[i]->Update();
    }

    if (InputManager::GetInstance()->GetKeyPressed(Controls::Jump)) {
        gameObjects[2]->GetPhysicsObject()->ApplyForce(glm::vec3(0.0f, 5000.0f, 0.0f));

        //Spawn Object Sample Code:
        /*
        std::shared_ptr<GameObject> newObject = std::make_shared<GameObject>(EntityManager::GetInstance()->GetMeshes()[MeshTypes::Sphere]);
        gameObjects.push_back(newObject);

        newObject->SetTransform(std::make_shared<Transform>(glm::vec3(0.0f, 2.5f, 0.0f)));
        newObject->SetPhysicsObject(std::make_shared<PhysicsObject>(newObject->GetTransform(), PhysicsLayers::Static, 1.0f, false, true));

        newObject->Spawn();
        */
    }
}

#pragma endregion