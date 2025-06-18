#include "Microwave.h"
#include <iostream>
#include <iomanip>
#include <cmath> 

Microwave::Microwave()
    : currentState(IDLE),
    doorState(CLOSED),
    lightOn(false),
    remainingSeconds(0)
{
    updateLightState();
}

Microwave::State Microwave::getCurrentState() const {
    return currentState;
}

Microwave::DoorState Microwave::getDoorState() const {
    return doorState;
}

Microwave::Time Microwave::getRemainingTime() const {
    if (currentState == BROKEN) {
        return Time(-1, -1);
    }
    if (currentState == IDLE || currentState == COOKING) {
        return Time::fromTotalSeconds(remainingSeconds);
    }
    else {
        return Time(0, 0);
    }
}

bool Microwave::isLightOn() const {
    return lightOn && currentState != BROKEN;
}


std::string Microwave::getStateName() const {
    switch (currentState) {
    case IDLE: return "IDLE";
    case COOKING: return "COOKING";
    case COOKING_COMPLETE: return "COOKING_COMPLETE";
    case BROKEN: return "BROKEN";
    default: return "UNKNOWN";
    }
}

std::string Microwave::getDoorStateName() const {
    switch (doorState) {
    case OPEN: return "OPEN";
    case CLOSED: return "CLOSED";
    default: return "UNKNOWN";
    }
}

void Microwave::updateLightState() {
    lightOn = (currentState == COOKING);
}

bool Microwave::canStartCooking() const {
    return currentState == IDLE && doorState == CLOSED && remainingSeconds > 0 && currentState != BROKEN;
}

void Microwave::resetTimer() {
    remainingSeconds = 0;
    inputBuffer.clear();
}

void Microwave::applyInputToTimer() {
    int minutesPart = 0;
    int secondsPart = 0;

    if (inputBuffer.size() >= 2) {
        secondsPart = inputBuffer[inputBuffer.size() - 2] * 10 + inputBuffer[inputBuffer.size() - 1];
    }
    else if (inputBuffer.size() == 1) {
        secondsPart = inputBuffer[0];
    }

    if (inputBuffer.size() > 2) {
        int minutesPowerOfTen = 1;
        for (size_t i = inputBuffer.size() - 3; ; --i) {
            minutesPart += inputBuffer[i] * minutesPowerOfTen;
            minutesPowerOfTen *= 10;
            if (i == 0) break;
        }
    }

    minutesPart += secondsPart / 60;
    secondsPart %= 60;

    remainingSeconds = minutesPart * 60 + secondsPart;
}

void Microwave::inputNumber(int number) {
    if (currentState == COOKING || currentState == BROKEN) {
        return;
    }
    if (doorState == OPEN) {
        return;
    }

    if (remainingSeconds == 0 && inputBuffer.empty()) {
        inputBuffer.clear();
        currentState = IDLE;
    }
    else if (currentState == COOKING_COMPLETE) {
        resetTimer();
        currentState = IDLE;
    }

    const size_t MAX_INPUT_DIGITS = 4;
    if (inputBuffer.size() < MAX_INPUT_DIGITS) {
        inputBuffer.push_back(number);
        applyInputToTimer();
    }
    else {
    }
}

void Microwave::clearInput() {
    if (currentState == COOKING || currentState == BROKEN) {
        return;
    }
    resetTimer();
    currentState = IDLE;
    updateLightState();
}

void Microwave::openDoor() {
    if (doorState == OPEN) {
        return;
    }

    doorState = OPEN;

    if (currentState == COOKING) {
        currentState = IDLE;
    }
    else if (currentState == COOKING_COMPLETE) {
        currentState = IDLE;
        resetTimer();
    }

    updateLightState();
}

void Microwave::closeDoor() {
    if (doorState == CLOSED) {
        return;
    }

    doorState = CLOSED;
    updateLightState();
}

void Microwave::startCooking() {
    if (currentState == BROKEN) {
        return;
    }

    if (!canStartCooking()) {
        return;
    }

    currentState = COOKING;
    inputBuffer.clear();
    updateLightState();
}

void Microwave::stopCooking() {
    if (currentState == COOKING) {
        currentState = IDLE;
    }
    else if (currentState == IDLE || currentState == COOKING_COMPLETE || currentState == BROKEN) {
        resetTimer();
        if (currentState == BROKEN) {
        }
        else {
            currentState = IDLE;
        }
    }
    updateLightState();
}

void Microwave::tick() {
    if (currentState == COOKING) {
        if (remainingSeconds > 0) {
            remainingSeconds--;
            if (remainingSeconds == 0) {
                currentState = COOKING_COMPLETE;
                updateLightState();
            }
        }
    }
}


void Microwave::breakMicrowave() {
    if (currentState == BROKEN) {
        return; 
    }
    currentState = BROKEN;
    doorState = CLOSED;
    remainingSeconds = 0; 
    inputBuffer.clear(); 
    lightOn = false;
}

void Microwave::repairMicrowave() {
    if (currentState != BROKEN) {
        return; 
    }
    currentState = IDLE;
    doorState = CLOSED;
    remainingSeconds = 0;
    inputBuffer.clear(); 
    lightOn = false; 
}