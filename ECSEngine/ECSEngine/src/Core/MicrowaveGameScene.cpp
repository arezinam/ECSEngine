#include "MicrowaveGameScene.h"
#include "Core/GameObject.h"
#include "Input/InputManager.h"
#include "Components/MeshComponent.h"
#include "Components/RenderComponent.h"
#include "Components/TransformComponent.h"
#include "Components/Camera2DComponent.h"
#include "Components/ClickableComponent.h" 
#include "Core/FontRenderer.h"
#include "Core/AssetManager.h"
#include "Core/Shader.h"
#include "Core/Texture.h"
#include "Core/Mesh.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iomanip>
#include <sstream>
#include <cmath>

MicrowaveGameScene::MicrowaveGameScene(const std::string& name)
	: Scene(name),
	m_microwaveGameObject(nullptr),
	m_windowGameObject(nullptr),
	m_hexContainerGameObject(nullptr),
	m_lightContainerGameObject(nullptr),
	m_windowPivotGameObject(nullptr),
	m_interiorContainerGameObject(nullptr),
	m_displayContainer(nullptr),
	m_doorAnimationTime(0.0f),
	m_animationDuration(0.5f),
	m_initialWindowWorldX(0.0f),
	m_initialWindowWorldY(0.0f), 
	m_initialWindowWidth(0.0f),
	m_timerTextRenderComponent(nullptr),
	m_hexRenderComponent(nullptr),
	m_flickerTimer(0.0f),
	m_flickerSpeed(20.0f),
	m_flickerMinAlpha(0.2f),
	m_flickerMaxAlpha(0.8f),
	m_baseLightColor(1.0f, 1.0f, 0.0f, 0.0f),
	m_errorBlinkTimer(0.0f),
	m_errorBlinkInterval(0.3f),
	m_isErrorTextVisible(true),
	m_currentDoorTargetAngle(0.0f),
	m_runningStateIndicator(nullptr),
	m_runningStateIndicatorRenderComponent(nullptr),
	m_runningBlinkTimer(0.0f),
	m_runningBlinkInterval(0.55f),
	m_isIndicatorVisible(true)
{
	std::cout << "MicrowaveGameScene '" << m_name << "' created." << std::endl;
}

MicrowaveGameScene::~MicrowaveGameScene() {
	std::cout << "MicrowaveGameScene '" << m_name << "' destroyed." << std::endl;
}

void MicrowaveGameScene::Init() {
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	Scene::Init();
	std::cout << "MicrowaveGameScene '" << m_name << "' initialized with 2D objects." << std::endl;
	SetupMicrowaveGameObjects();
}


glm::vec4 MicrowaveGameScene::calculateFlicker(float time, float speed) {
	glm::vec3 baseColor = glm::vec3(1.0f, 1.0f, 0.0f); 

	float flickerFactor = std::sin(time * speed) * 0.5f + 0.5f;
	float randomFactor = (std::rand() % 100) / 100.0f * 0.3f; 

	glm::vec3 flickerColor = glm::mix(baseColor, glm::vec3(1.0f), flickerFactor * randomFactor);

	float alpha = glm::mix(0.0f, 1.0f, flickerFactor * randomFactor);

	return glm::vec4(flickerColor, alpha);
}


