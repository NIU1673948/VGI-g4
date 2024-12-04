#include "GameLogic.h"

vector<COBJModel*> CAR_MODELS;
COBJModel* OBJECT_MODELS[3];
vector<COBJModel*> ENVIRONMENT_MODELS;

vector<string> environmentPaths = {
    //".\\OBJFiles\\Casa1\\house.obj"
    //".\\OBJFiles\\Casa2\\OldHouse.obj"
    ".\\OBJFiles\\Casa3\\new_house.obj",
    //".\\OBJFiles\\Edifici\\building.obj",
    ".\\OBJFiles\\Cottage\\cottage.obj"
};

vector<string> OBJpaths = {
    ".\\OBJFiles\\Car 01\\Car.obj",
    ".\\OBJFiles\\Car 01\\Car_blue.obj",
    ".\\OBJFiles\\Car 01\\Car_red.obj",
    ".\\OBJFiles\\Car 01\\Car_gray.obj",
    ".\\OBJFiles\\Car 02\\Car2.obj",
    ".\\OBJFiles\\Car 02\\Car2_black.obj",
    ".\\OBJFiles\\Car 02\\Car2_red.obj",
    ".\\OBJFiles\\Car 03\\Car3.obj",
    ".\\OBJFiles\\Car 03\\Car3_red.obj",
    ".\\OBJFiles\\Car 03\\Car3_yellow.obj",
    ".\\OBJFiles\\Car 04\\Car4.obj",
    ".\\OBJFiles\\Car 04\\Car4_grey.obj",
    ".\\OBJFiles\\Car 04\\Car4_lightgrey.obj",
    ".\\OBJFiles\\Car 04\\Car4_lightorange.obj",
    ".\\OBJFiles\\Car 05\\Car5.obj",
    ".\\OBJFiles\\Car 05\\Car5_Police.obj",
    ".\\OBJFiles\\Car 05\\Car5_Taxi.obj",
    ".\\OBJFiles\\Car 07\\Car7.obj",
    ".\\OBJFiles\\Car 07\\Car7_red.obj",
    ".\\OBJFiles\\Car 07\\Car7_grey.obj",
    ".\\OBJFiles\\Car 07\\Car7_green.obj",
    ".\\OBJFiles\\Car 07\\Car7_brown.obj",
    ".\\OBJFiles\\Car 07\\Car7_black.obj",
    ".\\OBJFiles\\Car 08\\Car8.obj",
    ".\\OBJFiles\\Car 08\\Car8_grey.obj",
    ".\\OBJFiles\\Car 08\\Car8_mail.obj",
    ".\\OBJFiles\\Car 08\\Car8_purple.obj",
};

vector<vector<int>> carColorMap = {
    {CAR1, CAR1BLUE, CAR1RED, CAR1GRAY},
    {CAR2, CAR2BLACK, CAR2RED},
    {CAR3, CAR3RED, CAR3YELLOW},
    {CAR4, CAR4GREY, CAR4LIGHTGREY, CAR4LIGHTORANGE},
    {CAR5, CAR5POLICE, CAR5TAXI},
    {CAR7, CAR7RED, CAR7GREY, CAR7GREEN, CAR7BROWN, CAR7BLACK},
    {CAR8, CAR8GREY, CAR8MAIL, CAR8PURPLE}
};

Car::Car() {
    m_x = 0;
    m_y = 0;
    m_height = CAR_HEIGHT;
    m_width = CAR_WIDTH;
    m_speed = OBSTACLE_SPEED;
    m_visible = true;
    m_model = NULL;
}

void Car::move(float dx, float dy) {
    m_x += dx;
    m_y += dy;
}

void Car::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const
{
    if (m_visible)
    {
        glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0), TransMatrix(1.0), ScaleMatrix(1.0), RotMatrix(1.0);
        TransMatrix = MatriuTG;

        TransMatrix = glm::translate(TransMatrix, vec3(m_x, 0, m_y));
        float scaleFactor = m_width / m_model->m_width;
        TransMatrix = glm::scale(TransMatrix, vec3(scaleFactor, scaleFactor, scaleFactor));
        TransMatrix = glm::rotate(TransMatrix, float(PI), vec3(0, 1, 0));
        ModelMatrix = TransMatrix;

        glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
        NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);

        m_model->draw_TriVAO_OBJ(sh_programID);
    }
}

void Car::assign(COBJModel* model)
{
    m_model = model;
}

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
        m_obstacles[i].m_model = CAR_MODELS[rand() % TOTALCARS];
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

Environment::Environment() : m_roadY(0), m_environmentObjects(nullptr), m_road(nullptr)
{
    m_road = new COBJModel();
    const char* rutaArxiu = ".\\OBJFiles\\Road\\Road.obj";
    m_road->LoadModel(const_cast<char*>(rutaArxiu));

    m_environmentObjects = new COBJModel[ENVIRONMENT_MODELS.size()];
}

void Environment::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const {
    float length = 2 * 63.8 * (20 + ROAD_WIDTH / 6.26);
    int n = ENVIRONMENT_MODELS.size();
    int side = 1;

    int num_repeats = 8;
    float z_spacing = length / 8;

    for (int z_index = 0; z_index < num_repeats; z_index++) {
        for (int i = 0; i < n; i++) { 
            m_environmentObjects[i] = *ENVIRONMENT_MODELS[i];

            glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0), TransMatrix(1.0), ScaleMatrix(1.0), RotMatrix(1.0);
            TransMatrix = MatriuTG;

            float scaleFactor = HOUSE_WIDTH / m_environmentObjects[i].m_width;

            if (side == 1) {
                TransMatrix = glm::translate(TransMatrix, glm::vec3(-50.0f * scaleFactor, 0.0f, z_index * z_spacing + fmod(m_roadY, z_spacing)));
                side = 2; 
            }
            else {
                TransMatrix = glm::translate(TransMatrix, glm::vec3(20.0f * scaleFactor, 0.0f, z_index * z_spacing + fmod(m_roadY, z_spacing)));
                side = 1;
            }

            TransMatrix = glm::scale(TransMatrix, glm::vec3(scaleFactor, scaleFactor, scaleFactor));
            TransMatrix = glm::rotate(TransMatrix, float(PI), vec3(0, 2, 0));
            ModelMatrix = TransMatrix;

            glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
            NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
            glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);

            m_environmentObjects[i].draw_TriVAO_OBJ(sh_programID);
        }
    }
}


