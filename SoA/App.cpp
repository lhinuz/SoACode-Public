#include "stdafx.h"
#include "App.h"

#include "GamePlayScreen.h"
#include <InputDispatcher.h>
#include "InitScreen.h"
#include "LoadScreen.h"
#include "MainMenuScreen.h"
#include "MeshManager.h"
#include "Options.h"
#include "ScreenList.h"
#include "SpaceSystem.h"
#include "SpriteBatch.h"

void App::addScreens() {
    scrInit = new InitScreen(this);
    scrLoad = new LoadScreen(this);
    scrMainMenu = new MainMenuScreen(this);
    scrGamePlay = new GamePlayScreen(this);

    _screenList->addScreen(scrInit);
    _screenList->addScreen(scrLoad);
    _screenList->addScreen(scrMainMenu);
    _screenList->addScreen(scrGamePlay);

    _screenList->setScreen(scrInit->getIndex());
}

void App::onInit() {
    
    // Load the graphical options
    loadOptions("Data/Options.yml");

    SamplerState::initPredefined();

    // Allocate resources
    spaceSystem = new SpaceSystem;
    meshManager = new MeshManager;

    // Showcase various information
    vui::InputDispatcher::window.onFile += createDelegate<const vui::WindowFileEvent&>([=] (void* sender, const vui::WindowFileEvent& e) {
        std::cout << "Received file: " << e.file << std::endl;
    });
    vui::InputDispatcher::window.onResize += createDelegate<const vui::WindowResizeEvent&>([=] (void* sender, const vui::WindowResizeEvent& e) {
        std::cout << "Window resized: " << e.w << " x " << e.h <<std::endl;
    });
    vui::InputDispatcher::window.onClose += createDelegate<>([=] (void* sender) {
        std::cout << "Window requested close" << std::endl;
    });
    vui::InputDispatcher::onQuit += createDelegate<>([=] (void* sender) {
        std::cout << "App requested quit" << std::endl;
    });
    vui::InputDispatcher::key.onFocusGained += createDelegate<>([=] (void* sender) {
        std::cout << "Keyboard gained focus" << std::endl;
    });
    vui::InputDispatcher::key.onFocusLost += createDelegate<>([=] (void* sender) {
        std::cout << "Keyboard lost focus" << std::endl;
    });
    vui::InputDispatcher::mouse.onFocusGained += createDelegate<const vui::MouseEvent&>([=] (void* sender, const vui::MouseEvent& e) {
        std::cout << "Mouse gained focus at:" << e.x << "," << e.y << std::endl;
    });
    vui::InputDispatcher::mouse.onFocusLost += createDelegate<const vui::MouseEvent&>([=] (void* sender, const vui::MouseEvent& e) {
        std::cout << "Mouse lost focus at:" << e.x << "," << e.y << std::endl;
    });
}

void App::onExit() {
    // Delete cache if it exists

    SpriteBatch::disposeProgram();
}

App::~App() {
    if (scrInit) {
        delete scrInit;
        scrInit = nullptr;
    }

    if (scrLoad) {
        delete scrLoad;
        scrLoad = nullptr;
    }

    delete meshManager;
}