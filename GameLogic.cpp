#include "GameLogic.h"

vector<COBJModel*> CAR_MODELS;
COBJModel* OBJECT_MODELS[3];


// Implementació de Car
Car::Car(float x, float y, float speed, float w, float h)
    : m_x(x), m_y(y), m_height(h), m_width(w), m_speed(speed), m_visible(true), m_model(NULL) {}

void Car::move(float dx, float dy) {
    m_x += dx;
    m_y += dy;
}

void Car::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const
{
    if (m_visible) {
        glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0), TransMatrix(1.0), ScaleMatrix(1.0), RotMatrix(1.0);
        TransMatrix = MatriuTG;

        TransMatrix = glm::translate(TransMatrix, vec3(m_x, 0, m_y));
        float scaleFactor = m_width / m_model->m_width;
        TransMatrix = glm::scale(TransMatrix, vec3(scaleFactor, scaleFactor, scaleFactor));
        TransMatrix = glm::rotate(TransMatrix, float(PI), vec3(0, 1, 0));
        ModelMatrix = TransMatrix;

        // Pas ModelView Matrix a shader
        glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
        NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
        // Pas NormalMatrix a shader
        glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);


        // Objecte OBJ: Dibuix de l'objecte OBJ amb textures amb varis VAO's, un per a cada material.
        m_model->draw_TriVAO_OBJ(sh_programID);	// Dibuixar VAO a pantalla
    }
}

// Implementació de RoadRow
RoadRow::RoadRow()
{
    for (int i = 0; i < NUM_LANES; i++) {
        m_obstacles[i].m_x = ROAD_START + LANE_WIDTH / 2 + i * LANE_WIDTH;
    }
}

bool RoadRow::checkCollision(const Player& player) const {
    if (getY() > COLLISION_START)
    {
        for (int i = 0; i < NUM_LANES; i++) {
            if (player.checkCollision(m_obstacles[i])) {
                return true;
            }
        }
    }
    return false;
}

void RoadRow::move(float dy) {
    for (int i = 0; i < NUM_LANES; i++) {
        m_obstacles[i].m_y += dy - m_obstacles[i].m_speed;
    }
    m_object.m_y += dy;
    m_object.m_rotation = fmod(m_object.m_rotation + ROTATION_VEL, TWOPI);
}

void RoadRow::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const {
    for (int i = 0; i < NUM_LANES; i++) {
        m_obstacles[i].draw(sh_programID, MatriuVista, MatriuTG);
    }
    m_object.draw(sh_programID, MatriuVista, MatriuTG);
}

void RoadRow::initRow(float y, int& nextEmptyLane)
{
    int currentEmptyLane = nextEmptyLane;

    for (int i = 0; i < NUM_LANES; ++i)
    {
        m_obstacles[i].m_model = CAR_MODELS[rand() % NUM_CAR_MODELS];
        m_obstacles[i].m_width = CAR_WIDTH;
        m_obstacles[i].m_height = (CAR_WIDTH / m_obstacles[i].m_model->m_width) * m_obstacles[i].m_model->m_depth;

        m_obstacles[i].m_y = y + (rand() % 2 == 0 ? -1 : 1) * (rand() % VERTICAL_NOISE);
        m_obstacles[i].m_visible = true;
    }
    m_obstacles[currentEmptyLane].m_visible = false;

    int variation = (rand() % 2 == 0) ? -1 : 1;
    nextEmptyLane += variation;
    if (nextEmptyLane < 0 || nextEmptyLane >= NUM_LANES) {
        nextEmptyLane -= variation * 2;
    }

    int numEmptyLanes = rand() % (NUM_LANES - MIN_CARS) + 1;

    int emptyLanes = 1;
    while (emptyLanes < numEmptyLanes) {
        int i = rand() % NUM_LANES;

        if (i != nextEmptyLane && m_obstacles[i].m_visible) {
            m_obstacles[i].m_visible = false;
            emptyLanes++;
        }
    }

    m_object.m_visible = rand() % 100 < PROB_OBJECT;

    if (m_object.m_visible)
    {
        int value = rand() % 100;

        if (value < PROB_FUEL) {
            m_object.m_type = FUEL;
            m_object.m_model = OBJECT_MODELS[FUEL];
        }
        else if (value < PROB_FUEL + PROB_SHIELD) {
            m_object.m_type = SHIELD;
            m_object.m_model = OBJECT_MODELS[SHIELD];

        }
        else {
            m_object.m_type = COIN;
            m_object.m_model = OBJECT_MODELS[COIN];
        }

        while(true)
        {
            int i = rand() % NUM_LANES;

            if (!m_obstacles[i].m_visible) {
                m_object.m_x = ROAD_START + LANE_WIDTH / 2 + i * LANE_WIDTH;
                m_object.m_y = y;
                break;
            }
        }
    }
}

