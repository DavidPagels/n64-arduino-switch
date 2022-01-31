#include <N64Controller.h>

N64Controller player1 (2); // this controller for player one is on PIN 12

void setup() {
    Serial.begin(115200);
    player1.begin(); // Initialisation
}

void loop() {
    player1.update(); // read key state
    uint8_t firstByte = (player1.A() << 7) | (player1.B() << 6) | (player1.Z() << 5) | (player1.Start() << 4) | (player1.D_up() << 3) | (player1.D_down() << 2) | (player1.D_left() << 1) | player1.D_right();
    uint8_t secondByte = (player1.L() << 5) | (player1.R() << 4) | (player1.C_up() << 3) | (player1.C_down() << 2) | (player1.C_left() << 1) | player1.C_right();
    uint8_t thirdByte = player1.axis_x();
    uint8_t fourthByte = player1.axis_y();
    Serial.write(firstByte);
    Serial.write(secondByte);
    Serial.write(thirdByte);
    Serial.write(fourthByte);
    delay(10);
}
