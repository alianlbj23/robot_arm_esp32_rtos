#include <armDriver.hpp>

void ArmManager::setServoAngle(uint8_t servoNum, float angle) {
    if (servoNum < this->numServos) {
        this->servos[servoNum].write(angle);  // 直接使用ESP32Servo庫控制
    }
}

ArmManager::ArmManager(
    const uint8_t numServos, const uint8_t servoPins[],
    const uint8_t servoMinAngles[], const uint8_t servoMaxAngles[],
    const uint8_t servoInitAngles[]) {
    // 初始化成員變數
    this->numServos = numServos;

    // 分配記憶體
    this->servos = new Servo[numServos];
    this->servoPins = new uint8_t[numServos];
    this->servoTargetAngles = new uint8_t[numServos];
    this->servoCurrentAngles = new float[numServos];
    this->servoMinAngles = new uint8_t[numServos];
    this->servoMaxAngles = new uint8_t[numServos];

    // 設置並初始化每個伺服馬達
    for (uint8_t i = 0; i < numServos; ++i) {
        this->servoPins[i] = servoPins[i];
        this->servoMinAngles[i] = servoMinAngles[i];
        this->servoMaxAngles[i] = servoMaxAngles[i];
        this->servoTargetAngles[i] = servoInitAngles[i];

        // 將當前角度設為初始角度+1，確保啟動時有動作
        this->servoCurrentAngles[i] = (float)servoInitAngles[i] + 1;

        // 初始化伺服馬達：連接到指定GPIO引腳並設置PWM範圍
        this->servos[i].setPeriodHertz(50);  // 設置PWM頻率為50Hz
        this->servos[i].attach(servoPins[i], SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);
    }
}

ArmManager::~ArmManager() {
    // 釋放記憶體
    for (uint8_t i = 0; i < this->numServos; ++i) {
        this->servos[i].detach();
    }
    delete[] this->servos;
    delete[] this->servoPins;
    delete[] this->servoTargetAngles;
    delete[] this->servoCurrentAngles;
    delete[] this->servoMinAngles;
    delete[] this->servoMaxAngles;
}

// 其他函數內容不變
void ArmManager::setServoTargetAngle(uint8_t servoNum, uint8_t targetAngle) {
    if (servoNum < this->numServos) {
        this->servoTargetAngles[servoNum] = constrain(targetAngle, servoMinAngles[servoNum],
                                                      servoMaxAngles[servoNum]);
    }
}

void ArmManager::changeServoTargetAngle(uint8_t servoNum, int8_t biasAngle) {
    if (servoNum < this->numServos) {
        this->servoTargetAngles[servoNum] = constrain(servoTargetAngles[servoNum] + biasAngle,
                                                      servoMinAngles[servoNum], servoMaxAngles[servoNum]);
    }
}

void ArmManager::getCurrentAngles(float currentAngles[]) {
    for (uint8_t i = 0; i < this->numServos; ++i) {
        currentAngles[i] = servoCurrentAngles[i];
    }
}

void ArmManager::moveArm() {
    // moveArm函數內容保持不變
    uint8_t i = 0;
    for (; i < HAND_BIAS; ++i) {
        if (abs(this->servoCurrentAngles[i] - this->servoTargetAngles[i]) >= ARM_MOVEMENT_STEP) {
            float step = (this->servoTargetAngles[i] > this->servoCurrentAngles[i]) ? ARM_MOVEMENT_STEP : -ARM_MOVEMENT_STEP;
            this->servoCurrentAngles[i] += step;
        } else {
            this->servoCurrentAngles[i] = this->servoTargetAngles[i];
        }
        setServoAngle(i, this->servoCurrentAngles[i]);
    }
    for (; i < this->numServos; ++i) {
        if (abs(this->servoCurrentAngles[i] - this->servoTargetAngles[i]) >= HAND_MOVEMENT_STEP) {
            float step = (this->servoTargetAngles[i] > this->servoCurrentAngles[i]) ? HAND_MOVEMENT_STEP : -HAND_MOVEMENT_STEP;
            this->servoCurrentAngles[i] += step;
        } else {
            this->servoCurrentAngles[i] = this->servoTargetAngles[i];
        }
        setServoAngle(i, this->servoCurrentAngles[i]);
    }
}