GameLogic::GameLogic() : gameRunning(true), score(0)
{
    srand(static_cast<unsigned int>(time(nullptr)));

    modelCoin = new COBJModel();
    string path = ".\\OBJFiles\\Final_Coin\\Final_Coin.obj";
    modelCoin->LoadModel(const_cast<char*>(path.c_str()));

    remainingFuel = FUEL_DURATION;
    remainingShield = -SHIELD_DURATION;
    shieldEquipped = false;

    nextEmptyLane = rand() % NUM_LANES;
    float y = -(CAR_HEIGHT / 2 + VERTICAL_NOISE);

    for (int i = 0; i < NUM_ROWS; ++i) {
        roadRows[i].initRow(y, nextEmptyLane);
        y -= CAR_HEIGHT + ROW_SPACING;
    }
}

void GameLogic::UpdateGameLogic() {

    UpdateRoadRows();

    if (remainingShield <= 0)
        DoCollisions();
    else
        remainingShield -= FRAME_TIME;
    
    DoPickUps();

    remainingFuel -= FRAME_TIME;

    if (remainingFuel <= 0)
        gameRunning = false;

    player.m_speed += 0.002;
    player.m_move_step += 0.00000002;

    score += player.m_speed;
    environment.m_roadY += player.m_speed;
}

void GameLogic::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const
{
    glUniform1f(glGetUniformLocation(sh_programID, "transparency"), 1.0f);


    for (int i = 0; i < NUM_ROWS; i++) {
        roadRows[i].draw(sh_programID, MatriuVista, MatriuTG);    }

    environment.dibuixaRoad(sh_programID, MatriuVista, MatriuTG);
    environment.draw(sh_programID, MatriuVista, MatriuTG);

    glUniform1f(glGetUniformLocation(sh_programID, "transparency"), remainingShield <= 0? 1:0.5f);

    player.draw(sh_programID, MatriuVista, MatriuTG);


}


void GameLogic::GetUserInput()
{
    if (GetKeyState('A') & 0x8000 && player.m_rotation<=0) {        
        player.move(-player.m_move_step, 0);

        player.rotate(player.m_rotation > -ROTATION_ANGLE ? -ROTATION_SPEED : 0);

        if (player.m_x < ROAD_START + player.m_width / 2)
        {
            player.move(ROAD_START + player.m_width / 2 - player.m_x, 0);
            player.rotate(player.m_rotation < 0 ? ROTATION_SPEED : 0);
        }
    }
    else if (GetKeyState('D') & 0x8000 && player.m_rotation >= 0) {
        player.move(player.m_move_step, 0);

        player.rotate(player.m_rotation < ROTATION_ANGLE ? ROTATION_SPEED : 0);
        if (player.m_x > ROAD_END - player.m_width / 2)
        {
            player.move(-(player.m_x - ROAD_END + player.m_width / 2), 0);
            player.rotate(player.m_rotation > 0 ? -ROTATION_SPEED : 0);
        }
    }
    else if (GetKeyState(VK_SPACE) & 0x8000 && shieldEquipped) {
        shieldEquipped = false;
        remainingShield = SHIELD_DURATION;
    }
    else
    {
        if (player.m_rotation > 0)
            player.rotate(player.m_rotation > ROTATION_SPEED ? -ROTATION_SPEED : -player.m_rotation);
        else if (player.m_rotation < 0)
            player.rotate(player.m_rotation < -ROTATION_SPEED ? ROTATION_SPEED : -player.m_rotation);

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
            case COIN:
                score += COIN_SCORE;
                player.m_speed = std::max(player.m_speed - COIN_SPEED_DOWN, 0.0f);
                break;
            case FUEL:
                remainingFuel = FUEL_DURATION;
                break;
            case SHIELD:
                shieldEquipped = true;
                break;
            default:
                break;
            }
        }
    }
}

Player::Player()
{
    m_model = CAR_MODELS[3];

    m_x = ROAD_START + ROAD_WIDTH / 2;
    m_y = WINDOW_HEIGHT - CAR_HEIGHT / 2 - MARGIN;
    m_height = (CAR_WIDTH / m_model->m_width) * m_model->m_depth;
    m_width = CAR_WIDTH;
    m_speed = PLAYER_SPEED;
    m_rotation = 0;
    m_move_step = STEP;
    m_rotation_speed = ROTATION_SPEED;

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

    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
    NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);

    m_model->draw_TriVAO_OBJ(sh_programID);}

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

        glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
        NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);

        m_model->draw_TriVAO_OBJ(sh_programID);    }
}

void GameLogic::finalCoinAnimation(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const
{
    glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0), TransMatrix(1.0), ScaleMatrix(1.0), RotMatrix(1.0);
    TransMatrix = MatriuTG;

    TransMatrix = translate(TransMatrix, vec3(0,0,-10));
    TransMatrix = scale(TransMatrix, vec3(100, 100, 100));

    ModelMatrix = MatriuVista * TransMatrix;

    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
    NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);

    modelCoin->draw_TriVAO_OBJ(sh_programID);}