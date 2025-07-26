/*
    Author: Harish Babu
*/

#include "imgui.h"
#include "imgui-SFML.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <windows.h>
#include <algorithm>
#include <cctype>

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 800;
const double GRAV_CONST = 6.6743e-11;

class Object {
private:
public:
    sf::CircleShape shape;
    sf::Vector2f direction;
    sf::Vector2f acceleration;

    float mass;
    float radius;
    int color_r, color_g, color_b;

    std::string obj_name;

    Object(std::string _name, float _radius, sf::Color _color, sf::Vector2f init_pos, float _mass, sf::Vector2f init_dir = sf::Vector2f(0, 0)) {
        radius = _radius;
        
        color_r = _color.r;
        color_b = _color.b;
        color_g = _color.g;

        shape.setFillColor(_color);
        shape.setRadius(_radius);
        shape.setPosition(init_pos);
        shape.setOrigin(sf::Vector2f(shape.getRadius(), shape.getRadius()));

        mass = _mass;
        obj_name = _name;
        direction = init_dir;
        acceleration = sf::Vector2f(0, 0.98f);
    }


    void UpdateObjAppearance() {
        shape.setRadius(radius);
        shape.setFillColor(sf::Color(color_r, color_g, color_b));
    }

    void ApplyMotion(float deltaTime) {
        UpdateObjAppearance();

        shape.move(direction * deltaTime);
        direction += acceleration;

        //std::cout << obj_name << " " << shape.getPosition().x << " " << shape.getPosition().y << std::endl;
    }

    void ApplyUniversalGravity(std::vector<Object> &_objects) {
        sf::Vector2f resultant_force = sf::Vector2f(0, 0);

        for (int i = 0; i < _objects.size(); i++) {
            if (_objects[i].obj_name != obj_name) {
                sf::Vector2f direction_force = _objects[i].shape.getPosition() - shape.getPosition();

                double dir_mag = direction_force.length();
                float force_magnitude = (GRAV_CONST * mass * _objects[i].mass) / (dir_mag * dir_mag * dir_mag);

                sf::Vector2f force_vector_for_pair = force_magnitude * direction_force;

                resultant_force += force_vector_for_pair;
            }
        }

        acceleration = resultant_force / (float)mass;
    }

    void UpdateGui(std::vector<Object> &_objects) {
        if (obj_name.empty()) return;
        bool broken = false;

        ImGui::Begin(obj_name.c_str());

        ImGui::SeparatorText("Edit Properties");
        ImGui::SliderFloat("Radius", &radius, 1.0f, 50.0f);
        ImGui::Text("\n");

        ImGui::SeparatorText("Color - RGB");
        ImGui::SliderInt("Red", &color_r, 0, 255);
        ImGui::SliderInt("Green", &color_g, 0, 255);
        ImGui::SliderInt("Blue", &color_b, 0, 255);
        ImGui::Text("\n");

        ImGui::SeparatorText("Mass");
        ImGui::SliderFloat(" ", &mass, 10.0f, 100000000000000.0f);
        ImGui::Text("\n");

        static int clicked = 0;

        if (ImGui::Button("Delete")) {
            clicked++;
        }
        if (clicked & 1) {
            for (int i = 0; i < _objects.size(); i++) {
                if (_objects.size() == 1) {
                    ImGui::SameLine();
                    ImGui::Text("At Least one Object has to exist in the world");
                    break;
                }

                if (_objects[i].obj_name == obj_name) {
                    std::cout << "Erase for: " << i << " | Size: " << _objects.size() << std::endl;
                    _objects.erase(_objects.begin() + i);
                    broken = true;
                    ImGui::End();
                    break;
                }
            }
            clicked = 0;
        }

        if (!broken) ImGui::End();
    }
};


void ShowHelpGui(float* move_scr_speed) {
    ImGui::Begin("How to move arround");

    ImGui::Text("Move Arround - Arrow Keys (or) WASD");
    ImGui::Text("Zoom in and out - Mouse wheel");

    ImGui::SeparatorText("Simulator Settings");
    ImGui::SliderFloat("Screen Move Speed", move_scr_speed, 10.0f, 100.0f);

    ImGui::End();
}

std::string trim_erase_remove(std::string str) {
    // Trim leading spaces
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
        }));

    // Trim trailing spaces
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
        }).base(), str.end());

    return str;
}

