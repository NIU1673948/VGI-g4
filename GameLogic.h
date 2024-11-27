#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <string>
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
const float CAR_WIDTH = 40;
const float LANE_WIDTH = 70;
const float CAR_HEIGHT = 100;
const float PLAYER_SPEED = 5;
const int NUM_LANES = 4;
const int NUM_ROWS = 15;
const float ROW_SPACING = 300;
const int VERTICAL_NOISE = ROW_SPACING / 2 - CAR_HEIGHT; // Hi ha lloc per a dos cotxes entre files
const float OBSTACLE_SPEED = 3;
const float ROTATION_ANGLE = 30 * (3.14159265f / 180.f);
const float ROTATION_SPEED = 3 * (3.14159265f / 180.f);
const float COLLISION_START = WINDOW_HEIGHT / 5;
const int MIN_CARS = 1;
const int PROB_OBJECT = 30; //%

const int PROB_FUEL = 30; //%
const int PROB_SHIELD = 10; //%

const float MODEL_WIDTH = 3.2; //3.4 original

const float TARGET_FPS = 60.0f;      // Freq��ncia de la l�gica
const float FRAME_TIME = 1.0f / TARGET_FPS;

const float COLLISION_TOLERANCE = 5;

const float ROAD_WIDTH = LANE_WIDTH * NUM_LANES;
const float ROAD_START = 0;
const float ROAD_END = ROAD_START + ROAD_WIDTH;

extern vector<COBJModel*> CAR_MODELS;
const int NUM_CAR_MODELS = 8;

extern COBJModel* OBJECT_MODELS[3];

const float ROTATION_VEL = 0.05;



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

void dibuixaPla(GLuint sh_programID, const glm::mat4& MatriuVista, const glm::mat4& MatriuTG, float width, float length);


class Car {
public:
    Car(float x = CAR_WIDTH / 2, float y = CAR_HEIGHT / 2, float speed = PLAYER_SPEED / 4,
        float w = CAR_WIDTH, float h = CAR_HEIGHT);
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
    Circle(float x = 0.0, float y = 0.0, float radius = LANE_WIDTH / 5)
        : m_x(x), m_y(y), m_radius(radius) {
    }

    float m_x, m_y, m_radius;
};

enum PickUp {
    COIN,
    FUEL,
    SHIELD
};

class Object : public Circle {
public:
    Object() : m_type(COIN), m_visible(false), m_model(NULL), m_rotation(0) {}
    void draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const;

    PickUp m_type;
    float m_rotation;
    bool m_visible;
    Circle m_circle;
    COBJModel* m_model;
};

class Player : public Car {
public:
    Player();
    void move(float dx, float dy);
    void rotate(float dAngle);
    void draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const;
    bool checkCollision(const Car& obstacle) const;
    bool checkCollision(const Object& object) const;

    float m_rotation;
    Circle m_collisionCircles[3];
};

class RoadRow {
public:
    RoadRow();
    bool checkCollision(const Player& player) const;
    void move(float dy);
    float getY() const;
    void draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const;
    void initRow(float y, int& nextEmptyLane);

    Car m_obstacles[NUM_LANES];
    Object m_object;
};

class GameLogic {
private:
    void UpdateRoadRows();
    void DoCollisions();
    void DoPickUps();
    COBJModel* m_road;
    float m_roadY;
    void dibuixaRoad(GLuint sh_programID, const glm::mat4 MatriuVista, const glm::mat4 MatriuTG) const;

public:
    GameLogic();
    void GetUserInput();
    void UpdateGameLogic();
    void draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const;

    bool gameRunning;
    int nextEmptyLane;
    Player player;
    RoadRow roadRows[NUM_ROWS];
};