void MicrowaveGameScene::Update(float deltaTime) {
	Scene::Update(deltaTime);

	static float accumulatedTimeForTick = 0.0f;
	accumulatedTimeForTick += deltaTime;

	if (accumulatedTimeForTick >= 1.0f) {
		m_microwave.tick();
		updateTimerDisplay(); 

		accumulatedTimeForTick -= 1.0f;
	}

	if (m_microwave.getCurrentState() == Microwave::State::BROKEN) {
		m_flickerTimer += deltaTime;
		if (m_lightContainerGameObject && m_lightContainerGameObject->getComponent<RenderComponent>()) {
			RenderComponent* lightRenderComp = m_lightContainerGameObject->getComponent<RenderComponent>();
			glm::vec4 flickeringColor = calculateFlicker(
				m_flickerTimer,
				m_flickerSpeed
			);
			lightRenderComp->setObjectColor(flickeringColor);
		}

		m_errorBlinkTimer += deltaTime;
		if (m_errorBlinkTimer >= m_errorBlinkInterval) {
			m_isErrorTextVisible = !m_isErrorTextVisible;
			updateTimerDisplay();
			m_errorBlinkTimer -= m_errorBlinkInterval;
		}
		if (m_smokeFilterRenderComponent) {
			float targetAlpha = 0.7f;
			float increaseSpeed = 0.15f;
			float currentSmokeAlpha = m_smokeFilterRenderComponent->m_objectColor.a;
			float newSmokeAlpha = glm::min(currentSmokeAlpha + deltaTime * increaseSpeed, targetAlpha);
			m_smokeFilterRenderComponent->setObjectColor(glm::vec4(0.5f, 0.5f, 0.5f, newSmokeAlpha));
		}
	}
	else if (m_microwave.getCurrentState() == Microwave::State::COOKING) {
		if (m_lightContainerGameObject && m_lightContainerGameObject->getComponent<RenderComponent>()) {
			RenderComponent* lightRenderComp = m_lightContainerGameObject->getComponent<RenderComponent>();
			glm::vec4 currentColor = lightRenderComp->m_objectColor;
			float targetAlpha = m_microwave.isLightOn() ? m_baseLightColor.a : 0.0f; 
			float fadeSpeed = 5.0f;
			currentColor.a = glm::mix(currentColor.a, targetAlpha, deltaTime * fadeSpeed);
			lightRenderComp->setObjectColor(currentColor);
		}

		if (m_runningStateIndicatorRenderComponent) {
			m_runningBlinkTimer += deltaTime;
			if (m_runningBlinkTimer >= m_runningBlinkInterval) {
				m_isIndicatorVisible = !m_isIndicatorVisible;
				if (m_isIndicatorVisible) {
					m_runningStateIndicatorRenderComponent->setObjectColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));  
				}
				else {
					m_runningStateIndicatorRenderComponent->setObjectColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); 
				}
				m_runningBlinkTimer -= m_runningBlinkInterval;
			}
		}
		if (!m_isErrorTextVisible) { m_isErrorTextVisible = true; updateTimerDisplay(); }
		m_errorBlinkTimer = 0.0f;
		m_flickerTimer = 0.0f;
		if (m_smokeFilterRenderComponent) {
			float targetAlpha = 0.0f;
			float decreaseSpeed = 0.5f;
			float currentSmokeAlpha = m_smokeFilterRenderComponent->m_objectColor.a;
			float newSmokeAlpha = glm::max(currentSmokeAlpha - deltaTime * decreaseSpeed, targetAlpha);
			m_smokeFilterRenderComponent->setObjectColor(glm::vec4(0.5f, 0.5f, 0.5f, newSmokeAlpha));
		}
	}
	else { 
		if (m_lightContainerGameObject && m_lightContainerGameObject->getComponent<RenderComponent>()) {
			RenderComponent* lightRenderComp = m_lightContainerGameObject->getComponent<RenderComponent>();
			glm::vec4 currentColor = lightRenderComp->m_objectColor;

			float targetAlpha = m_microwave.isLightOn() ? m_baseLightColor.a : 0.0f;
			float fadeSpeed = 5.0f;

			currentColor.a = glm::mix(currentColor.a, targetAlpha, deltaTime * fadeSpeed);
			lightRenderComp->setObjectColor(currentColor);
		}
		if (!m_isErrorTextVisible) {
			m_isErrorTextVisible = true;
			updateTimerDisplay();
		}
		m_errorBlinkTimer = 0.0f;
		m_flickerTimer = 0.0f; 

		if (m_smokeFilterRenderComponent) {
			float targetAlpha = 0.0f;
			float decreaseSpeed = 0.5f; 
			float currentSmokeAlpha = m_smokeFilterRenderComponent->m_objectColor.a;
			float newSmokeAlpha = glm::max(currentSmokeAlpha - deltaTime * decreaseSpeed, targetAlpha);
			m_smokeFilterRenderComponent->setObjectColor(glm::vec4(0.5f, 0.5f, 0.5f, newSmokeAlpha));
		}
		if (m_runningStateIndicatorRenderComponent) {
			m_runningStateIndicatorRenderComponent->setObjectColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); 
			m_isIndicatorVisible = true; 
		}
		m_runningBlinkTimer = 0.0f; 
	}


	for (int i = 0; i <= 9; ++i) {
		int glfw_key = GLFW_KEY_0 + i;
		if (InputManager::getInstance().isKeyJustPressed(glfw_key)) {
			m_microwave.inputNumber(i);
			updateTimerDisplay();
		}
	}

	if (InputManager::getInstance().isKeyJustPressed(GLFW_KEY_ENTER)) {
		m_microwave.startCooking();
	}

	if (InputManager::getInstance().isKeyJustPressed(GLFW_KEY_BACKSPACE)) { 
		m_microwave.stopCooking();
		updateTimerDisplay();
	}

	if (InputManager::getInstance().isKeyJustPressed(GLFW_KEY_DELETE)) { 
		m_microwave.clearInput();
		updateTimerDisplay();
	}


	if (InputManager::getInstance().isKeyJustPressed(GLFW_KEY_R)) { 
		m_microwave.repairMicrowave();
		updateTimerDisplay(); 
	}

	if (InputManager::getInstance().isKeyJustPressed(GLFW_KEY_P)) {
		m_hexContainerGameObject->getComponent<RenderComponent>()->m_objectColor.a = m_hexContainerGameObject->getComponent<RenderComponent>()->m_objectColor.a == 0.5f ? 1.0f : 0.5f;
	}

	if (InputManager::getInstance().isKeyJustPressed(GLFW_KEY_SPACE)) {
		if (m_microwave.getDoorState() == Microwave::DoorState::CLOSED) {
			m_microwave.openDoor();
			m_currentDoorTargetAngle = -180.0f; 
		}
		else {
			m_microwave.closeDoor();
			m_currentDoorTargetAngle = 0.0f; 
		}
		m_doorAnimationTime = 0.0f;
	}

	if (InputManager::getInstance().isKeyJustPressed(GLFW_KEY_X)) {
		m_microwave.breakMicrowave();
		updateTimerDisplay();
	}


	if (m_windowPivotGameObject && m_windowPivotGameObject->getTransform()) {
		float currentAnimatedDoorAngle = m_windowPivotGameObject->getTransform()->getLocalEulerAnglesDegrees().y;

			m_doorAnimationTime += deltaTime;
			float progress = glm::clamp(m_doorAnimationTime / m_animationDuration, 0.0f, 1.0f);

			float newAngle = glm::mix(currentAnimatedDoorAngle, m_currentDoorTargetAngle, progress * 4.0f);

			if (m_currentDoorTargetAngle > currentAnimatedDoorAngle) {
				newAngle = glm::min(newAngle, m_currentDoorTargetAngle);
			}
			else {
				newAngle = glm::max(newAngle, m_currentDoorTargetAngle);
			}

			m_windowPivotGameObject->getTransform()->setLocalEulerRotation(glm::vec3(0.0f, newAngle, 0.0f));

			if (progress >= 1.0f) { 
				m_windowPivotGameObject->getTransform()->setLocalEulerRotation(glm::vec3(0.0f, m_currentDoorTargetAngle, 0.0f));
			}
		

	}
}

