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

// Constants de la carretera
const int WINDOW_HEIGHT = 600;
const float LANE_WIDTH = 60;
const int NUM_LANES = 4;
const float ROAD_WIDTH = LANE_WIDTH * NUM_LANES;
const float ROAD_LENGTH = 2 * 63.8 * (20 + ROAD_WIDTH / 6.26);
const float ROAD_START = 0;
const float ROAD_END = ROAD_START + ROAD_WIDTH;

// Constants de l'entorn
const float HOUSE_WIDTH = 325;
const float HOUSE_HEIGHT = 1000;
const int NUM_REPEATS = 6;
const float Z_SPACE = ROAD_LENGTH / NUM_REPEATS;
const float INIT_POSITION = 200;
extern vector<COBJModel*> ENVIRONMENT_MODELS;

// Constants del jugador
const float MARGIN = 30;
const float STEP = 5;
const float PLAYER_SPEED = 5;
const float ROTATION_ANGLE = 30 * (PI / 180.f);
const float ROTATION_SPEED = 3 * (PI / 180.f);
const float ROTATION_VEL = 0.05;

// Constants cotxes obstacle
const float CAR_WIDTH = 40;
const float CAR_HEIGHT = 100;
const int MIN_CARS = 1;
const float OBSTACLE_SPEED = PLAYER_SPEED/4;
extern vector<COBJModel*> CAR_MODELS;
//const int NUM_CAR_MODELS = 8;

// Constants de files de cotxes
const float ROW_SPACING = 300;
const int NUM_ROWS = 15;
const int VERTICAL_NOISE = ROW_SPACING / 2 - CAR_HEIGHT; // Hi ha lloc per a dos cotxes entre files

// Constants de col�lisions
const float COLLISION_START = WINDOW_HEIGHT / 5;
const float COLLISION_TOLERANCE = 5;

// Constants d'objectes
const int PROB_OBJECT = 40; //%
const int PROB_FUEL = 40; //%
const int PROB_SHIELD = 20; //%
const float FUEL_DURATION = 30;
const float SHIELD_DURATION = 5;
const float COIN_SPEED_DOWN = 0.5;
const float COIN_SCORE = 100;
extern COBJModel* OBJECT_MODELS[3];

// Constants de la moneda final
const float ANIMATION_DURATION = 2; //segons

// Constants de visualitzaci�
const float TARGET_FPS = 60.0f;      // Freq��ncia de la l�gica
const float FRAME_TIME = 1.0f / TARGET_FPS;




enum CARS {
    CAR1,
    CAR1BLUE,
    CAR1RED,
    CAR1GRAY,
    CAR2,
    CAR2BLACK,
    CAR2RED,
    CAR3,
    CAR3RED,
    CAR3YELLOW,
    CAR4,
    CAR4GREY,
    CAR4LIGHTGREY,
    CAR4LIGHTORANGE,
    CAR5,
    CAR5POLICE,
    CAR5TAXI,
    CAR7,
    CAR7RED,
    CAR7GREY,
    CAR7GREEN,
    CAR7BROWN,
    CAR7BLACK,
    CAR8,
    CAR8GREY,
    CAR8MAIL,
    CAR8PURPLE,
    TOTALCARS
};

extern vector<string> OBJpaths;
extern vector<vector<int>> carColorMap;
extern vector<string> environmentPaths;

class Car {
public:
    Car();
    void move(float dx, float dy);
    void draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const;
    void assign(COBJModel* model);

    float m_x;
    float m_y;
    float m_height;
    float m_width;
    float m_speed;
    bool m_visible;
    COBJModel* m_model;
};

class Circle {
public:
    Circle() : m_x(0), m_y(0), m_radius(LANE_WIDTH / 5) {}
    Circle(float x, float y, float radius) : m_x(x), m_y(y), m_radius(radius) {}

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
    float m_move_step;
    float m_rotation_speed;
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

class EnvironmentRow {
public:
    EnvironmentRow() : m_leftHouse(nullptr), m_rightHouse(nullptr), m_z(0) {};
    void initRow(float z);
    void move(float dz);
    void draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const;
    float getZ() const;

private:
    COBJModel* m_leftHouse;
    COBJModel* m_rightHouse;
    float m_z;
};


class Environment {
public:
    Environment();

    void draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const;
    void update(float dz);
    void dibuixaRoad(GLuint sh_programID, const glm::mat4 MatriuVista, const glm::mat4 MatriuTG) const;
    float m_roadY;
private:
    EnvironmentRow m_environmentRows[NUM_REPEATS];
    COBJModel* m_road;
};

class GameLogic {
private:
    void UpdateRoadRows();
    void DoCollisions();
    void DoPickUps();
    void finalCoinAnimation(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, float t) const;
    int nextEmptyLane;
    COBJModel* modelCoin;
    float t;


public:
    GameLogic();
    void GetUserInput();
    void UpdateGameLogic();
    bool CoinFlip();
    void draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const;

    float remainingFuel;
    float remainingShield;
    int score;
    bool animationRunning;
    bool shieldEquipped;
    bool gameRunning;
    bool extraLife;
    Player player;
    RoadRow roadRows[NUM_ROWS];
    Environment environment;
};