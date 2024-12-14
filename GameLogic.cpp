#include "GameLogic.h"

vector<COBJModel*> CAR_MODELS;
COBJModel* OBJECT_MODELS[3];
vector<COBJModel*> ENVIRONMENT_MODELS;

vector<string> environmentPaths = {
    ".\\OBJFiles\\low_poly_houses_pack\\house_01.obj",
    ".\\OBJFiles\\low_poly_houses_pack\\house_02.obj",
    //".\\OBJFiles\\low_poly_houses_pack\\house_03.obj",
    //".\\OBJFiles\\low_poly_houses_pack\\house_04.obj",
    ".\\OBJFiles\\low_poly_houses_pack\\house_05.obj",
    ".\\OBJFiles\\low_poly_houses_pack\\house_06.obj",
    ".\\OBJFiles\\low_poly_houses_pack\\house_07.obj",
    //".\\OBJFiles\\low_poly_houses_pack\\house_08.obj",
    //".\\OBJFiles\\low_poly_houses_pack\\house_09.obj",
    //".\\OBJFiles\\low_poly_houses_pack\\house_10.obj"

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

void doMusic(const std::string& musicFilePath) {
    irrklang::ISoundEngine* tempEngine = irrklang::createIrrKlangDevice();
    if (!tempEngine) {
        std::cout << "Could not startup engine" << std::endl;
        return;
    }
    // Reproduce el sonido y no esperes su finalizaci�n en un loop infinito.
    tempEngine->play2D(musicFilePath.c_str(), false);
    // Aqu� surge un problema: Si sales de la funci�n de inmediato, el engine se destruye al salir 
    // (si es local) o si haces drop(), la reproducci�n se corta. Debes mantener el engine vivo 
    // mientras suene el sonido.
}



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

    switch (m_dificultat)
    {
        case(PRINCIPIANT):
            m_object.m_visible = rand() % 100 < PROB_OBJECT_PRINCIPIANT;

            if (m_object.m_visible)
            {
                int value = rand() % 100;

                if (value < PROB_FUEL_PRINCIPIANT) {
                    m_object.m_type = FUEL;
                    m_object.m_model = OBJECT_MODELS[FUEL];
                }
                else if (value < PROB_FUEL_PRINCIPIANT + PROB_SHIELD_PRINCIPIANT) {
                    m_object.m_type = SHIELD;
                    m_object.m_model = OBJECT_MODELS[SHIELD];
                }
                else {
                    m_object.m_type = COIN;
                    m_object.m_model = OBJECT_MODELS[COIN];
                }

                while (true)
                {
                    int i = rand() % NUM_LANES;

                    if (!m_obstacles[i].m_visible) {
                        m_object.m_x = ROAD_START + LANE_WIDTH / 2 + i * LANE_WIDTH;
                        m_object.m_y = y;
                        break;
                    }
                }
            }
            break;
        case(INTERMIG):
            m_object.m_visible = rand() % 100 < PROB_OBJECT_INTERMIG;

            if (m_object.m_visible)
            {
                int value = rand() % 100;

                if (value < PROB_FUEL_INTERMIG) {
                    m_object.m_type = FUEL;
                    m_object.m_model = OBJECT_MODELS[FUEL];
                }
                else if (value < PROB_FUEL_INTERMIG + PROB_SHIELD_INTERMIG) {
                    m_object.m_type = SHIELD;
                    m_object.m_model = OBJECT_MODELS[SHIELD];
                }
                else {
                    m_object.m_type = COIN;
                    m_object.m_model = OBJECT_MODELS[COIN];
                }

                while (true)
                {
                    int i = rand() % NUM_LANES;

                    if (!m_obstacles[i].m_visible) {
                        m_object.m_x = ROAD_START + LANE_WIDTH / 2 + i * LANE_WIDTH;
                        m_object.m_y = y;
                        break;
                    }
                }
            }
            break;
        case(EXPERT):
            m_object.m_visible = rand() % 100 < PROB_OBJECT_EXPERT;

            if (m_object.m_visible)
            {
                int value = rand() % 100;

                if (value < PROB_FUEL_EXPERT) {
                    m_object.m_type = FUEL;
                    m_object.m_model = OBJECT_MODELS[FUEL];
                }
                else if (value < PROB_FUEL_EXPERT + PROB_SHIELD_EXPERT) {
                    m_object.m_type = SHIELD;
                    m_object.m_model = OBJECT_MODELS[SHIELD];
                }
                else {
                    m_object.m_type = COIN;
                    m_object.m_model = OBJECT_MODELS[COIN];
                }

                while (true)
                {
                    int i = rand() % NUM_LANES;

                    if (!m_obstacles[i].m_visible) {
                        m_object.m_x = ROAD_START + LANE_WIDTH / 2 + i * LANE_WIDTH;
                        m_object.m_y = y;
                        break;
                    }
                }
            }
            break;
    }
}

float RoadRow::getY() const {
    return m_obstacles[0].m_y;
}