void MicrowaveGameScene::Render() {
	Scene::Render();

	if (m_fontRenderer) {
		std::string nameText = "Milan Arežina, SV55/2021";
		float nameTextScale = 0.3f;
		float estimatedCharWidth = 48.0f * 0.6f;
		float textPixelWidth = nameText.length() * estimatedCharWidth * nameTextScale;
		float padding = 10.0f;
		float nameX = 800.0f - textPixelWidth - padding;
		float nameY = padding; 
		m_fontRenderer->renderText(nameText, nameX, nameY, nameTextScale, glm::vec3(0.0f, 0.0f, 0.0f));

		float controlTextScale = 0.3f;
		float controlPaddingX = 10.0f;
		float controlPaddingY = 10.0f;
		float lineHeight = 48.0f * controlTextScale * 1.2f; 

		float currentY = getWindowHeight() - 20.0f; 
		glm::vec3 controlsColor = glm::vec3(0.0f, 0.0f, 0.0f);
		m_fontRenderer->renderText("Keyboard Controls:", controlPaddingX, currentY, controlTextScale * 1.2f, controlsColor);
		currentY -= lineHeight;
		m_fontRenderer->renderText("  Start -> ENTER", controlPaddingX, currentY, controlTextScale, controlsColor);
		currentY -= lineHeight;
		m_fontRenderer->renderText("  Stop/Pause -> BACKSPACE", controlPaddingX, currentY, controlTextScale, controlsColor);
		currentY -= lineHeight;
		m_fontRenderer->renderText("  Toggle door -> SPACE (or click door)", controlPaddingX, currentY, controlTextScale, controlsColor);
		currentY -= lineHeight;
		m_fontRenderer->renderText("  Numbers Input -> 0-9", controlPaddingX, currentY, controlTextScale, controlsColor);
		currentY -= lineHeight;
		m_fontRenderer->renderText("  Reset timer input -> DELETE", controlPaddingX, currentY, controlTextScale, controlsColor);
		currentY -= lineHeight;
		m_fontRenderer->renderText("  Break Microwave -> X", controlPaddingX, currentY, controlTextScale, controlsColor);
		currentY -= lineHeight;
		m_fontRenderer->renderText("  Repair Microwave -> R", controlPaddingX, currentY, controlTextScale, controlsColor);
		currentY -= lineHeight;
		m_fontRenderer->renderText("  Toggle window transparency -> P", controlPaddingX, currentY, controlTextScale, controlsColor);

	}
}

void MicrowaveGameScene::Shutdown() {
	std::cout << "Shutting down MicrowaveGameScene '" << m_name << "'..." << std::endl;
	m_microwaveGameObject = nullptr;
	m_windowGameObject = nullptr;
	m_hexContainerGameObject = nullptr;
	m_lightContainerGameObject = nullptr;
	m_windowPivotGameObject = nullptr;
	m_interiorContainerGameObject = nullptr;
	m_displayContainer = nullptr;
	m_timerTextRenderComponent = nullptr;
	m_hexRenderComponent = nullptr;
	Scene::Shutdown();
	std::cout << "MicrowaveGameScene '" << m_name << "' shutdown complete." << std::endl;
}

void MicrowaveGameScene::updateTimerDisplay() {
	if (!m_timerTextRenderComponent) {
		std::cerr << "Error: Timer text render component not found." << std::endl;
		return;
	}

	std::string timeString;
	glm::vec4 textColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); 

	if (m_microwave.getCurrentState() == Microwave::State::BROKEN) {
		if (m_isErrorTextVisible) {
			timeString = "ERROR";
			textColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else {
			timeString = ""; 
		}
	}
	else {
		Microwave::Time remainingTime = m_microwave.getRemainingTime();
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(2) << remainingTime.minutes << ":"
			<< std::setfill('0') << std::setw(2) << remainingTime.seconds;
		timeString = ss.str();
	}

	float timerTextPixelSize = 64.0f;

	std::unique_ptr<Texture> newTimerTextTexture = m_fontRenderer->GenerateTextTexture(
		"res/fonts/Roboto-Regular.ttf",
		timeString,
		static_cast<int>(timerTextPixelSize)
	);

	if (newTimerTextTexture) {
		m_timerTextRenderComponent->setTexture(std::shared_ptr<Texture>(std::move(newTimerTextTexture)));
		m_timerTextRenderComponent->setObjectColor(textColor);
	}
	else {
		std::cerr << "WARNING: Failed to generate timer text texture for string: '" << timeString << "'." << std::endl;
		m_timerTextRenderComponent->setTexture(nullptr);
		m_timerTextRenderComponent->setObjectColor(glm::vec4(1.0f, 0.0f, 0.0f, 0.5f)); 
	}
}


