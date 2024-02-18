#include <iostream>
#include <memory>
#include <fstream>

#include "SFML/Graphics.hpp"
#include "imgui.h"
#include "imgui-SFML.h"

enum ShapeType
{
    Circle,
    Rectangle
};

struct Shape
{
    ShapeType type;
    std::string name;
    float x, y;
    float vx, vy;
    int r, g, b;
    float radius;
    float width, height;
    std::shared_ptr<sf::Shape> shape;
};


int main(int argc, char* argv[])
{
    const std::string filename = "config.txt";

    std::ifstream fin(filename);
    if (!fin.is_open())
    {
        std::cerr << "Could not load config" << "\n";
        exit(-1);
    }

    std::string settingType;
    // window
    int wWidth{};
    int wHeight{};
    // font
    std::string fontPath{ };
    unsigned int fontSize{ };
    float font_r{}, font_g{}, font_b{};

    std::vector<Shape> shapes{};

    if (fin.is_open())
    {
        while (fin >> settingType)
        {
            
            if (settingType == "Window")
            {
                fin >> wWidth >> wHeight;
            }
            else if (settingType == "Font")
            {
                fin >> fontPath >> fontSize >> font_r, font_g, font_b;

                // ImGui takes rgb values as floats from 0 - 1.0f
                if (font_r != 0) {
                    font_r /= 255;
                }
                if (font_g != 0) {
                    font_g /= 255;
                }
                if (font_b != 0) {
                    font_b /= 255;
                }
            }
            else if (settingType == "Circle")
            {
                
                std::string name{};
                float x_pos{}, y_pos{};
                float x_vel{}, y_vel{};
                int r{}, g{}, b{};
                float radius{};

                fin >> name >> x_pos >> y_pos >> x_vel >> y_vel >> r >> g >> b >> radius;
                auto shape = std::make_shared<sf::CircleShape>(radius);
                shape->setPosition(x_pos, y_pos);
                shape->setFillColor(sf::Color(r, g, b));

                shapes.push_back({ ShapeType::Circle, name, x_pos, y_pos, x_vel, y_vel, r, g, b, radius, 0, 0, shape });

            }
            else if (settingType == "Rectangle")
            {
                std::string name{};
                float x_pos{}, y_pos{};
                float x_vel{}, y_vel{};
                int r{}, g{}, b{};
                float width{}, height{};

                fin >> name >> x_pos >> y_pos >> x_vel >> y_vel >> r >> g >> b >> width >> height;
                auto shape = std::make_shared<sf::RectangleShape>(sf::Vector2<float>(width, height));
                shape->setPosition(x_pos, y_pos);
                shape->setFillColor(sf::Color(r, g, b));

                shapes.push_back({ ShapeType::Circle, name, x_pos, y_pos, x_vel, y_vel, r, g, b, 0, width, height, shape });

            }
        }
        fin.close();
    }

    sf::RenderWindow window(sf::VideoMode(wWidth, wHeight), "Window Title");
    window.setFramerateLimit(60); 

    // Initialize ImGUI and create a clock used for internal timing
    ImGui::SFML::Init(window);
    sf::Clock deltaClock;

    ImGui::GetStyle().ScaleAllSizes(1.0f);

    // ImGui color {r, g, b} wheel requires floats from 0 - 1.0 instead of ints 0-255
    float c[3] = { 0.0f, 1.0f, 1.0f };

    float circleRadius = 50;
    int circleSegments = 32;
    float circleSpeedX = 1.0f;
    float circleSpeedY = 0.5f;
    bool drawCircle = true;
    bool drawText = true;

    sf::CircleShape circle(circleRadius, circleSegments );
    circle.setPosition(10.0f, 10.0f);


    sf::Font myFont;

    if (!myFont.loadFromFile(fontPath))
    {
        std::cerr << "Could not load font from file!\n";
        exit(-1);
    }

    sf::Text text{ "Sample text", myFont, fontSize };

    text.setPosition(0, wHeight - (float)text.getCharacterSize());

    char displayString[255] = "Sample text!";

    std::shared_ptr<Shape> selectedShape = nullptr;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed)
            {
                std::cout << "Key pressed with code: " << event.key.code << "\n";

                if (event.key.code == sf::Keyboard::X)
                {
                    circleSpeedX *= -1.0f;
                }
            }

            if (event.type == sf::Event::MouseButtonPressed)
            {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);

                selectedShape = nullptr;
                for (auto& s : shapes)
                {
                    if (s.shape->getGlobalBounds().contains(worldPos))
                    {
                        selectedShape = std::make_shared<Shape>(s);
                        break;
                    }
                }
            }
        }
        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Window title");
        ImGui::Text("Window text!");
        ImGui::Checkbox("Draw circle", &drawCircle);
        ImGui::SameLine();
        ImGui::Checkbox("Draw text", &drawText);
        ImGui::SliderFloat("Radius", &circleRadius, 0.0f, 300.0f);
        ImGui::SliderInt("Sides", &circleSegments, 3, 64);
        ImGui::ColorEdit3("Color circle", c);
        ImGui::InputText("Text", displayString, 255);
        if (ImGui::Button("Set Text"))
        {
            text.setString(displayString);
        }
        ImGui::SameLine();
        ImGui::End();
        
        circle.setPointCount(circleSegments);
        circle.setRadius(circleRadius);

        circle.setPosition(
            circle.getPosition().x + circleSpeedX,
            circle.getPosition().y + circleSpeedY
        );
        

        window.clear(); // Color background

        for (auto& s : shapes)
        {
            // Origin (0,0) of shape is in top left corner of box
            float originX = s.shape->getPosition().x + s.vx;
            float shapeWidth = s.shape->getLocalBounds().width;

            float originY = s.shape->getPosition().y + s.vy;
            float shapeHeight = s.shape->getLocalBounds().height;

            if (originX < 0 || (originX + shapeWidth) > wWidth)
            {
                s.vx *= -1.0f;
            }
            if (originY < 0 || (originY + shapeHeight) > wHeight)
            {
                s.vy *= -1.0f;
            }
  
            s.shape->setPosition(
                s.shape->getPosition().x + s.vx,
                s.shape->getPosition().y + s.vy
            );


            s.shape->setFillColor(sf::Color(s.r, s.g, s.b));
            //s.shape->setPointCount(circleSegments);
            window.draw(*s.shape);
        }

        if (drawCircle)
        {
            window.draw(circle);
        }
        if (drawText)
            window.draw(text);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}