constexpr int driveIn1{12};
constexpr int driveIn2{13};
constexpr int driveOut1{2};
constexpr int driveOut2{15};

void setup() {
  pinMode(driveIn1, INPUT_PULLDOWN);
  pinMode(driveIn2, INPUT_PULLDOWN);
  pinMode(driveOut1, OUTPUT);
  pinMode(driveOut2, OUTPUT);
}

void loop() {
  digitalWrite(driveOut1, digitalRead(driveIn1));
  digitalWrite(driveOut1, digitalRead(driveIn2));
}
