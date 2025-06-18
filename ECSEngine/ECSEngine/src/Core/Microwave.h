#include <iostream>
#include <functional>
#include <string>
#include <algorithm>
#include <vector>

class Microwave {
public:
    enum State {
        IDLE,
        COOKING,
        COOKING_COMPLETE,
        BROKEN 
    };

    enum DoorState {
        OPEN,
        CLOSED
    };

    struct Time {
        int minutes;
        int seconds;

        Time(int m = 0, int s = 0) : minutes(m), seconds(s) {}

        static Time fromTotalSeconds(int totalSeconds) {
            return Time(totalSeconds / 60, totalSeconds % 60);
        }
    };

    Microwave();

    void startCooking();
    void stopCooking();
    void openDoor();
    void closeDoor();

    void inputNumber(int number);
    void clearInput();

    void breakMicrowave();
    void repairMicrowave();

    State getCurrentState() const;
    DoorState getDoorState() const;
    Time getRemainingTime() const;
    bool isLightOn() const;

    void tick();

    std::string getStateName() const;
    std::string getDoorStateName() const;

private:
    State currentState;
    DoorState doorState;
    bool lightOn;
    int remainingSeconds;

    std::vector<int> inputBuffer;

    void updateLightState();
    bool canStartCooking() const;
    void resetTimer();
    void applyInputToTimer();
};