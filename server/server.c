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

#include <korebot/korebot.h>
#include <stdio.h>

static knet_dev_t *dsPic;
static knet_dev_t *motor1;
static knet_dev_t *motor2;

static char buf[200];

// This function initializes a motor of the Khepera 3 robot. It is largely inspired
// from the function "initMot" in khepera3_test.c (libkorebot)
static void motor_init(knet_dev_t *motor)
{
    kmot_SetMode(motor, kMotModeIdle);
    kmot_SetSampleTime(motor, 1550);
    kmot_SetMargin(motor, 20);
    
    if (motor == motor1) {
        // Invert the direction of one of the motors
        kmot_SetOptions(motor, 0, kMotSWOptWindup | kMotSWOptStopMotorBlk | kMotSWOptDirectionInv);
    } else {
        kmot_SetOptions(motor, 0, kMotSWOptWindup | kMotSWOptStopMotorBlk);
    }
    
    kmot_ResetError(motor);
    kmot_SetBlockedTime(motor, 10);
    kmot_SetLimits(motor, kMotRegCurrent, 0, 500);
}

/**
 * @brief Read two bytes in a buffer and arrange them in an integer
 *
 * @param buf Buffer from which the data is read
 * @param offset Offset (in bytes) at which reading should start
 */
static int read_int(const char *buf, int offset)
{
    return (int)buf[offset] | ((int)buf[offset + 1] << 8);
}

/**
 * @brief Set the torque of a motor
 *
 * Usage: setTorque [0 (left) or 1 (right)] [-32767 to 32767]
 *
 * Return a sequence of numbers representing the readings from the sensors. The
 * numbers are in this order:
 * 
 * - Speed of left motor, speed of right motor
 * - IR back left, IR left (under the Khepera logo), ..., IR back right
 * - Line sensor right, line sensor left
 * - Distance in cm as measured by the far left, left, front, ..., far right sonar
 */
static int cmd_settorque(int argc, char **argv, void *reply)
{
    // Set the torque of the motor
    int motor = atoi(argv[1]);
    int pwm = atoi(argv[2]);

    kmot_SetPoint(motor ? motor2 : motor1, kMotRegOpenLoop, pwm);

    // Read the motor speed
    int values[18];

    memset(values, 0, sizeof(values));
    
    values[0] = kmot_GetMeasure(motor1, kMotRegSpeed);
    values[1] = kmot_GetMeasure(motor2, kMotRegSpeed);

    // Read the IR values
    if (kh3_proximity_ir(buf, dsPic)) {
        for (int i=0; i<11; ++i) {
            values[i + 2] = read_int(buf, i * 2 + 1);
        }
    }

    // Read the sonar values
    for (int sonar=0; sonar<5; ++sonar) {
        if (kh3_measure_us(buf, sonar + 1, dsPic)) {
            int num_echo = read_int(buf, 1);

            if (num_echo > 0) {
                values[sonar + 13] = read_int(buf, 3);
            }
        }
    }

    // Send the response
    char *str = buf;
    int offset = 0;

    for (int i=0; i<18; ++i) {
        offset += snprintf(str + offset, 200 - offset, "%i ", values[i]);
    }

    ksock_send_answer((int *)reply, buf);

    return 0;
}

int main(int argc, char *argv[]) {
    int rc;

    // Set the libkorebot debug level - Highly recommended for development.
    kb_set_debug_level(2);
  
    // Init the korebot library
    if (kh3_init() < 0 || ksock_init('\n', 200) < 0) {
        return 1;
    }

    // Open the devices needed for controlling the Khepera robot
    dsPic = knet_open("Khepera3:dsPic", KNET_BUS_I2C, 0, NULL);
    motor1 = knet_open("Khepera3:mot1", KNET_BUS_I2C, 0, NULL);
    motor2 = knet_open("Khepera3:mot2", KNET_BUS_I2C, 0, NULL);
    
    // Initialize the two motors
    motor_init(motor1);
    motor_init(motor2);
    
    // Initialize the ultrasonic sensors
    kh3_configure_os(buf, 0, 31, dsPic);

    // Start to listen for connections
    ksock_t server;
    
    ksock_server_open(&server, 1993);
    
    // Commands recognized by the Khepera3 robot
    ksock_add_command("setTorque", 2, 2, cmd_settorque);
    list_command();
    
    // Handle one client at a time
    while (1) {
        int client = ksock_next_connection(&server);
        int ok = 1;
        
        while (ok) {
            int rc = ksock_get_command(client, buf, sizeof(buf) - 1);
            
            if (rc > 0) {
                ksock_exec_command_pending(client, buf);
            } else if (rc < -1) {
                // Error that is not "try again, command incomplete", discard this
                // client.
                ok = 0;
            }
        }
        
        // Close the client socket and wait for another client
        close(client);
    }

    return 0;  
}