float RoadRow::getY() const {
    return m_obstacles[0].m_y;
}

// Implementació de GameLogic
GameLogic::GameLogic() : gameRunning(true), m_roadY(0){
    srand(static_cast<unsigned int>(time(nullptr)));


    m_road = new COBJModel();
    const char* rutaArxiu = "..\\x64\\Release\\OBJFiles\\Road\\road.obj";
    m_road->LoadModel(const_cast<char*>(rutaArxiu));

    nextEmptyLane = rand() % NUM_LANES;
    float y = -(CAR_HEIGHT / 2 + VERTICAL_NOISE);

    for (int i = 0; i < NUM_ROWS; ++i) {
        roadRows[i].initRow(y, nextEmptyLane);
        y += -CAR_HEIGHT - ROW_SPACING;
    }
}

void GameLogic::UpdateGameLogic() {

    UpdateRoadRows();
    DoCollisions();
    DoPickUps();

    player.m_speed += 0.002;
    m_roadY += player.m_speed;
}

void GameLogic::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const
{
    dibuixaRoad(sh_programID, MatriuVista, MatriuTG);

    for (int i = 0; i < NUM_ROWS; i++) {
        roadRows[i].draw(sh_programID, MatriuVista, MatriuTG);  // Dibuixar cada fila
    }

    player.draw(sh_programID, MatriuVista, MatriuTG);  // Dibuixar el jugador
}

void GameLogic::GetUserInput()
{
    if (GetKeyState('A') & 0x8000 && player.m_rotation<=0) { //Només per a Windows 
        player.move(-STEP, 0);

        player.rotate(player.m_rotation > -ROTATION_ANGLE ? -ROTATION_SPEED : 0);

        if (player.m_x < ROAD_START + player.m_width / 2)
        {
            player.move(ROAD_START + player.m_width / 2 - player.m_x, 0);
            player.rotate(player.m_rotation < 0 ? ROTATION_SPEED : 0);
        }
    }
    else if (GetKeyState('D') & 0x8000 && player.m_rotation >= 0) {
        player.move(STEP, 0);

        player.rotate(player.m_rotation < ROTATION_ANGLE ? ROTATION_SPEED : 0);
        if (player.m_x > ROAD_END - player.m_width / 2)
        {
            player.move(-(player.m_x - ROAD_END + player.m_width / 2), 0);
            player.rotate(player.m_rotation > 0 ? -ROTATION_SPEED : 0);
        }
    }
    else
    {
        player.rotate(player.m_rotation > 0 ? -ROTATION_SPEED : 0);
        player.rotate(player.m_rotation < 0 ? ROTATION_SPEED : 0);
    }
}

void GameLogic::UpdateRoadRows()
{
    for (int i = 0; i < NUM_ROWS; ++i) {
        roadRows[i].move(player.m_speed);

        if (roadRows[i].getY() > 2 * WINDOW_HEIGHT) {
            float newY = roadRows[(i + NUM_ROWS - 1) % NUM_ROWS].getY() - CAR_HEIGHT - ROW_SPACING;
            roadRows[i].initRow(newY, nextEmptyLane);
        }
    }
}

void GameLogic::DoCollisions()
{
    bool collision = false;
    int i = 0;
    while (i < NUM_ROWS && !collision)
    {
        collision = collision || roadRows[i].checkCollision(player);
        i++;
    }
    gameRunning = !collision;
}

void GameLogic::DoPickUps()
{
    for (int i = 0; i < NUM_ROWS; i++)
    {
        if (player.checkCollision(roadRows[i].m_object))
        {
            roadRows[i].m_object.m_visible = false;
            PickUp& p = roadRows[i].m_object.m_type;
            switch (p)
            {
                // TODO
            default:
                break;
            }
        }
    }
}

// Implementació Player
Player::Player() : Car::Car(ROAD_START + ROAD_WIDTH / 2, WINDOW_HEIGHT - CAR_HEIGHT / 2 - MARGIN, PLAYER_SPEED)
{
    m_rotation = 0;
    m_model = CAR_MODELS[3];
    m_width = CAR_WIDTH;
    m_height = (CAR_WIDTH / m_model->m_width)* m_model->m_depth;

    for (int i = 0; i < 3; i++)
    {
        m_collisionCircles[i] = Circle(m_x, m_y - m_height / 4 + m_height / 4 * i, m_width / 2);
    }
}

void Player::move(float dx, float dy)
{
    Car::move(dx, dy);
    for (int i = 0; i < 3; i++)
    {
        m_collisionCircles[i].m_x += dx;
    }
}

