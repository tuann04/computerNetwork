#include "UI.h"
#include "Image.h"
#include <WinSock2.h>
#include <SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "Socket.h"
#include "main.h"
#include <WS2tcpip.h>
#include <iostream>
#pragma comment (lib, "ws2_32.lib")


void broadcastS() {
    SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);
    struct timeval tv;
    tv.tv_sec = 10;  // 1 second timeout
    tv.tv_usec = 0;
    //setsockopt(in, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    sockaddr_in serverHint;
    ZeroMemory(&serverHint, sizeof(serverHint));
    serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
    serverHint.sin_family = AF_INET;
    serverHint.sin_port = htons(broadcastPort);
    if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR)
    {
        std::cout << "Can't bind socket! " << WSAGetLastError() << "\n";
        return;
    }
    sockaddr_in client;
    int clientLength = sizeof(client);
    char buf[1024];
    std::cout << "waiting for name req from client broadcast.\n";
    while (connectionState != ConnectionState::CONNECTED)
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ZeroMemory(&client, clientLength);
        ZeroMemory(buf, 1024);
        int bytesIn = recvfrom(in, buf, 1024, 0, (sockaddr*)&client, &clientLength);
        if (bytesIn == SOCKET_ERROR)
        {
            std::cout << "Error receiving from client " << WSAGetLastError() << "\n";
            continue;
        }
        char clientIp[256];
        ZeroMemory(clientIp, 256);
        inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);
        std::cout << "Message recv from " << clientIp << " : " << buf << "\n";
        char computerName[100]{ 0 };
        gethostname(computerName, 100);
        //for (int i = 0; i < 5; i++)
        sendto(in, computerName, strlen(computerName) + 1, 0, (sockaddr*)&client, clientLength);
    }
    closesocket(in);
}


void initUI() {
    const double scale = 0.8;
    int appWidth = screenWidth * scale, appHeight = screenHeight * scale;

    screenRect = { 0, 0, appWidth, appHeight };

    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, appWidth, appHeight, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontFromFileTTF("OpenSans-Regular.ttf", 24);
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
}

void freeUI() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void waitingForConnection(SOCKET& serverSocket, SOCKET& acceptSocket) {
    listenSocket(serverSocket, acceptSocket);
    connectionState = ConnectionState::CONNECTED;
    state = State::START_THREADS;
}

void displayConnectMenu() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(screenRect.w, screenRect.h));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    if (ImGui::Begin("Initialize server", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Server's IP Address");
        ImGui::InputText("##IP", ip, IM_ARRAYSIZE(ip));
        if (ImGui::Button("Initialize")) {
            if (bindSocket(imageSocket, ip, imagePort)) {
                bindingState = BindingState::BOUND;
            }
            else {
                bindingState = BindingState::FAILED;
            }
        }

        if (bindingState == BindingState::FAILED) {
            ImGui::Text("Initialize failed!");
        }

        if (bindingState == BindingState::BOUND) {
            ImGui::Text("Initialize success!");
            ImGui::Text("Waiting for client...");

            if (connectionState == ConnectionState::NOT_YET) {
                connectionState = ConnectionState::WAITING;
                std::thread waitingForConnectionThread(waitingForConnection, std::ref(imageSocket), std::ref(acceptImageSocket));
                waitingForConnectionThread.detach();

                std::thread broadcastThread(broadcastS);
                broadcastThread.join();
            }
        }
    }
    ImGui::End();

    ImGui::Render();

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    SDL_RenderPresent(renderer);
}

void displayControlPanel() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    //ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(200, 100));
    ImGui::SetNextWindowSize(ImVec2(200, 100));
    if (ImGui::Begin("Control Panel", NULL, ImGuiWindowFlags_NoResize)) {
        if (ImGui::Button("Exit")) {
            state = State::QUIT;
        }
    }
    ImGui::End();

    ImGui::Render();

    SDL_RenderClear(renderer);

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());

    SDL_RenderPresent(renderer);
}