void ShowAddGui(std::vector<Object> &_objects) {
    ImGui::Begin("Add New Object");

    static char _name[128];
    ImGui::SeparatorText("Object Name (max 256 chars)");
    ImGui::InputText(" ", _name, IM_ARRAYSIZE(_name));

    static float _radius = 10.0f;
    ImGui::SeparatorText("Object Radius");
    ImGui::SliderFloat("  ", &_radius, 1.0f, 50.0f);

    static int c_r = 100, c_g = 100, c_b = 100;
    ImGui::SeparatorText("Object Color - RGB");
    ImGui::SliderInt("Red", &c_r, 0.0f, 255.0f);
    ImGui::SliderInt("Green", &c_g, 0.0f, 255.0f);
    ImGui::SliderInt("Blue", &c_b, 0.0f, 255.0f);

    static float _mass = 10000000;
    ImGui::SeparatorText("Mass of Object");
    ImGui::SliderFloat("Mass", &_mass, 10.0f, 100000000000000.0f);

    static float init_pos_x = 500.f, init_pos_y = 500.f;
    ImGui::SeparatorText("Initial Position");
    ImGui::InputFloat("X", &init_pos_x, 1.0f, 10.f);
    ImGui::InputFloat("Y", &init_pos_y, 1.0f, 10.f);

    static float init_vel_x = 0, init_vel_y = -60.0f;
    ImGui::SeparatorText("Initial Velocity vector");
    ImGui::InputFloat("X ", &init_vel_x, 1.0f, 10.f);
    ImGui::InputFloat("Y ", &init_vel_y, 1.0f, 10.f);

    static int clicked = 0;
    if (ImGui::Button("Create")) {
        clicked++;
    }
    if (clicked & 1) {
        ImGui::SameLine();

        std::string obj_name_inp = _name;
        obj_name_inp = trim_erase_remove(obj_name_inp);
        bool can_create = true;

        if (obj_name_inp.empty()) {
            ImGui::Text("Please enter a name");
            can_create = false;
        }

        for (Object obj: _objects) {
            if (obj.obj_name == obj_name_inp && !obj_name_inp.empty()) {
                ImGui::Text("This name, already exists");
                can_create = false;
            }
        }

        if (can_create) {
            Object new_obj(
                obj_name_inp,
                _radius,
                sf::Color(c_r, c_g, c_b),
                sf::Vector2f(init_pos_x, init_pos_y),
                _mass,
                sf::Vector2f(init_vel_x, init_vel_y)
            );

            _objects.push_back(new_obj);

            memset(_name, '\0', sizeof(_name));
        }

        clicked = 0;
    }

    ImGui::End();
}

void showCoordinatesTooltipGui(sf::Vector2f worldMousePos) {
    if (!ImGui::IsAnyItemHovered() && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
        ImGui::BeginTooltip();
        ImGui::Text("Coordinates");
        ImGui::Separator();
        ImGui::Text("X: %.2f", worldMousePos.x);
        ImGui::Text("Y: %.2f", worldMousePos.y);
        ImGui::EndTooltip();
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode({ SCREEN_WIDTH, SCREEN_HEIGHT }), "Gravity Simulation! By Harish Babu");
    ::ShowWindow(window.getNativeHandle(), SW_MAXIMIZE);
    ImGui::SFML::Init(window);

    // set icon
    sf::Image icon;
    icon.loadFromFile("images/gravity-icon-1.jpg");
    window.setIcon(icon);

    // define objects
    double some_mass = 10000000000000;
    float init_speed = 25.f;
    float some_radius = 15.0f;

    Object obj1(
        "obj1",
        some_radius,
        sf::Color(50, 100, 255),
        sf::Vector2f(500.0f, 500.0f),
        some_mass,
        sf::Vector2f(0, init_speed)
    );

    Object obj2(
        "obj2",
        some_radius,
        sf::Color(255, 0, 255),
        sf::Vector2f(800.0f, 600.0f),
        some_mass,
        sf::Vector2f(0, -init_speed)
    );

    // Store Objects as a dynamic array;
    std::vector<Object> objects = { obj1, obj2 };
    
    sf::Clock clock;
    sf::Clock imClock;

    float deltaTime = 0.0f;
    float move_screen_speed = 60.0f;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
                window.close();

            // Window Resize
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::FloatRect visibleArea({ 0.f, 0.f }, sf::Vector2f(resized->size));
                window.setView(sf::View(visibleArea));
            }

            // Zooming in and out
            if (const auto* mouseEve = event->getIf<sf::Event::MouseWheelScrolled>()) {
                sf::View curreView = window.getView();
                if (mouseEve->delta > 0) {
                    curreView.zoom(0.9f);
                }
                else if (mouseEve->delta < 0 ) {
                    curreView.zoom(1.1f);
                }
                window.setView(curreView);
            }

            // Moving around Viewport
            if (const auto* keyEve = event->getIf<sf::Event::KeyPressed>()) {
                sf::Vector2f move_vec = sf::Vector2f(0,0);
                sf::View curreView = window.getView();
                
                if ((keyEve->code == sf::Keyboard::Key::Up) || (keyEve->code == sf::Keyboard::Key::W)) {
                    move_vec.y -= move_screen_speed;
                } 
                else if ((keyEve->code == sf::Keyboard::Key::Down) || (keyEve->code == sf::Keyboard::Key::S)) {
                    move_vec.y += move_screen_speed;
                }
                else if ((keyEve->code == sf::Keyboard::Key::Right) || (keyEve->code == sf::Keyboard::Key::D)) {
                    move_vec.x += move_screen_speed;
                }
                else if ((keyEve->code == sf::Keyboard::Key::Left) || (keyEve->code == sf::Keyboard::Key::A)) {
                    move_vec.x -= move_screen_speed;
                }

                curreView.move(move_vec);
                window.setView(curreView);
            }
        }

        deltaTime = clock.restart().asSeconds();
        ImGui::SFML::Update(window, imClock.restart());     

        // Some Extra GUI
        ShowHelpGui(&move_screen_speed);
        ShowAddGui(objects);

        sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
        sf::Vector2f worldMousePos = window.mapPixelToCoords(mousePixelPos);
        showCoordinatesTooltipGui(worldMousePos);

        // Update Objects
        for (Object &obj: objects) {
            obj.UpdateGui(objects);
            obj.ApplyUniversalGravity(objects);
            obj.ApplyMotion(deltaTime);
        }

        // Render Out All Stuff
        window.clear();
        for (int i = 0; i < objects.size(); i++) {
            window.draw(objects[i].shape);
        }
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}