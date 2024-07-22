//315176347
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include "httplib.h"
#include "json.hpp" // Include the JSON library
#include <iomanip>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

//api key=dc7844f9e5a24d3427638b0916e59911
// Function to fetch weather data
std::string fetchWeatherData(const std::string& city) {
    httplib::Client cli("api.openweathermap.org");
    std::string apiKey = "dc7844f9e5a24d3427638b0916e59911"; // Replace with your API key

    // Fetch the current weather to get the parameters
    std::string path = "/data/2.5/weather?q=" + city + "&appid=" + apiKey;
    auto res = cli.Get(path);

    if (!res) {
        return "Failed to connect to the API.";
    }

    if (res->status != 200) {
        return "Invalid City Name/Name is not in the registry, please try again.";
    }

    auto json = nlohmann::json::parse(res->body);

    // Extract weather data
    double temperatureKelvin = json["main"]["temp"].get<double>(); // Temperature in Kelvin
    double temperatureCelsius = temperatureKelvin - 273.15; // Convert to Celsius
    double humidity = json["main"]["humidity"].get<double>(); // Humidity in percentage
    double windSpeed = json["wind"]["speed"].get<double>(); // Wind speed in meters per second
    std::string description = json["weather"][0]["description"].get<std::string>(); // Weather description

    // Format the result
    std::ostringstream result;
    result << "Current Weather in " << city << ":\n";
    result << "Temperature: " << std::fixed << std::setprecision(1) << temperatureCelsius << " °C\n";
    result << "Humidity: " << humidity << "%\n";
    result << "Wind Speed: " << windSpeed << " m/s\n";
    result << "Weather: " << description << "\n";

    return result.str();
}

int main(){
    //Initialize GLFW
    if(!glfwInit()){
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    //create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(1440,720,"Weather App", nullptr, nullptr);
    if(!window){
        std::cerr<< "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);//enable vsync

    //setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsLight();

    //setup platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL3_Init("#version 130");

    //Variables for managing state
    std::string city;
    std::string weatherData;
    std::vector<std::string> favoriteCities;
    bool showFavoritesWindow = false;
    std::string selectedCity;
    bool showCityOptions = false;
    std::thread cityThread;
    std::mutex mtx;
    bool threadRunning = false;
    std::string statusMessage;
    float statusMessageTime = 0.0f;

    //main loop
    while(!glfwWindowShouldClose(window)){
        //poll and handle events
        glfwPollEvents();

        //start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //adjust font size
        ImGui::GetIO().FontGlobalScale = 2.0f;


        //create ImGui window
        ImGui::SetNextWindowPos(ImVec2(0,0),ImGuiCond_Always);//set the window position to the top left corner always
        ImGui::SetWindowSize(ImVec2(640,720),ImGuiCond_Always);//set window size
        ImGui::Begin("Weather App", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        //center the input line and place buttons
        ImGui::PushItemWidth(300.0f);

        static char cityBuffer[256] = "";
        ImGui::InputTextWithHint("##CityInput", "Input city name", cityBuffer, IM_ARRAYSIZE(cityBuffer),ImGuiInputTextFlags_EnterReturnsTrue);

        if(ImGui::IsItemDeactivatedAfterEdit()){//if "enter" key is pressed
            city=cityBuffer;
            weatherData= fetchWeatherData(city);
            memset(cityBuffer,0, sizeof(cityBuffer));//clear the input field
        }

        ImGui::SameLine();//place the button next to the input field
        if(ImGui::Button("Search")){
            city = cityBuffer;
            weatherData = fetchWeatherData(city);
            memset(cityBuffer,0, sizeof(cityBuffer));//clear the input field
        }

        ImGui::SameLine();//place the clear button next to the search button
        if(ImGui::Button("Clear")) {
            //clear the displayed data
            weatherData.clear();
            memset(cityBuffer,0,sizeof(cityBuffer));//clear the input field
            city.clear();
        }

        ImGui::SameLine();
        if(ImGui::Button("Add to Favorites")){//place the favorites button next to the search button
            if(city.empty()){
                statusMessage = "Please input a City.";
                statusMessageTime = 3.0f;//show message for 3 seconds
            }else if (std::find(favoriteCities.begin(), favoriteCities.end(), city) == favoriteCities.end()) {
                favoriteCities.push_back(city);
                statusMessage = "City added to favorites.";
                statusMessageTime = 3.0f; // Show message for 3 seconds
            }
        }

        ImGui::SameLine();//place in the same line
        if(ImGui::Button("Favorites")){
            showFavoritesWindow= true;
        }

        //Display weather data
        ImGui::Text("%s",weatherData.c_str());

        //display status message
        if(statusMessageTime>0.0f){
            ImGui::TextColored(ImVec4(1,0,0,1), "%s", statusMessage.c_str());
            statusMessageTime -= ImGui::GetIO().DeltaTime;
        }

        ImGui::End();


        //display favorites window
        if(showFavoritesWindow){
            ImGui::SetNextWindowPos(ImVec2(885,0),ImGuiCond_Always);//position favorites list next to the main window
            ImGui::SetWindowSize(ImVec2(640,720),ImGuiCond_Always);

            ImGui::Begin("Favorites", &showFavoritesWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);//pass reference to the bool to enable closing the window

            ImGui::Text("Favorites cities:");
            for(const auto&favCity : favoriteCities){
                ImGui::BulletText("%s",favCity.c_str());
                ImGui::SameLine();

                if (ImGui::Button(("Select##" + favCity).c_str()) && !threadRunning){
                    selectedCity = favCity;
                    showCityOptions=true;
                    threadRunning= true;
                    cityThread=std::thread([&]{
                        std::lock_guard<std::mutex> lock(mtx);
                        //thread runs until View, Remove, or Back is pressed
                        while(showCityOptions){
                            //do nothing, just keep the thread alive
                        }
                        threadRunning=false;
                    });
                    cityThread.detach();
                }

                if(showCityOptions && selectedCity==favCity){
                    ImGui::SameLine();
                    if(ImGui::Button("View")){
                        weatherData = fetchWeatherData(favCity);
                        showCityOptions = false;
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("Remove")){
                        favoriteCities.erase(std::remove(favoriteCities.begin(),favoriteCities.end(),favCity),favoriteCities.end());
                        statusMessage = favCity +  " Removed from favorites.";
                        statusMessageTime =2.0f;
                        showCityOptions=false;
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("Back")){
                        showCityOptions = false;
                    }
                }
            }
            if(ImGui::Button("close")){
                showFavoritesWindow=false;
            }

            ImGui::End();
        }
        //Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Cleanup
    if(threadRunning){
        showCityOptions=false;
        cityThread.join();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}