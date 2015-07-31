/*
 * Copyright (c) 2015 Vrije Universiteit Brussel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <ros/ros.h>
#include <std_msgs/Int32.h>
#include <std_msgs/Float32.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <sstream>
#include <iostream>
#include <string.h>

static ros::Subscriber sub_leftmotor;
static ros::Subscriber sub_rightmotor;

static ros::Publisher pub_leftspeed;
static ros::Publisher pub_rightspeed;
static ros::Publisher pub_ir[11];
static ros::Publisher pub_us[5];

static int sock;
static char buffer[200];

template<int index>
void set_motor_torque(const std_msgs::Float32::ConstPtr &torque)
{
    // Write the "setTorque <motor> <torque>" to the socket
    std::ostringstream command;

    command << "setTorque " << index << ' ' << int(torque->data * 32768) << std::endl;

    write(sock, command.str().data(), command.str().size());

    // Read the respose, a bunch of numbers
    ssize_t size = read(sock, buffer, 200);
    std::istringstream response(std::string(buffer, size));
    int values[18];
    
    for (int i=0; i<18; ++i) {
        response >> values[i];
    }

    // Publish everything
    std_msgs::Int32 msg;
    
    msg.data = values[0];
    pub_leftspeed.publish(msg);
    msg.data = values[1];
    pub_rightspeed.publish(msg);
    
    for (int i=0; i<11; ++i) {
        msg.data = values[i + 2];
        pub_ir[i].publish(msg);
    }

    for (int i=0; i<5; ++i) {
        if (values[i + 13] > 0) {
            // Sometimes, an immediate reading is not available. Don't publish
            // anything when this happen and let ROS expose the old value.
            msg.data = values[i + 13];
            pub_us[i].publish(msg);
        }
    }
}

int main(int argc, char **argv)
{
    // Check arguments
    if (argc != 2) {
        std::cerr << "Usage: roskhepera <ip address of robot>" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Make sure that the robot is turned on, has the server installed and running, and that this computer is connected to the khepWaves ad-hoc Wifi network." << std::endl;

        return 1;
    }

    ros::init(argc, argv, "khepera");
    ros::NodeHandle node("khepera");

    // Subscriber for the motor topics
    sub_leftmotor = node.subscribe<std_msgs::Float32>("leftTorque", 1, &set_motor_torque<0>);
    sub_rightmotor = node.subscribe<std_msgs::Float32>("rightTorque", 1, &set_motor_torque<1>);
    
    // Publishers
    char template_ir[] = "infraredIntensity00";
    char template_us[] = "ultrasonicDistanceCM0";

    pub_leftspeed = node.advertise<std_msgs::Int32>("leftSpeed", 1);
    pub_rightspeed = node.advertise<std_msgs::Int32>("rightSpeed", 1);
    
    for (int i=0; i<11; ++i) {
        template_ir[18] = '0' + (i % 10);
        template_ir[17] = '0' + (i / 10);

        pub_ir[i] = node.advertise<std_msgs::Int32>(template_ir, 1);
    }

    for (int i=0; i<5; ++i) {
        template_us[20] = '0' + i;

        pub_us[i] = node.advertise<std_msgs::Int32>(template_us, 1);
    }
    
    // Open a connection to the Khepera robot
    sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));

    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1993);

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (sock < 0 || connect(sock, (sockaddr *)&addr, sizeof(sockaddr)) < 0) {
        perror("Unable to connect to the Khepera robot");
        return EXIT_FAILURE;
    }

    // Let ROS do its job
    ros::spin();

    close(sock);
    return 0;
}
