# Weather App

##Usage
1.Input the name of a city and press "search" or hit enter.
2. There current weather data for the city will be displayed.
3.Add the city to your favorites if you wish, by pressing the "Add to favorites" button.
4. View your favorite cities by pressing the "Favorites" button.
5. Remove a city from the favorites list by pressing "Remove".

This is a weather application built using C++ and Dear ImGui for the GUI. The application fetches weather data for a specified city from the OpenWeatherMap API and displays it along with weather icons.

## Features

- Search for the current weather of any city.
- Display weather data including temperature, humidity, wind speed, and weather description.
- Add cities to a favorites list and quickly view or remove them.

## Requirements

- C++ compiler
- OpenGL
- GLFW
- Dear ImGui
- JSON for Modern C++
- httplib

## Building and Running
g++ -std=c++11 -I path_to_glfw_include -I path_to_imgui_include -I path_to_json_include -I path_to_httplib_include main.cpp -L path_to_glfw_lib -lglfw -lGL -o weather_app

Run the executable:
./weather_app

### Prerequisites

Ensure you have the following libraries and tools installed:

- [GLFW](https://www.glfw.org/)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [JSON for Modern C++](https://github.com/nlohmann/json)
- [httplib](https://github.com/yhirose/cpp-httplib)

### Building

1. Clone the repository:

```bash
git clone https://github.com/Tomer-Levi99/Weather-App.git
cd your-repository