void MicrowaveGameScene::SetupMicrowaveGameObjects() {
	std::shared_ptr<Shader> basicShader = AssetManager::getInstance().getShader("res/shaders/basic.vert", "res/shaders/basic.frag");
	if (!basicShader) {
		std::cerr << "ERROR: Failed to load basic shader! Objects may not render." << std::endl;
		return;
	}
	std::shared_ptr<Texture> wallTexture = AssetManager::getInstance().getTexture("res/textures/pizza.png", "diffuse");
	std::shared_ptr<Texture> grassTexture = AssetManager::getInstance().getTexture("res/textures/kitchenCounter.png", "diffuse");


	auto cameraObject = std::make_unique<GameObject>("MainCamera");
	CameraBaseComponent* cameraComp = cameraObject->addComponent<Camera2DComponent>(
		static_cast<float>(getWindowWidth()),
		static_cast<float>(getWindowHeight()),
		-1.0f, 1.0f
	);
	AddGameObject(std::move(cameraObject));
	setActiveCamera(cameraComp);


	auto background = std::make_unique<GameObject>("Background");
	auto smokeFilter = std::make_unique<GameObject>("SmokeFilter");
	auto microwaveBody = std::make_unique<GameObject>("MicrowaveBody");
	auto windowPivot = std::make_unique<GameObject>("WindowPivot");
	auto windowActual = std::make_unique<GameObject>("WindowActual");
	auto interiorContainer = std::make_unique<GameObject>("InteriorContainer");
	auto topBar = std::make_unique<GameObject>("WindowTopBar");
	auto bottomBar = std::make_unique<GameObject>("WindowBottomBar");
	auto leftBar = std::make_unique<GameObject>("WindowLeftBar");
	auto rightBar = std::make_unique<GameObject>("WindowRightBar");


	auto m_displayContainer_GameObject = std::make_unique<GameObject>("DisplayContainer");

	auto lightContainer = std::make_unique<GameObject>("LightContainer");
	auto foodContainer = std::make_unique<GameObject>("FoodContainer");
	auto hexContainer = std::make_unique<GameObject>("HexContainer");


	auto timerContainer = std::make_unique<GameObject>("TimerContainer");
	auto timerTextContainer = std::make_unique<GameObject>("TimerTextContainer");

	auto keyboardLayoutContainer = std::make_unique<GameObject>("KeyboardLayoutContainer");

	auto runningIndicator = std::make_unique<GameObject>("RunningIndicator");

	MeshComponent* bgMeshComp = nullptr;
	RenderComponent* bgRenderComp = nullptr;
	MeshComponent* mwBodyMeshComp = nullptr;
	RenderComponent* mwBodyRenderComp = nullptr;
	MeshComponent* topBarMeshComp = nullptr;
	RenderComponent* topBarRenderComp = nullptr;
	MeshComponent* bottomBarMeshComp = nullptr;
	RenderComponent* bottomBarRenderComp = nullptr;
	MeshComponent* leftBarMeshComp = nullptr;
	RenderComponent* leftBarRenderComp = nullptr;
	MeshComponent* rightBarMeshComp = nullptr;
	RenderComponent* rightBarRenderComp = nullptr;
	MeshComponent* displayMeshComp = nullptr;
	RenderComponent* displayRenderComp = nullptr;
	MeshComponent* lightMeshComp = nullptr;
	RenderComponent* lightRenderComp = nullptr;
	MeshComponent* foodMeshComp = nullptr;
	RenderComponent* foodRenderComp = nullptr;
	MeshComponent* hexMeshComp = nullptr;
	RenderComponent* hexRenderComp = nullptr;
	MeshComponent* timerMeshComp = nullptr;
	RenderComponent* timerRenderComp = nullptr;
	MeshComponent* keyboardLayoutMeshComp = nullptr;
	RenderComponent* keyboardLayoutRenderComp = nullptr;
	MeshComponent* smokeFilterMeshComp = nullptr;
	RenderComponent* smokeFilterRenderComp = nullptr;
	MeshComponent* indicatorMeshComp = nullptr;
	RenderComponent* indicatorRenderComp = nullptr;




	float microwaveWidth = 400.0f;
	float microwaveHeight = 250.0f;
	float microwavePosX = getWindowWidth() / 2.0f;
	float microwavePosY = getWindowHeight() / 2.0f - 100.0f;

	float internalPaddingX = 20.0f;
	float internalPaddingY = 20.0f;
	float separationX = 20.0f;

	float relativePaddingX = internalPaddingX / microwaveWidth;
	float relativePaddingY = internalPaddingY / microwaveHeight;
	float relativeSeparationX = separationX / microwaveWidth;

	float availableLocalRelativeWidth = 1.0f - (2 * relativePaddingX) - relativeSeparationX;
	float windowLocalRelativeWidth = availableLocalRelativeWidth * 0.7f;
	float displayLocalRelativeWidth = availableLocalRelativeWidth * 0.3f;
	float commonLocalRelativeHeight = 1.0f - (2 * relativePaddingY);

	float windowHingeLocalX_relative_to_microwave_center = -0.5f + relativePaddingX;
	float windowHingeLocalY_relative_to_microwave_center = -0.5f + relativePaddingY + (commonLocalRelativeHeight / 2.0f);


	float holeWidthRatio = 0.8f;
	float holeHeightRatio = 0.8f;

	float barThicknessX_ratio = (1.0f - holeWidthRatio) / 2.0f;
	float barThicknessY_ratio = (1.0f - holeHeightRatio) / 2.0f;

	float innerContentRelativeWidth_in_hole = holeWidthRatio;
	float innerContentRelativeHeight_in_hole = holeHeightRatio;

	float foodWidthRatio = 0.5f;
	float foodHeightRatio = 0.4f;
	float foodLocalY = -0.5f + (foodHeightRatio / 2.0f) + 0.1f;

	glm::vec4 frameColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.8f);
	glm::vec4 microwaveBodyColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
	glm::vec4 smokeFilterColor = glm::vec4(0.7f, 0.7f, 0.7f, 0.0f);
	glm::vec4 windowGlassColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.6f);
	glm::vec4 displayColor = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
	glm::vec4 timerBackgroundColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 keyboardLayoutBackgroundColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	glm::vec4 keypadButtonBackgroundColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);


	float desiredDisplayPixelPadding = 10.0f;


	float keypadButtonPadding = 0.1f;


