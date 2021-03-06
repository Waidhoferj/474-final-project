#include "SpaceGame.h"

void PlayerShip::update(vec2 mousePos, double dt)
{
    if (state == Flying || state == Delivering)
        updatePos(mousePos, dt);
    time_left -= dt;
    if (time_left < 0.0)
    {
        time_left = 0.0;
        state = Exploding;
    }
    bool withinDeliveryDist = glm::length(destination->pos - pos) < scale + destination->scale + 1.0;
    if (withinDeliveryDist)
    {
        state = Delivering;
    }
}

void PlayerShip::updatePos(vec2 mousePos, double dt)
{
    vec2 mouseDif = mousePos - vec2(0.5);
    static vec2 smoothDif = mouseDif;
    smoothDif += (mouseDif - smoothDif) * vec2(0.07);
    double spdFact = abs(glm::min(glm::length(mouseDif * vec2(2)), 1.0f));
    static double smoothSpdFact = 0.0;
    smoothSpdFact += (spdFact - smoothSpdFact) * 0.5;
    double angle = atan2(-mouseDif.y, mouseDif.x);
    quat shipRot = quat(rotate(mat4(1.0), (float)angle, vec3(0, 1, 0)));
    static quat smoothDir = quat(mat4(1.0));
    smoothDir = slerp(smoothDir, shipRot, 0.07f);
    rotMat = mat4(smoothDir);
    vec3 spd = vec3(0.2);
    static vec3 shipPos = vec3(0.0);
    vec3 smoothAngles = eulerAngles(smoothDir);
    vec3 shipDir = vec3(cos(smoothDif.x < 0 ? acos(0) * 2 - smoothAngles.y : smoothAngles.y), 0, -sin(smoothAngles.y));
    vec3 v = shipDir * spd * vec3(smoothSpdFact);
    pos += v;

    // draw particles
    static int part_counter = 0;
    part_counter++;
    vec3 exhaust_pos = pos - shipDir * vec3(0.6);
    float r = (float)rand() / (float)RAND_MAX;
    vec3 exhaust_vel = -v * vec3(1.5);
    exhaust_vel.y += (r * 2.0f - 1.0f) * 0.08f * smoothSpdFact;
    if (exhaust.size() < 50 && part_counter > 10)
    {
        part_counter = 0;
        Particle p;
        p.life = 0.0;
        exhaust.push_back(p);
    }

    for (auto &p : exhaust)
    {
        bool shouldRespawn = p.update(dt);
        if (shouldRespawn)
            p.respawn(exhaust_pos, exhaust_vel);
    }
}

void PlayerShip::chooseNewDestination(vector<Planet> &planets)
{
    Planet *newP;
    do
    {
        int i = ((double)rand()) / RAND_MAX * (planets.size() - 1);
        newP = &planets[i];
    } while (newP == destination);
    destination = newP;
}

bool PlayerShip::intersects(Asteroid &asteroid)
{
    return glm::length(glm::distance(asteroid.pos, pos)) <= asteroid.scale + scale && state == Flying;
}
bool PlayerShip::intersects(Planet &planet)
{
    return glm::length(glm::distance(planet.pos, pos)) <= planet.scale + scale && state == Flying;
}

void PlayerShip::respawn()
{
    points = 0;
    opacity = 1.0;
    pos = vec3(0);
    time_left = 30.0;
    state = Flying;
}

Planet::Planet()
{
    float rand_axis = (float)rand() / RAND_MAX * (float)acos(0) / 4.0f;
    rot_axis = quat(glm::rotate(mat4(1.0), rand_axis, vec3(0, 0, 1)));
    scale = (double)rand() / RAND_MAX * 1.5 + 0.5;
}

Planet::Planet(vec3 position)
{
    float rand_axis = (float)rand() / RAND_MAX * (float)acos(0) / 4.0f;
    rot_axis = quat(glm::rotate(mat4(1.0), rand_axis, vec3(0, 0, 1)));
    scale = (double)rand() / RAND_MAX * 1.5 + 0.5;
    pos = position;
}
void Planet::update(double time)
{
    quat spin = quat(glm::rotate(mat4(1.0), (float)time, vec3(0, 1, 0)));
    rotMat = mat4(spin * rot_axis);
}

void Asteroid::setPath(vector<vec3> points)
{
    spline(path, points, 200, 2);
}

Asteroid::Asteroid()
{
    float rand_axis = (float)rand() / RAND_MAX * (float)acos(0) / 4.0f;
    rotScalar = (float)rand() / RAND_MAX * 2.0;
    speedScalar = (float)rand() / RAND_MAX + 0.1;
    rot_axis = quat(glm::rotate(mat4(1.0), rand_axis, vec3(0, 0, 1)));
    scale = (double)rand() / RAND_MAX * 0.1 + 0.2;
}

void Asteroid::update(double time)
{
    time = time * speedScalar;
    double anim_time = time * path.size() / 10.0;
    int anim_step = (int)anim_time;
    double interp_t = anim_time - (double)anim_step;

    // Interpolate along path
    int point_index = anim_step % (path.size() - 1);

    vec3 cur_point = path[point_index];
    vec3 next_point = path[point_index + 1];
    pos = cur_point * glm::vec3(1.0 - interp_t) + next_point * glm::vec3(interp_t);
}

Particle::Particle()
{
    scale = 1.0;
    position = vec3(0, 0, 0);
    life = lifetime;
    velocity = vec3(0);
}
Particle::Particle(vec3 pos, vec3 vel)
{
    scale = 1.0;
    position = vec3(0, 0, 0);
    life = lifetime;
    velocity = vel;
}

void Particle::respawn(vec3 pos, vec3 vel)
{
    position = pos;
    velocity = vel;
    life = lifetime;
}

bool Particle::update(float dt)
{
    life -= dt;
    scale = life / lifetime;
    if (life <= 0.0)
        return true;
    position += velocity;
    return false;
}
