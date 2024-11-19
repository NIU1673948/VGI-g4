#include "GameLogic.h"

//sf::Color getRandomColor() {
//    return carColors[rand() % carColors.size()];
//}



// Implementaci� de Car
Car::Car(float x, float y, float w, float h, float speed)
    : m_x(x), m_y(y), m_height(h), m_width(w), m_speed(speed), m_visible(true)
{

    int i = rand() % 8 + 1;
    m_model = ::new COBJModel;
    m_model->netejaVAOList_OBJ();
    m_model->netejaTextures_OBJ();
    const string rutaArxiu = "..\\x64\\Release\\OBJFiles\\Car 0"+to_string(i)+"\\Car"+to_string(i)+".obj";
    char rutaCopia[1024];
    std::strncpy(rutaCopia, rutaArxiu.c_str(), sizeof(rutaCopia) - 1);
    rutaCopia[sizeof(rutaCopia) - 1] = '\0';
    m_model->LoadModel(const_cast<char*>(rutaCopia));
}

void Car::move(float dx, float dy) {
    m_x += dx;
    m_y += dy;
}

void Car::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const
{
    if (m_visible) {
        glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0), TransMatrix(1.0), ScaleMatrix(1.0), RotMatrix(1.0);
        TransMatrix = MatriuTG;

        TransMatrix = glm::translate(TransMatrix, vec3(m_x, 0, m_y)); // ALBERT mirar comentaris a Player::draw (�s el mateix)
        TransMatrix = glm::scale(TransMatrix, vec3(CAR_WIDTH/MODEL_WIDTH, CAR_WIDTH / MODEL_WIDTH, CAR_WIDTH / MODEL_WIDTH));
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

// Implementaci� de RoadRow
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
        m_obstacles[i].m_y += dy;
    }
    m_object.m_y += dy;
}

void RoadRow::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const {
    for (int i = 0; i < NUM_LANES; i++) {
        m_obstacles[i].draw(sh_programID, MatriuVista, MatriuTG);
    }
    //m_object.draw(mat4 MatriuVista, mat4 MatriuTG);
}

void RoadRow::initRow(float y, int& nextEmptyLane)
{
    int currentEmptyLane = nextEmptyLane;

    for (int i = 0; i < NUM_LANES; ++i)
    {
        //m_obstacles[i].m_color = getRandomColor();
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

    m_object.m_visible = 0 == rand() % PROB_OBJECT;

    while (m_object.m_visible) {
        int i = rand() % NUM_LANES;

        if (!m_obstacles[i].m_visible) {
            m_object.m_x = ROAD_START + LANE_WIDTH / 2 + i * LANE_WIDTH;
            m_object.m_y = y;
            m_object.m_isCoin = 0 != rand() % PROB_SHIELD;
            break;
        }
    }

}

PickUp RoadRow::getPickUp(const Player& player) const
{
    if (player.checkCollision(m_object))
        return m_object.m_isCoin ? COIN : SHIELD;

    return NONE;
}

float RoadRow::getY() const {
    return m_obstacles[0].m_y;
}

// Implementaci� de GameLogic
GameLogic::GameLogic() : gameRunning(true) {
    srand(static_cast<unsigned int>(time(nullptr)));

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
}

void GameLogic::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const
{
    //// Dibuixar elements
    //window.clear(sf::Color::Black);

    //// Dibuixar la carretera
    //sf::RectangleShape road(sf::Vector2f(ROAD_WIDTH, WINDOW_HEIGHT));
    //road.setFillColor(sf::Color(50, 50, 50));
    //road.setPosition(ROAD_START, 0);
    //window.draw(road);

    road.draw(sh_programID, MatriuVista, MatriuTG);
    player.draw(sh_programID, MatriuVista, MatriuTG);  // Dibuixar el jugador
    for (int i = 0; i < NUM_ROWS; i++) {
        roadRows[i].draw(sh_programID, MatriuVista, MatriuTG);  // Dibuixar cada fila
    }
}

void GameLogic::GetUserInput()
{
    if (GetKeyState('A') & 0x8000) { //Nom�s per a Windows ALBERT
        player.move(-STEP, 0);

        player.rotate(player.m_rotation > -ROTATION_ANGLE ? -ROTATION_SPEED : 0);

        if (player.m_x < ROAD_START + player.m_width / 2)
        {
            player.move(ROAD_START + player.m_width / 2 - player.m_x, 0);
            player.rotate(player.m_rotation < 0 ? ROTATION_SPEED : 0);
        }
    }
    else if (GetKeyState('D') & 0x8000) {
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
        PickUp p = roadRows[i].getPickUp(player);
        if (p != NONE)
        {
            roadRows[i].m_object.m_visible = false;
            if (p == COIN)
            {
                player.m_speed = PLAYER_SPEED > player.m_speed - 0.2f ? PLAYER_SPEED : player.m_speed - 0.2f;
            }
        }
    }
}

// Implementaci� Circle
//void Circle::draw(sf::RenderWindow& window) const
//{
//    if (m_visible)
//    {
//        sf::CircleShape circle(m_radius);
//        circle.setOrigin(m_radius, m_radius);
//        circle.setPosition(m_x, m_y);
//        circle.setFillColor(m_isCoin? sf::Color::Yellow : sf::Color(128,128,128));
//        window.draw(circle);
//    }
//}

// Implementaci� Player
Player::Player() : Car::Car(ROAD_START + ROAD_WIDTH / 2, WINDOW_HEIGHT - CAR_HEIGHT / 2 - MARGIN)
{
    m_rotation = 0;

    for (int i = 0; i < NUM_CIRCLES; i++)
    {
        m_collisionCircles[i] = Circle(m_x, m_y - m_height / 4 + m_height / 4 * i, m_width / 2, false);
    }
}

void Player::move(float dx, float dy)
{
    Car::move(dx, dy);
    for (int i = 0; i < NUM_CIRCLES; i++)
    {
        m_collisionCircles[i].m_x += dx;
    }
}

void Player::rotate(float dAngle)
{
    m_rotation += dAngle;
    for (int i = 0; i < NUM_CIRCLES; i++)
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
    TransMatrix = glm::translate(TransMatrix, vec3(-m_width / 2, 0, -m_height / 4));
    TransMatrix = glm::rotate(TransMatrix, m_rotation, vec3(0, -1, 0));
    TransMatrix = glm::translate(TransMatrix, vec3(m_width / 2, 0, m_height / 4)); 
    TransMatrix = glm::rotate(TransMatrix, float(PI), vec3(0, 1, 0));
    TransMatrix = glm::scale(TransMatrix, vec3(CAR_WIDTH/MODEL_WIDTH, CAR_WIDTH / MODEL_WIDTH, CAR_WIDTH / MODEL_WIDTH));
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
        for (int i = 0; i < NUM_CIRCLES; i++)
        {
            const Circle& playerCircle = m_collisionCircles[i];

            glm::vec2 center(playerCircle.m_x, playerCircle.m_y);
            glm::vec2 aabb_half_extents(obstacle.m_width / 2.0f, obstacle.m_height / 2.0f);
            glm::vec2 aabb_center(obstacle.m_x, obstacle.m_y);

            glm::vec2 difference = center - aabb_center;
            glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
            glm::vec2 closest = aabb_center + clamped;
            difference = closest - center;

            if (glm::length(difference) < playerCircle.m_radius)
                return true;
        }
    }
    return false;
}