void EnvironmentRow::initRow(float y, bool rightHouse, COBJModel* tree) {
    m_y = y;
    m_house = ENVIRONMENT_MODELS[rand() % ENVIRONMENT_MODELS.size()];
    m_rightHouse = rightHouse;
    m_treeVisible = rand() % 2;
    m_tree = tree;
}

void EnvironmentRow::move(float dy) {
    m_y += dy;
}

float EnvironmentRow::getY() const {
    return m_y;
}

void EnvironmentRow::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const
{
    glm::mat4 ModelMatrix(1.0), TransMatrix(1.0);
    float scaleFactor = HOUSE_WIDTH / m_house->m_width;

    if (m_rightHouse)
    {
        TransMatrix = glm::translate(MatriuTG, glm::vec3(15.0f * scaleFactor, 0.0f, m_y));
        TransMatrix = glm::rotate(TransMatrix, float(PI), glm::vec3(0, 2, 0));
    }
    else
    {
        TransMatrix = glm::translate(MatriuTG, glm::vec3(-9.0f * scaleFactor, 0.0f, m_y));
    }

    TransMatrix = glm::scale(TransMatrix, glm::vec3(scaleFactor, scaleFactor, scaleFactor));
    ModelMatrix = TransMatrix;
    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
    m_house->draw_TriVAO_OBJ(sh_programID);

    if (m_rightHouse)
    {
        TransMatrix = glm::translate(MatriuTG, glm::vec3(15.0f * scaleFactor, 0.0f, m_y+(m_house->m_depth/2 * scaleFactor)+400));
        TransMatrix = glm::rotate(TransMatrix, float(PI), glm::vec3(0, 2, 0));
    }
    else
    {
        TransMatrix = glm::translate(MatriuTG, glm::vec3(-9.0f * scaleFactor, 0.0f, m_y + (m_house->m_depth / 2 * scaleFactor) + 400));
    }

    scaleFactor = TREE_WIDTH / m_tree->m_width;
    TransMatrix = glm::scale(TransMatrix, glm::vec3(scaleFactor, scaleFactor, scaleFactor));
    ModelMatrix = TransMatrix;
    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
    m_tree->draw_TriVAO_OBJ(sh_programID);


}


Environment::Environment() : m_roadY(0), m_road(nullptr)
{
    m_road = new COBJModel();
    const char* rutaArxiu = ".\\OBJFiles\\Road\\Road.obj";
    m_road->LoadModel(const_cast<char*>(rutaArxiu));

    m_tree = new COBJModel();
    rutaArxiu = ".\\OBJFiles\\Tree\\WC_Tree_01.obj";
    m_tree->LoadModel(const_cast<char*>(rutaArxiu));

    nextRightHouse = true;

    float y = INIT_POSITION;
    for (int i = 0; i < NUM_REPEATS; ++i) {
        m_environmentRows[i].initRow(y, nextRightHouse, m_tree);
        nextRightHouse = !nextRightHouse;
        y -= Y_SPACE;
    }

}

void Environment::update(float dy) {
    for (int i = 0; i < NUM_REPEATS; ++i) {
        m_environmentRows[i].move(dy);

        if (m_environmentRows[i].getY() > INIT_POSITION * 6) {
            float newY = m_environmentRows[(i + NUM_REPEATS - 1) % NUM_REPEATS].getY() - Y_SPACE;
            m_environmentRows[i].initRow(newY, nextRightHouse, m_tree);
            nextRightHouse = !nextRightHouse;
        }
    }
}

void Environment::dibuixaRoad(GLuint sh_programID, const glm::mat4 MatriuVista, const glm::mat4 MatriuTG) const
{

    glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0), TransMatrix(1.0), ScaleMatrix(1.0), RotMatrix(1.0);
    TransMatrix = MatriuTG;

    TransMatrix = glm::translate(TransMatrix, vec3(10 + ROAD_WIDTH / 2, -10, ROAD_LENGTH / 8 + fmod(m_roadY, ROAD_LENGTH / 8)));
    TransMatrix = glm::scale(TransMatrix, vec3(20 + ROAD_WIDTH / 6.26, 20 + ROAD_WIDTH / 6.26, 20 + ROAD_WIDTH / 6.26));
    TransMatrix = glm::rotate(TransMatrix, float(PI / 2), vec3(0, 1, 0));
    ModelMatrix = TransMatrix;

    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
    NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);

    m_road->draw_TriVAO_OBJ(sh_programID);
}

void Environment::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const {
    for (const auto& row : m_environmentRows) {
        row.draw(sh_programID, MatriuVista, MatriuTG);
    }
    dibuixaRoad(sh_programID, MatriuVista, MatriuTG);
}

GameLogic::GameLogic(int dif) : gameRunning(true), score(0), t(0), animationRunning(false), m_dificultat(dif)
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
        roadRows[i].m_dificultat=dif;
        roadRows[i].initRow(y, nextEmptyLane);
        y -= CAR_HEIGHT + ROW_SPACING;
    }

    extraLife = rand() % 2 == 0;
}

