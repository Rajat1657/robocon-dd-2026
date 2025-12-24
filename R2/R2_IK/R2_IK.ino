/* 
This is the code for controlling the R2 bot, that is a X config Omni Drive
The left joystick move the bot front, back, left and right, while the right joystick rotates the bot CW or CCW.
*/

#include <USBHost_t36.h>

int FL_LPWM = 0, FL_RPWM = 1; // FRONT LEFT 
int FR_LPWM = 4, FR_RPWM = 5;
int BL_LPWM = 22,  BL_RPWM = 23;
int BR_LPWM = 13,  BR_RPWM = 12;

// USB Host objects
USBHost myusb;
BluetoothController bluet(myusb, true, "0000");   // PS4 pairing
JoystickController joystick(myusb);

const int pwmResolution = 14; // PWM Values are from 0 to 16384 (14bit resolution)
const int maxPWM = 500; // Change for higher speeds

float maxSpeed = 1.0;

// ZERO drift
int deadZone = 25;

void setup() {
  Serial.begin(115200);
  myusb.begin();
  analogWriteResolution(pwmResolution);

  pinMode(FL_LPWM, OUTPUT); pinMode(FL_RPWM, OUTPUT);
  pinMode(FR_LPWM, OUTPUT); pinMode(FR_RPWM, OUTPUT);
  pinMode(BL_LPWM, OUTPUT); pinMode(BL_RPWM, OUTPUT);
  pinMode(BR_LPWM, OUTPUT); pinMode(BR_RPWM, OUTPUT);

  Serial.println("READY — Waiting for PS4 controller...");
}

void loop() {
  myusb.Task();  // USB/Bluetooth handling

  if (!joystick.available()) {
    stopAll();
    return;
  }

  int lx = joystick.getAxis(0) - 128;   // left horizontal
  int ly = joystick.getAxis(1) - 128;   // left vertical
  int rx = joystick.getAxis(2) - 128;   // right horizontal

  // Deadzone
  if (abs(lx) < deadZone) lx = 0;
  if (abs(ly) < deadZone) ly = 0;
  if (abs(rx) < deadZone) rx = 0;

  // Convert
  float Vy =  lx / 128.0 * maxSpeed;   // Strafe
  float Vx = -ly / 128.0 * maxSpeed;   // Forward
  float w  =  rx / 128.0 * maxSpeed;   // Rotation

  // X-OMNI IK Equation
  float wFL =  (Vx + Vy - w);
  float wFR =  (Vx - Vy + w);
  float wBL =  (Vx - Vy - w);
  float wBR =  (Vx + Vy + w);

  setMotor(FL_LPWM, FL_RPWM, wFL);
  setMotor(FR_LPWM, FR_RPWM, wFR);
  setMotor(BL_LPWM, BL_RPWM, wBL);
  setMotor(BR_LPWM, BR_RPWM, wBR);

}

void setMotor(int lpwm, int rpwm, float spd) {
  int pwm = abs(spd) * maxPWM;
  if (pwm < 200) pwm = 0;

  if (spd > 0) {
    analogWrite(lpwm, pwm);
    analogWrite(rpwm, 0);
  } 
  else if (spd < 0) {
    analogWrite(lpwm, 0);
    analogWrite(rpwm, pwm);
  }
  else {
    analogWrite(lpwm, 0);
    analogWrite(rpwm, 0);
  }
}

void stopAll() {
  analogWrite(FL_LPWM, 0); analogWrite(FL_RPWM, 0);
  analogWrite(FR_LPWM, 0); analogWrite(FR_RPWM, 0);
  analogWrite(BL_LPWM, 0); analogWrite(BL_RPWM, 0);
  analogWrite(BR_LPWM, 0); analogWrite(BR_RPWM, 0);
}