bool Player::checkCollision(const Circle& object) const {
    if (object.m_visible)
    {
        for (int i = 0; i < NUM_CIRCLES; i++)
        {
            const Circle& playerCircle = m_collisionCircles[i];
            glm::vec2 difference(playerCircle.m_x - object.m_x, playerCircle.m_y - object.m_y);

            if (glm::length(difference) < playerCircle.m_radius + object.m_radius)
                return true;
        }
    }
    return false;
}



//ROAD
Road::Road() { //Per defecte es genera amb aquestes dimensions
    float w = ROAD_WIDTH;
    float h = WINDOW_HEIGHT;
    vertices = {
        0.0f, 0.0f,  -h, 0.0f, 0.0f,    // V�rtex inferior esquerre
         w,  0.0f,  -h, 1.0f, 0.0f,    // V�rtex inferior dret
        0.0f, 0.0f, h, 0.0f, 1.0f,       // V�rtex superior esquerre
         w,  0.0f, h, 1.0f, 1.0f         // V�rtex superior dret
    };

    indices = {
        0, 1, 2, // Primer tri�ngulo
        2, 1, 3  // Segundo tri�ngulo
    };

    setupMesh();
}

Road::~Road() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Road::setRoadSize(const float& width, const float& length) {
    vertices = {
        -width / 2, 0.0f,  0.0f, 0.0f, 0.0f,  // V�rtice inferior izquierdo
         width / 2, 0.0f,  0.0f, 1.0f, 0.0f,  // V�rtice inferior derecho
        -width / 2, 0.0f, -length, 0.0f, 1.0f, // V�rtice superior izquierdo
         width / 2, 0.0f, -length, 1.0f, 1.0f  // V�rtice superior derecho
    };

    indices = {
        0, 1, 2, // Primer tri�ngulo
        2, 1, 3  // Segundo tri�ngulo
    };
}

void Road::setupMesh() {


    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Road::draw(GLuint sh_programID, glm::mat4 MatriuVista, glm::mat4 MatriuTG) const {
    glm::mat4 NormalMatrix(1.0), ModelMatrix(1.0), TransMatrix(1.0), ScaleMatrix(1.0), RotMatrix(1.0);

    TransMatrix = MatriuTG;
    // Desplacem la carretera intentant deixar el cotxe en mig
    TransMatrix = glm::translate(MatriuTG, glm::vec3(5.0f, 0.0f, 15.0f));
    //ModelMatrix = glm::scale(TransMatrix, vec3(12 * 0.583f, 0.1f, 0.3f));
    ModelMatrix = TransMatrix;

    // Pas ModelView Matrix a shader
    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "modelMatrix"), 1, GL_FALSE, &ModelMatrix[0][0]);
    NormalMatrix = transpose(inverse(MatriuVista * ModelMatrix));
    // Pas NormalMatrix a shader
    glUniformMatrix4fv(glGetUniformLocation(sh_programID, "normalMatrix"), 1, GL_FALSE, &NormalMatrix[0][0]);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}