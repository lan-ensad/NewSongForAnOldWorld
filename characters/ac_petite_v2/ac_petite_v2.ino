class Servo {
private:
  byte PIN;
  int MIN, MAX, MAXSPEED, MINSPEED;
  int prevAn;

  int duty_cycle(int t) {
    int r = map(t, 0, 180, MIN, MAX);
    return r;
  }
  int angle_speed(int t) {
    int r = map(t, 0, 100, MINSPEED, MAXSPEED);
    return r;
  }

public:
  Servo(byte PIN, int MIN, int MAX, int MINSPEED, int MAXSPEED) {
    this->PIN = PIN;
    this->MIN = MIN;
    this->MAX = MAX;
    this->MINSPEED = MINSPEED;
    this->MAXSPEED = MAXSPEED;
    pinMode(PIN, OUTPUT);
    prevAn = 0;
  }
  void Move(int angle, int motor_speed) {
    while (prevAn < angle) {
      digitalWrite(PIN, HIGH);
      delayMicroseconds(duty_cycle(angle));
      digitalWrite(PIN, LOW);
      delayMicroseconds(angle_speed(motor_speed));
      prevAn++;
    }
    while (prevAn > angle) {
      digitalWrite(PIN, HIGH);
      delayMicroseconds(duty_cycle(angle));
      digitalWrite(PIN, LOW);
      delayMicroseconds(angle_speed(motor_speed));
      prevAn--;
    }
  }
};