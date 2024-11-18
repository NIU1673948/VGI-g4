#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include "stdafx.h"
#include "material.h"
#include "visualitzacio.h"
#include "objLoader.h"	

using namespace std;
using namespace glm;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float MARGIN = 30;
const float STEP = 5;
const float SPEED = 5;
const float CAR_WIDTH = 50;
const float LANE_WIDTH = CAR_WIDTH + MARGIN;
const float CAR_HEIGHT = 100;
const float PLAYER_SPEED = 5;
const int NUM_LANES = 4;
const int NUM_ROWS = 4;
const float ROW_SPACING = 300;
const int VERTICAL_NOISE = ROW_SPACING / 2 - CAR_HEIGHT; // Hi ha lloc per a dos cotxes entre files
const float OBSTACLE_SPEED = 3;
const float ROTATION_ANGLE = 30 * (3.14159265f / 180.f);
const float ROTATION_SPEED = 3 * (3.14159265f / 180.f);
const int NUM_CIRCLES = 3;
const float COLLISION_START = WINDOW_HEIGHT / 4;
const int MIN_CARS = 1;
const int PROB_OBJECT = 10;
const int PROB_SHIELD = 4;


const float ROAD_WIDTH = LANE_WIDTH * NUM_LANES;
const float ROAD_LENGTH = 500.0f;
const float ROAD_START = (WINDOW_WIDTH - ROAD_WIDTH) / 2;
const float ROAD_END = ROAD_START + ROAD_WIDTH;


//const std::vector<sf::Color> carColors = {
//    sf::Color(0, 95, 115),
//    sf::Color(10, 147, 150),
//    sf::Color(148, 210, 189),
//    sf::Color(238, 155, 0),
//    sf::Color(187, 62, 3),
//    sf::Color(174, 32, 18),
//    sf::Color(154, 34, 38)
//};
//
//sf::Color getRandomColor();

class Car {
public:
    Car(float x = CAR_WIDTH / 2, float y = CAR_HEIGHT / 2,
        float w = CAR_WIDTH, float h = CAR_HEIGHT, float speed = SPEED);
    void move(float dx, float dy);
    void draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const;

    float m_x, m_y;
    float m_height, m_width;
    float m_speed;
    COBJModel* m_model;
    bool m_visible;
};

class Circle {
public:
    Circle(float x = 0.0, float y = 0.0, float radius = LANE_WIDTH / 4, bool visible = true)
        : m_x(x), m_y(y), m_radius(radius), m_visible(visible), m_isCoin(true) {
    }
    //void draw(sf::RenderWindow& window) const;

    float m_x, m_y, m_radius;
    //sf::Color m_color;
    bool m_visible;
    bool m_isCoin;
};

class Player : public Car {
public:
    Player();
    void move(float dx, float dy);
    void rotate(float dAngle);
    void draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const;
    bool checkCollision(const Car& obstacle) const;
    bool checkCollision(const Circle& object) const;


    float m_rotation;
    Circle m_collisionCircles[NUM_CIRCLES];
};

class Road {
public:
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    Road();
    ~Road();
    void setRoadSize(const float& width, const float& length);
    void setupMesh();
    void draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const;


};

enum PickUp {
    NONE,
    COIN,
    SHIELD
};

class RoadRow {
public:
    RoadRow();
    bool checkCollision(const Player& player) const;
    void move(float dy);
    float getY() const;
    void draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const;
    void initRow(float y, int& nextEmptyLane);
    PickUp getPickUp(const Player& player) const;

    Car m_obstacles[NUM_LANES];
    Circle m_object;
};

class GameLogic {
private:
    void UpdateRoadRows();
    void DoCollisions();
    void DoPickUps();

public:
    GameLogic();
    void GetUserInput();
    void UpdateGameLogic();
    void draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const;

    bool gameRunning;
    int nextEmptyLane;
    Player player;
    RoadRow roadRows[NUM_ROWS];
    Road road;
};
