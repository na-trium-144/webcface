#include <iostream>
#include <webcface/webcface.hpp>
#include <cmath>
#include <vector>
#include <cassert>
#include <random>
#include <array>
#include <thread>
#include <chrono>

std::random_device rnd;
std::mt19937 mt(rnd());

using LrfPointT = std::array<uint16_t, 2>;
using LrfDataT = std::vector<LrfPointT>;
using RobotPosT = std::array<float, 3>;
using RobotVelT = std::array<float, 3>;


LrfDataT get_lrf_data(const RobotPosT& robot_pos)
{
    assert(robot_pos.size() == 3);
    constexpr int n = 800;
    constexpr int n4 = n / 4;
    constexpr int randmax = 100;
    std::uniform_int_distribution<> randomInt(0, randmax);
    LrfDataT pts;

    const int corners[] = {-2000 + (int)robot_pos[0], -3000 + (int)robot_pos[1],
        5000 + (int)robot_pos[0], 5000 + (int)robot_pos[1]};

    for (int i = 0; i < n4; i++) {
        const LrfPointT p1{
            (uint16_t)(corners[0] + ((float)i / n4) * (corners[2] - corners[0]) + randomInt(mt)),
            (uint16_t)(corners[1] + randomInt(mt))};
        const LrfPointT p2{
            (uint16_t)(corners[0] + ((float)i / n4) * (corners[2] - corners[0]) + randomInt(mt)),
            (uint16_t)(corners[3] + randomInt(rnd))};
        const LrfPointT p3{
            (uint16_t)(corners[0] + randomInt(mt)),
            (uint16_t)(corners[1] + ((float)i / n4) * (corners[3] - corners[1]) + randomInt(mt)),
        };
        const LrfPointT p4{
            (uint16_t)(corners[2] + randomInt(mt)),
            (uint16_t)(corners[1] + ((float)i / n4) * (corners[3] - corners[1]) + randomInt(mt)),
        };
        pts.push_back(p1);
        pts.push_back(p2);
        pts.push_back(p3);
        pts.push_back(p4);
    }
    return pts;
}

RobotPosT get_robot_pos()
{
    std::uniform_int_distribution<> randX(0, 7000);
    std::uniform_int_distribution<> randY(0, 5000);
    std::uniform_int_distribution<> randTheta(0, 628);
    return {(float)randX(mt), (float)randY(mt), (float)randTheta(mt) / 628};
}

RobotVelT get_robot_vel()
{
    std::uniform_int_distribution<> randX(-100, 100);
    std::uniform_int_distribution<> randY(-100, 100);
    std::uniform_int_distribution<> randTheta(0, 628);
    return {(float)randX(mt), (float)randY(mt), (float)randTheta(mt) / 628};
}


int main()
{
    WebCFace::initStdLogger();
    LrfDataT lrf{};
    RobotPosT robot_pos{0, 0, 0};
    RobotVelT robot_vel{0, 0, 0};
    WebCFace::startServer(3001);
    WebCFace::addSharedVarFromRobot("lrf", lrf);
    WebCFace::addSharedVarFromRobot("robot_pos", robot_pos);
    WebCFace::addSharedVarFromRobot("robot_vel", robot_vel);
    while (true) {
        {
            std::lock_guard l(WebCFace::callback_mutex);
            robot_pos = get_robot_pos();
            robot_vel = get_robot_vel();
            lrf = get_lrf_data(robot_pos);
            WebCFace::sendData();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
    }
}