void Player::rotate(float dAngle)
{
    m_rotation += dAngle;
    for (int i = 0; i < 3; i++)
    {
        m_collisionCircles[i].m_x = m_x - sin(m_rotation) * m_height / 4 * i;
        m_collisionCircles[i].m_y = m_y - m_height / 4 + cos(abs(m_rotation)) * m_height / 4 * i;

    }
}

void Player::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const
{
    glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0), TransMatrix(1.0), ScaleMatrix(1.0), RotMatrix(1.0);
    TransMatrix = MatriuTG;


    TransMatrix = glm::translate(TransMatrix, vec3(m_x, 0, m_y - m_height / 4));
    TransMatrix = glm::rotate(TransMatrix, m_rotation, vec3(0, -1, 0));
    TransMatrix = glm::translate(TransMatrix, vec3(0, 0, m_height / 4)); 
    TransMatrix = glm::rotate(TransMatrix, float(PI), vec3(0, 1, 0));
    float scaleFactor = m_width / m_model->m_width;
    TransMatrix = glm::scale(TransMatrix, vec3(scaleFactor, scaleFactor, scaleFactor));
    ModelMatrix = TransMatrix;

    // Pas ModelView Matrix a shader
    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
    NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
    // Pas NormalMatrix a shader
    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);

    // Objecte OBJ: Dibuix de l'objecte OBJ amb textures amb varis VAO's, un per a cada material.
    m_model->draw_TriVAO_OBJ(sh_programID);	// Dibuixar VAO a pantalla
}

bool Player::checkCollision(const Car& obstacle) const {
    if (obstacle.m_visible)
    {
        for (int i = 0; i < 3; i++)
        {
            const Circle& playerCircle = m_collisionCircles[i];

            glm::vec2 center(playerCircle.m_x, playerCircle.m_y);
            glm::vec2 aabb_half_extents(obstacle.m_width / 2.0f, obstacle.m_height / 2.0f);
            glm::vec2 aabb_center(obstacle.m_x, obstacle.m_y);

            glm::vec2 difference = center - aabb_center;
            glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
            glm::vec2 closest = aabb_center + clamped;
            difference = closest - center;

            if (glm::length(difference) < playerCircle.m_radius - COLLISION_TOLERANCE)
                return true;
        }
    }
    return false;
}

bool Player::checkCollision(const Object& object) const
{
    if (object.m_visible)
    {
        for (int i = 0; i < 3; i++)
        {
            const Circle& playerCircle = m_collisionCircles[i];
            glm::vec2 difference(playerCircle.m_x - object.m_x, playerCircle.m_y - object.m_y);

            if (glm::length(difference) < playerCircle.m_radius + object.m_radius)
                return true;
        }
    }
    return false;
}




void GameLogic::dibuixaRoad(GLuint sh_programID, const glm::mat4 MatriuVista, const glm::mat4 MatriuTG) const {
    
    float length = 2* 63.8 * (20 + ROAD_WIDTH / 6.26);

    glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0), TransMatrix(1.0), ScaleMatrix(1.0), RotMatrix(1.0);
    TransMatrix = MatriuTG;

    TransMatrix = glm::translate(TransMatrix, vec3(10 + ROAD_WIDTH / 2, -10, length/8 + fmod(m_roadY, length/8)));
    TransMatrix = glm::scale(TransMatrix, vec3(20 + ROAD_WIDTH / 6.26, 20 + ROAD_WIDTH / 6.26, 20 + ROAD_WIDTH / 6.26));
    TransMatrix = glm::rotate(TransMatrix, float(PI / 2), vec3(0, 1, 0));
    ModelMatrix = TransMatrix;

    // Pas ModelView Matrix a shader
    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
    NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
    // Pas NormalMatrix a shader
    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);

    // Objecte OBJ: Dibuix de l'objecte OBJ amb textures amb varis VAO's, un per a cada material.
    m_road->draw_TriVAO_OBJ(sh_programID);	// Dibuixar VAO a pantalla
}

void Object::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const
{
    if (m_visible) {
        glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0), TransMatrix(1.0), ScaleMatrix(1.0), RotMatrix(1.0);
        TransMatrix = MatriuTG;

        float scaleFactor = m_radius * 2 / m_model->m_width;
        TransMatrix = translate(TransMatrix, vec3(m_x, m_model->m_height* scaleFactor *2/3, m_y));
        TransMatrix = rotate(TransMatrix, m_rotation, vec3(0, 1, 0));
        TransMatrix = scale(TransMatrix, vec3(scaleFactor, scaleFactor, scaleFactor));
        ModelMatrix = TransMatrix;

        // Pas ModelView Matrix a shader
        glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
        NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
        // Pas NormalMatrix a shader
        glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);


        // Objecte OBJ: Dibuix de l'objecte OBJ amb textures amb varis VAO's, un per a cada material.
        m_model->draw_TriVAO_OBJ(sh_programID);	// Dibuixar VAO a pantalla
    }
}