;


	bgMeshComp = background->addComponent<MeshComponent>();
	bgMeshComp->getMesh()->generateQuad2D();
	bgRenderComp = background->addComponent<RenderComponent>(basicShader);
	bgRenderComp->setMesh(bgMeshComp->getMesh());
	bgRenderComp->setTexture(grassTexture);
	bgRenderComp->setObjectColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	background->getTransform()->setLocalPosition(glm::vec3(getWindowWidth() / 2.0f, getWindowHeight() / 2.0f, 0.0f));
	background->getTransform()->setLocalScale(glm::vec3(getWindowWidth(), getWindowHeight(), 1.0f));
	AddGameObject(std::move(background));

	smokeFilterMeshComp = smokeFilter->addComponent<MeshComponent>();
	smokeFilterMeshComp->getMesh()->generateQuad2D();
	smokeFilterRenderComp = smokeFilter->addComponent<RenderComponent>(basicShader);
	smokeFilterRenderComp->setMesh(smokeFilterMeshComp->getMesh());
	smokeFilterRenderComp->setObjectColor(smokeFilterColor);
	smokeFilterRenderComp->setTexture(nullptr);
	smokeFilter->getTransform()->setLocalPosition(glm::vec3(getWindowWidth() / 2.0f, getWindowHeight() / 2.0f, 0.0f));
	smokeFilter->getTransform()->setLocalScale(glm::vec3(getWindowWidth(), getWindowHeight(), 1.0f));
	m_smokeFilterRenderComponent = smokeFilterRenderComp;


	mwBodyMeshComp = microwaveBody->addComponent<MeshComponent>();
	mwBodyMeshComp->getMesh()->generateQuad2D();
	mwBodyRenderComp = microwaveBody->addComponent<RenderComponent>(basicShader);
	mwBodyRenderComp->setMesh(mwBodyMeshComp->getMesh());
	mwBodyRenderComp->setObjectColor(microwaveBodyColor);
	mwBodyRenderComp->setTexture(nullptr);
	m_microwaveGameObject = AddGameObject(std::move(microwaveBody));
	m_microwaveGameObject->getTransform()->setLocalPosition(glm::vec3(microwavePosX, microwavePosY, 0.0f));
	m_microwaveGameObject->getTransform()->setLocalScale(glm::vec3(microwaveWidth, microwaveHeight, 1.0f));

	m_smokeFilterGameObject = AddGameObject(std::move(smokeFilter));




	interiorContainer->getTransform()->setLocalPosition(glm::vec3(
		windowHingeLocalX_relative_to_microwave_center + (windowLocalRelativeWidth / 2.0f),
		windowHingeLocalY_relative_to_microwave_center,
		0.0f
	));
	interiorContainer->getTransform()->setLocalScale(glm::vec3(
		windowLocalRelativeWidth,
		commonLocalRelativeHeight,
		1.0f
	));

	MeshComponent* mwInteriorComp = interiorContainer->addComponent<MeshComponent>();
	mwInteriorComp->getMesh()->generateQuad2D();
	RenderComponent* mwInteriorRenderComp = interiorContainer->addComponent<RenderComponent>(basicShader);
	mwInteriorRenderComp->setMesh(mwInteriorComp->getMesh());
	mwInteriorRenderComp->setObjectColor(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
	mwInteriorRenderComp->setTexture(nullptr);
	m_interiorContainerGameObject = m_microwaveGameObject->addChild(std::move(interiorContainer));
	

	lightMeshComp = lightContainer->addComponent<MeshComponent>();
	lightMeshComp->getMesh()->generateQuad2D();
	lightRenderComp = lightContainer->addComponent<RenderComponent>(basicShader);
	lightRenderComp->setMesh(lightMeshComp->getMesh());
	m_baseLightColor.a = 0.7f; 
	lightRenderComp->setObjectColor(m_baseLightColor);
	lightRenderComp->setTexture(nullptr);
	lightContainer->getTransform()->setLocalPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	lightContainer->getTransform()->setLocalScale(glm::vec3(1.0f, 1.0f, 1.0f));
	m_lightContainerGameObject = m_interiorContainerGameObject->addChild(std::move(lightContainer));

	foodMeshComp = foodContainer->addComponent<MeshComponent>();
	foodMeshComp->getMesh()->generateQuad2D();
	foodRenderComp = foodContainer->addComponent<RenderComponent>(basicShader);
	foodRenderComp->setMesh(foodMeshComp->getMesh());
	foodRenderComp->setTexture(wallTexture);
	foodRenderComp->setObjectColor(glm::vec4(0.6f, 0.4f, 0.2f, 1.0f));
	foodContainer->getTransform()->setLocalPosition(glm::vec3(0.0f, foodLocalY, 0.0f));
	foodContainer->getTransform()->setLocalScale(glm::vec3(foodWidthRatio, foodHeightRatio, 1.0f));
	m_interiorContainerGameObject->addChild(std::move(foodContainer));

	windowPivot->getTransform()->setLocalPosition(glm::vec3(
		windowHingeLocalX_relative_to_microwave_center,
		windowHingeLocalY_relative_to_microwave_center,
		0.0f
	));
	windowPivot->getTransform()->setLocalScale(glm::vec3(1.0f, 1.0f, 1.0f));
	m_windowPivotGameObject = m_microwaveGameObject->addChild(std::move(windowPivot));

	MeshComponent* windowActualMeshComp = windowActual->addComponent<MeshComponent>();
	windowActualMeshComp->getMesh()->generateQuad2D();
	RenderComponent* windowActualRenderComp = windowActual->addComponent<RenderComponent>(basicShader);
	windowActualRenderComp->setMesh(windowActualMeshComp->getMesh());
	windowActualRenderComp->setObjectColor(windowGlassColor);
	windowActualRenderComp->setTexture(nullptr);

	windowActual->getTransform()->setLocalPosition(glm::vec3(
		(windowLocalRelativeWidth / 2.0f),
		0.0f,
		0.0f
	));
	windowActual->getTransform()->setLocalScale(glm::vec3(
		windowLocalRelativeWidth,
		commonLocalRelativeHeight,
		1.0f
	));
	m_windowGameObject = m_windowPivotGameObject->addChild(std::move(windowActual));
	ClickableComponent* windowClickable = m_windowGameObject->addComponent<ClickableComponent>(PickingMethod::Method2D);
	windowClickable->setOnClickCallback([this]() {
		if (m_microwave.getDoorState() == Microwave::DoorState::CLOSED) {
			m_microwave.openDoor();
			m_currentDoorTargetAngle = -180.0f;
		}
		else {
			m_microwave.closeDoor();
			m_currentDoorTargetAngle = 0.0f;
		}
		m_doorAnimationTime = 0.0f;
		std::cout << "Microwave door clicked! New state: " << m_microwave.getDoorStateName() << std::endl;
		});
	topBarMeshComp = topBar->addComponent<MeshComponent>();
	topBarMeshComp->getMesh()->generateQuad2D();
	topBarRenderComp = topBar->addComponent<RenderComponent>(basicShader);
	topBarRenderComp->setMesh(topBarMeshComp->getMesh());
	topBarRenderComp->setObjectColor(frameColor);
	topBar->getTransform()->setLocalPosition(glm::vec3(0.0f, 0.5f - (barThicknessY_ratio / 2.0f), 0.0f));
	topBar->getTransform()->setLocalScale(glm::vec3(1.0f, barThicknessY_ratio, 1.0f));
	m_windowGameObject->addChild(std::move(topBar));

	bottomBarMeshComp = bottomBar->addComponent<MeshComponent>();
	bottomBarMeshComp->getMesh()->generateQuad2D();
	bottomBarRenderComp = bottomBar->addComponent<RenderComponent>(basicShader);
	bottomBarRenderComp->setMesh(bottomBarMeshComp->getMesh());
	bottomBarRenderComp->setObjectColor(frameColor);
	bottomBar->getTransform()->setLocalPosition(glm::vec3(0.0f, -0.5f + (barThicknessY_ratio / 2.0f), 0.0f));
	bottomBar->getTransform()->setLocalScale(glm::vec3(1.0f, barThicknessY_ratio, 1.0f));
	m_windowGameObject->addChild(std::move(bottomBar));

	leftBarMeshComp = leftBar->addComponent<MeshComponent>();
	leftBarMeshComp->getMesh()->generateQuad2D();
	leftBarRenderComp = leftBar->addComponent<RenderComponent>(basicShader);
	leftBarRenderComp->setMesh(leftBarMeshComp->getMesh());
	leftBarRenderComp->setObjectColor(frameColor);
	leftBar->getTransform()->setLocalPosition(glm::vec3(-0.5f + (barThicknessX_ratio / 2.0f), 0.0f, 0.0f));
	leftBar->getTransform()->setLocalScale(glm::vec3(barThicknessX_ratio, 1.0f - (2 * barThicknessY_ratio), 1.0f));
	m_windowGameObject->addChild(std::move(leftBar));

	rightBarMeshComp = rightBar->addComponent<MeshComponent>();
	rightBarMeshComp->getMesh()->generateQuad2D();
	rightBarRenderComp = rightBar->addComponent<RenderComponent>(basicShader);
	rightBarRenderComp->setMesh(rightBarMeshComp->getMesh());
	rightBarRenderComp->setObjectColor(frameColor);
	rightBar->getTransform()->setLocalPosition(glm::vec3(0.5f - (barThicknessX_ratio / 2.0f), 0.0f, 0.0f));
	rightBar->getTransform()->setLocalScale(glm::vec3(barThicknessX_ratio, 1.0f - (2 * barThicknessY_ratio), 1.0f));
	m_windowGameObject->addChild(std::move(rightBar));

	hexMeshComp = hexContainer->addComponent<MeshComponent>();
	hexMeshComp->getMesh()->generateQuad2D();
	hexRenderComp = hexContainer->addComponent<RenderComponent>(basicShader);
	hexRenderComp->setMesh(hexMeshComp->getMesh());
	hexRenderComp->setObjectColor(glm::vec4(0.35f, 0.55f, 1.0f, 0.5f));
	hexRenderComp->setTexture(nullptr);
	hexContainer->getTransform()->setLocalPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	hexContainer->getTransform()->setLocalScale(glm::vec3(innerContentRelativeWidth_in_hole, innerContentRelativeHeight_in_hole, 1.0f));
	m_hexContainerGameObject = m_windowGameObject->addChild(std::move(hexContainer));
	m_hexRenderComponent = hexRenderComp;



	m_initialWindowWorldX = m_windowPivotGameObject->getTransform()->getWorldMatrix()[3].x;
	m_initialWindowWorldY = m_windowPivotGameObject->getTransform()->getWorldMatrix()[3].y;
	m_initialWindowWidth = windowLocalRelativeWidth * microwaveWidth;


	displayMeshComp = m_displayContainer_GameObject->addComponent<MeshComponent>();
	displayMeshComp->getMesh()->generateQuad2D();
	displayRenderComp = m_displayContainer_GameObject->addComponent<RenderComponent>(basicShader);
	displayRenderComp->setMesh(displayMeshComp->getMesh());
	displayRenderComp->setObjectColor(displayColor);
	displayRenderComp->setTexture(nullptr);

	float displayLocalX_relative = windowHingeLocalX_relative_to_microwave_center + windowLocalRelativeWidth + relativeSeparationX + (displayLocalRelativeWidth / 2.0f);
	float displayLocalY_relative = windowHingeLocalY_relative_to_microwave_center;
	m_displayContainer = m_microwaveGameObject->addChild(std::move(m_displayContainer_GameObject));
	m_displayContainer->getTransform()->setLocalPosition(glm::vec3(displayLocalX_relative, displayLocalY_relative, 0.0f));
	m_displayContainer->getTransform()->setLocalScale(glm::vec3(displayLocalRelativeWidth, commonLocalRelativeHeight, 1.0f));

	float actualDisplayWidthPixels = displayLocalRelativeWidth * microwaveWidth;
	float actualDisplayHeightPixels = commonLocalRelativeHeight * microwaveHeight;


	float displayHorizontalPaddingRelative = desiredDisplayPixelPadding / actualDisplayWidthPixels;
	float displayVerticalPaddingRelative = desiredDisplayPixelPadding / actualDisplayHeightPixels;


	float effectiveDisplayWidth = 1.0f - (2 * displayHorizontalPaddingRelative);
	float effectiveDisplayHeight = 1.0f - (2 * displayVerticalPaddingRelative);


	timerMeshComp = timerContainer->addComponent<MeshComponent>();
	timerMeshComp->getMesh()->generateQuad2D();
	timerRenderComp = timerContainer->addComponent<RenderComponent>(basicShader);
	timerRenderComp->setMesh(timerMeshComp->getMesh());
	timerRenderComp->setObjectColor(timerBackgroundColor);
	timerRenderComp->setTexture(nullptr);

	float timerRelativeHeight = 0.20f;
	float timerLocalY = (effectiveDisplayHeight / 2.0f) - (timerRelativeHeight / 2.0f) - (displayVerticalPaddingRelative / 2.0f);

	timerContainer->getTransform()->setLocalPosition(glm::vec3(0.0f, timerLocalY, 0.0f));
	timerContainer->getTransform()->setLocalScale(glm::vec3(effectiveDisplayWidth, timerRelativeHeight, 1.0f));
	GameObject* m_timerContainer = m_displayContainer->addChild(std::move(timerContainer));

	MeshComponent* timerTextMeshComp_ptr = timerTextContainer->addComponent<MeshComponent>();
	RenderComponent* timerTextRenderComp_ptr = timerTextContainer->addComponent<RenderComponent>(basicShader);
	timerTextMeshComp_ptr->getMesh()->generateQuad2D();
	timerTextRenderComp_ptr->setMesh(timerTextMeshComp_ptr->getMesh());

	m_timerTextRenderComponent = timerTextRenderComp_ptr;


	float timerTextPixelSize = 64.0f;

	std::unique_ptr<Texture> initialTimerTextTexture = m_fontRenderer->GenerateTextTexture(
		"res/fonts/Roboto-Regular.ttf",
		"00:00",
		static_cast<int>(timerTextPixelSize)
	);

	if (initialTimerTextTexture) {
		m_timerTextRenderComponent->setTexture(std::shared_ptr<Texture>(std::move(initialTimerTextTexture)));
		m_timerTextRenderComponent->setObjectColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}
	else {
		std::cerr << "WARNING: Failed to generate initial timer text texture using FontRenderer::GenerateTextTexture." << std::endl;
		m_timerTextRenderComponent->setTexture(nullptr);
		m_timerTextRenderComponent->setObjectColor(glm::vec4(1.0f, 0.0f, 0.0f, 0.5f));
	}

	float actualTimerWidthPixels = effectiveDisplayWidth * actualDisplayWidthPixels;
	float actualTimerHeightPixels = timerRelativeHeight * actualDisplayHeightPixels;

	float timerHorizontalPaddingRelative = desiredDisplayPixelPadding / actualTimerWidthPixels;
	float timerVerticalPaddingRelative = desiredDisplayPixelPadding / actualTimerHeightPixels;

	float effectiveTimerWidth = 1.0f - (2 * timerHorizontalPaddingRelative);
	float effectiveTimerHeight = 1.0f - (2 * timerVerticalPaddingRelative);

	timerTextContainer->getTransform()->setLocalPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	timerTextContainer->getTransform()->setLocalScale(glm::vec3(effectiveTimerWidth, effectiveTimerHeight, 1.0f));
	m_timerContainer->addChild(std::move(timerTextContainer));

	if (m_timerContainer) {
		glm::mat4 timerWorldMatrix = m_timerContainer->getTransform()->getWorldMatrix();
		float finalTimerWorldWidth = timerWorldMatrix[0][0];
		float finalTimerWorldHeight = timerWorldMatrix[1][1];
		std::shared_ptr<Texture> currentTimerTexture = m_timerTextRenderComponent->m_texture;
		if (currentTimerTexture) {
			std::cout << "DEBUG: Timer Container World Dims (pixels): " << finalTimerWorldWidth << "x" << finalTimerWorldHeight
				<< " | Timer Text Texture Dims: " << currentTimerTexture->getWidth() << "x" << currentTimerTexture->getHeight() << std::endl;
		}
		else {
			std::cout << "DEBUG: Timer Container World Dims (pixels): " << finalTimerWorldWidth << "x" << finalTimerWorldHeight
				<< " | Timer Text Texture Dims: (N/A - texture not generated)" << std::endl;
		}
	}


	keyboardLayoutMeshComp = keyboardLayoutContainer->addComponent<MeshComponent>();
	keyboardLayoutMeshComp->getMesh()->generateQuad2D();
	keyboardLayoutRenderComp = keyboardLayoutContainer->addComponent<RenderComponent>(basicShader);
	keyboardLayoutRenderComp->setMesh(keyboardLayoutMeshComp->getMesh());
	keyboardLayoutRenderComp->setObjectColor(keyboardLayoutBackgroundColor);
	keyboardLayoutRenderComp->setTexture(nullptr);

	float keyboardRelativeHeight = 0.50f;
	float keyboardLocalY = -(effectiveDisplayHeight / 2.0f) + (keyboardRelativeHeight / 2.0f) + (displayVerticalPaddingRelative / 2.0f);

	keyboardLayoutContainer->getTransform()->setLocalPosition(glm::vec3(0.0f, keyboardLocalY, 0.0f));
	keyboardLayoutContainer->getTransform()->setLocalScale(glm::vec3(effectiveDisplayWidth, keyboardRelativeHeight, 1.0f));
	GameObject* keyboardContainerPtr = m_displayContainer->addChild(std::move(keyboardLayoutContainer));


	float numRows = 4;
	float numCols = 3;
	float keyGridWidth = 1.0f;
	float keyGridHeight = 1.0f;

	float keyGap = 0.02f;

	float totalHorizontalGaps = (numCols - 1) * keyGap;
	float totalVerticalGaps = (numRows - 1) * keyGap;

	float availableWidthForButtons = keyGridWidth - totalHorizontalGaps;
	float availableHeightForButtons = keyGridHeight - totalVerticalGaps;

	float actualButtonWidth = availableWidthForButtons / numCols;
	float actualButtonHeight = availableHeightForButtons / numRows;

	struct KeypadButtonData {
		std::string name;
		std::string text;
		std::function<void()> action;
	};

	std::vector<KeypadButtonData> keypadButtons = {
		{"Keypad_1Button", "1", [this]() { m_microwave.inputNumber(1); updateTimerDisplay(); }},
		{"Keypad_2Button", "2", [this]() { m_microwave.inputNumber(2); updateTimerDisplay(); }},
		{"Keypad_3Button", "3", [this]() { m_microwave.inputNumber(3); updateTimerDisplay(); }},
		{"Keypad_4Button", "4", [this]() { m_microwave.inputNumber(4); updateTimerDisplay(); }},
		{"Keypad_5Button", "5", [this]() { m_microwave.inputNumber(5); updateTimerDisplay(); }},
		{"Keypad_6Button", "6", [this]() { m_microwave.inputNumber(6); updateTimerDisplay(); }},
		{"Keypad_7Button", "7", [this]() { m_microwave.inputNumber(7); updateTimerDisplay(); }},
		{"Keypad_8Button", "8", [this]() { m_microwave.inputNumber(8); updateTimerDisplay(); }},
		{"Keypad_9Button", "9", [this]() { m_microwave.inputNumber(9); updateTimerDisplay(); }},
		{"Keypad_StartButton", "START", [this]() { m_microwave.startCooking(); }},
		{"Keypad_0Button", "0", [this]() { m_microwave.inputNumber(0); updateTimerDisplay(); }},
		{"Keypad_StopButton", "STOP", [this]() { m_microwave.stopCooking(); updateTimerDisplay(); }}
	};

	float keypadTextPixelSize = 48.0f;

	for (size_t i = 0; i < keypadButtons.size(); ++i) {
		KeypadButtonData& buttonData = keypadButtons[i];

		auto currentKeyContainer = std::make_unique<GameObject>(buttonData.name);

		MeshComponent* keyContainerMeshComp = currentKeyContainer->addComponent<MeshComponent>();
		RenderComponent* keyContainerRenderComp = currentKeyContainer->addComponent<RenderComponent>(basicShader);
		keyContainerMeshComp->getMesh()->generateQuad2D();
		keyContainerRenderComp->setMesh(keyContainerMeshComp->getMesh());
		keyContainerRenderComp->setObjectColor(keypadButtonBackgroundColor);
		keyContainerRenderComp->setTexture(nullptr);


		int row = i / (int)numCols;
		int col = i % (int)numCols;

		float localX = -keyGridWidth / 2.0f + (col * (actualButtonWidth + keyGap)) + (actualButtonWidth / 2.0f);
		float localY = keyGridHeight / 2.0f - (row * (actualButtonHeight + keyGap)) - (actualButtonHeight / 2.0f);


		currentKeyContainer->getTransform()->setLocalPosition(glm::vec3(localX, localY, 0.0f));
		currentKeyContainer->getTransform()->setLocalScale(glm::vec3(actualButtonWidth, actualButtonHeight, 1.0f));

		GameObject* addedKeyContainer = keyboardContainerPtr->addChild(std::move(currentKeyContainer));

		ClickableComponent* clickable = addedKeyContainer->addComponent<ClickableComponent>(PickingMethod::Method2D);
		clickable->setOnClickCallback(buttonData.action); 
		clickable->setOnHoverEnterCallback([buttonData, keyContainerRenderComp]() {
			std::cout << "Hovering over " << buttonData.text << " button!" << std::endl;
			if (keyContainerRenderComp) {
				keyContainerRenderComp->setObjectColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)); 
			}
			});
		clickable->setOnHoverExitCallback([keyContainerRenderComp]() {
			if (keyContainerRenderComp) { 
				keyContainerRenderComp->setObjectColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); 
			}
			std::cout << "Exited hover from button." << std::endl;
			});


		auto keyTextGameObject = std::make_unique<GameObject>("Keypad_" + buttonData.text + "Text");
		MeshComponent* keyTextMeshComp = keyTextGameObject->addComponent<MeshComponent>();
		RenderComponent* keyTextRenderComp = keyTextGameObject->addComponent<RenderComponent>(basicShader);
		keyTextMeshComp->getMesh()->generateQuad2D();
		keyTextRenderComp->setMesh(keyTextMeshComp->getMesh());

		std::unique_ptr<Texture> keyTextTexture = m_fontRenderer->GenerateTextTexture(
			"res/fonts/Roboto-Regular.ttf",
			buttonData.text, 
			static_cast<int>(keypadTextPixelSize)
		);

		if (keyTextTexture) {
			keyTextRenderComp->setTexture(std::shared_ptr<Texture>(std::move(keyTextTexture)));
			keyTextRenderComp->setObjectColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		}
		else {
			std::cerr << "WARNING: Failed to generate text texture for key '" << buttonData.text << "' using FontRenderer::GenerateTextTexture." << std::endl;
			keyTextRenderComp->setTexture(nullptr);
			keyTextRenderComp->setObjectColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		}
		float effectiveKeypadButtonWidth = 1.0f - (2 * keypadButtonPadding);
		float effectiveKeypadButtonHeight = 1.0f - (2 * keypadButtonPadding);

		keyTextGameObject->getTransform()->setLocalPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		keyTextGameObject->getTransform()->setLocalScale(glm::vec3(effectiveKeypadButtonWidth, effectiveKeypadButtonHeight, 1.0f));
		addedKeyContainer->addChild(std::move(keyTextGameObject));
	}

	indicatorMeshComp = runningIndicator->addComponent<MeshComponent>();
	indicatorMeshComp->getMesh()->generateQuad2D();
	indicatorRenderComp = runningIndicator->addComponent<RenderComponent>(basicShader);
	indicatorRenderComp->setMesh(indicatorMeshComp->getMesh());
	indicatorRenderComp->setObjectColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	indicatorRenderComp->setTexture(nullptr);

	float indicatorDesiredHeightRatioOfDisplay = 0.15f;
	float paddingBelowTimer = 0.015f;

	float timerBottomEdgeY = timerLocalY - (timerRelativeHeight / 2.0f);

	float indicatorLocalY = timerBottomEdgeY - (indicatorDesiredHeightRatioOfDisplay / 2.0f) - paddingBelowTimer;

	float actualHeightInMicrowaveSpace = indicatorDesiredHeightRatioOfDisplay * commonLocalRelativeHeight;
	float indicatorLocalXScale = actualHeightInMicrowaveSpace / displayLocalRelativeWidth;

	runningIndicator->getTransform()->setLocalPosition(glm::vec3(0.0f, indicatorLocalY, 0.0f));
	runningIndicator->getTransform()->setLocalScale(glm::vec3(0.2f, 0.1f, 1.0f));
	m_runningStateIndicator = m_displayContainer->addChild(std::move(runningIndicator));
	m_runningStateIndicatorRenderComponent = indicatorRenderComp;
}