void GameLogic::UpdateGameLogic(SoundManager& soundManager) {

    UpdateRoadRows();
    environment.update(player.m_speed);

    if (remainingShield <= 0)
        DoCollisions();
    else
        remainingShield -= FRAME_TIME;
    
    DoPickUps(soundManager);

    remainingFuel -= FRAME_TIME;

    if (remainingFuel <= 0)
        gameRunning = false;

    switch (m_dificultat)
    {
        case(PRINCIPIANT):
            player.m_speed += 0.002;
            break;
        case(INTERMIG):
            player.m_speed += 0.003;
            break;
        case(EXPERT):
            player.m_speed += 0.005;
            break;
    }

    player.m_move_step += 0.00000002;

    score += player.m_speed;
    environment.m_roadY += player.m_speed;
}

bool GameLogic::CoinFlip()
{
    animationRunning = t <= ANIMATION_DURATION + 1;
    t += FRAME_TIME;

    return extraLife;
}

void GameLogic::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const
{
    glUniform1f(glGetUniformLocation(sh_programID, "transparency"), 1.0f);


    for (int i = 0; i < NUM_ROWS; i++) {
        roadRows[i].draw(sh_programID, MatriuVista, MatriuTG);
    }

    environment.draw(sh_programID, MatriuVista, MatriuTG);

    if (animationRunning) finalCoinAnimation(sh_programID, MatriuVista, MatriuTG, t);

    if (remainingShield >= 1.5 || (remainingShield > 0 && fmod(remainingShield, 0.5f) < 0.25f) ){
        glUniform1f(glGetUniformLocation(sh_programID, "transparency"), 0.5f);
    }

    player.draw(sh_programID, MatriuVista, MatriuTG);
}


void GameLogic::GetUserInput()
{
    if (GetKeyState('A') & 0x8000 && player.m_rotation <= 0) {
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
    else
    {
        if (player.m_rotation > 0)
            player.rotate(player.m_rotation > ROTATION_SPEED ? -ROTATION_SPEED : -player.m_rotation);
        else if (player.m_rotation < 0)
            player.rotate(player.m_rotation < -ROTATION_SPEED ? ROTATION_SPEED : -player.m_rotation);

    }

    if (GetKeyState(VK_SPACE) & 0x8000 && shieldEquipped) {
        shieldEquipped = false;
        remainingShield = SHIELD_DURATION;
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

void GameLogic::DoPickUps(SoundManager& soundManager)
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
                player.m_speed = player.m_speed * (1 - COIN_SPEED_DOWN);
                soundManager.playSound(".\\media\\coin.mp3");
                break;
            case FUEL:
                remainingFuel = FUEL_DURATION;
                soundManager.playSound(".\\media\\fuel.mp3");
                break;
            case SHIELD:
                shieldEquipped = true;
                soundManager.playSound(".\\media\\shield.mp3");
                
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

    m_model->draw_TriVAO_OBJ(sh_programID);
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

void Object::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const
{
    if (m_visible) {
        glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0), TransMatrix(1.0), ScaleMatrix(1.0), RotMatrix(1.0);
        TransMatrix = MatriuTG;

        float scaleFactor = m_radius * 2 / m_model->m_width;
        TransMatrix = translate(TransMatrix, vec3(m_x, m_model->m_height * scaleFactor * 2 / 3, m_y));
        TransMatrix = rotate(TransMatrix, m_rotation, vec3(0, 1, 0));
        TransMatrix = scale(TransMatrix, vec3(scaleFactor, scaleFactor, scaleFactor));
        ModelMatrix = TransMatrix;

        glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
        NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);

        m_model->draw_TriVAO_OBJ(sh_programID);
    }
}

void GameLogic::finalCoinAnimation(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG, float t) const
{
    int finalTime = ANIMATION_DURATION;

    t = t > finalTime ? finalTime : t;

    int x = 200 / finalTime * t;
    int y = (200 - x) * 0.05 * x;
    float alpha = TWOPI * 5 / finalTime * t;
    float beta = std::sin(TWOPI * 4 / finalTime * t) * 0.2;

    glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0), TransMatrix(1.0), ScaleMatrix(1.0), RotMatrix(1.0);

    TransMatrix = translate(TransMatrix, vec3(-x * 2, 0, -y));
    TransMatrix = translate(TransMatrix, vec3(300, 1100, 3000));
    TransMatrix = rotate(TransMatrix, float(beta), vec3(1, 0, 0));
    TransMatrix = rotate(TransMatrix, float(alpha), vec3(0, 1, 0));
    TransMatrix = rotate(TransMatrix, float(PI * !extraLife), vec3(0, 1, 0));
    TransMatrix = scale(TransMatrix, vec3(100, 100, 100));

    ModelMatrix = TransMatrix;

    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
    NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);

    modelCoin->draw_TriVAO_OBJ(sh_programID);
}
