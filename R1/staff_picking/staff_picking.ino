/*
This is the code for the staff picking for the R1 bot.
The Staff picking part of the R1 bot has 2 actuators:
  1. Servo Motor (for the arm)
  2. Pneumatic Actuator (for the claw)

I have assigned the RT button of the controller for the Staff Picking to happen

Before running the code,
  1. Make sure the solenoidPin is correct. (line 29)
  2. Change the pickAngle and the restAngle values accordingly. (lines 32 & 33)
  3. To increase or decrease the speed, pickupDelay and the clawDelay is changed. (lines 53 & 54)
  4. Make sure the Servo pin is correct. (line 64)
Each variable has inline comments for easier understanding 
*/

#include <USBHost_t36.h>
#include <Servo.h>

// USB host setup for reading the controller
USBHost myusb;
BluetoothController bluet(myusb, true, "0000");   // PS4 pairing
JoystickController joystick(myusb);

// Servo which moves the arm
Servo armServo;

// Pin connected to the solenoid for pneumatic claw
const int solenoidPin = 8;

// Arm angles
int pickAngle = 90;     // angle for the claw to reach the staff
int restAngle = 0;     // arm fully up

// RT trigger axis and threshold
const int RT_AXIS = 5; //RT is Axis 5
const int RT_THRESHOLD = 100; //RT is analog button so just puttin a threshold so that button is not pressed accidently

// States for the pickup sequence
enum State {
  IDLE,
  MOVE_TO_PICKUP,
  CLOSE_CLAW,
  MOVE_TO_REST
};

State currentState = IDLE;

// Used to handle timing between steps
unsigned long stateStartTime = 0;

// Time delays for movement and pneumatics
unsigned long pickupDelay = 1000; // time for arm to reach pickup angle (change accordingly)
unsigned long clawDelay   = 500;  // time for claw to fully close (change this also accordingly)

// Used to detect a fresh RT press
bool rtState = false;

void setup() {
  // Start USB host
  myusb.begin();

  // Setup servo pin and solenoid
  armServo.attach(10);
  pinMode(solenoidPin, OUTPUT);

  // Initial positions
  armServo.write(restAngle);       // arm starts up
  digitalWrite(solenoidPin, LOW);  // claw starts open
}

void loop() {
  // Keep USB running
  myusb.Task();

  // Exit if controller data is not available
  if (!joystick.available()) return;

  int rtValue = joystick.getAxis(RT_AXIS); // inputting value from the RT trigger
  bool rtPressed = rtValue > RT_THRESHOLD; // if the RT button is pressed more than the threshold value

  // Start pickup only on a new RT press while idle
  if (rtPressed && !rtState && currentState == IDLE) {
    armServo.write(pickAngle);     // move arm down
    currentState = MOVE_TO_PICKUP;
    stateStartTime = millis();
  }

  // Store trigger state (Pressed or Not Pressed)
  rtState = rtPressed;

  // Run the pickup sequence
  handleClawState();
}

void handleClawState() {
  unsigned long now = millis();

  switch (currentState) {
 /* im using switch-case because using a delay will block the whole program and thus even the controller inputs 
    cant be read by the bot (for moving and stuff) at the same tiem. With state we can do both simultaneously*/
    case MOVE_TO_PICKUP:
      // Wait until arm reaches pickup position
      if (now - stateStartTime >= pickupDelay) {
        digitalWrite(solenoidPin, HIGH); // close claw
        currentState = CLOSE_CLAW; //goes ot the next case that is close claw
        stateStartTime = now;
      }
      break;

    case CLOSE_CLAW:
      // Give time for the claw to grip properly
      if (now - stateStartTime >= clawDelay) {
        armServo.write(restAngle); // lift arm back up
        currentState = MOVE_TO_REST; // as the job is completed it goes to the Rest position
        stateStartTime = now;
      }
      break;

    case MOVE_TO_REST:
      // Holding the staff
      break;

    case IDLE:
    default:
      break;
  }
}
