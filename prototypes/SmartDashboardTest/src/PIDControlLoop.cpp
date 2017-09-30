#include "PIDControlLoop.h"

PIDConfig::PIDConfig() {
	pFac = 0.0;
	iFac = 0.0;
	dFac = 0.0;
	maxAbsOutput = 0.0;
	maxAbsError = 0.0;
	maxAbsDiffError = 0.0;
	desiredAccuracy = 0.0;
	maxAbsITerm = 0.1;
	outputValuesForTuning = false;
}

PIDControlLoop::PIDControlLoop(PIDConfig* myConfig) {
	pidConfig = myConfig;
}

void PIDControlLoop::Init(double myInitialSensorValue,
						  double myDesiredSensorValue) {
	initialSensorValue = myInitialSensorValue;
	desiredSensorValue = myDesiredSensorValue;
	oldError = 0.0;
	sumError = 0.0;
}

// Returns the actuator value (motor speed, etc.)
double PIDControlLoop::Update(double currentSensorValue) {
	double error = desiredSensorValue - currentSensorValue;
	error = Saturate(error, pidConfig->maxAbsError);
	double diffError = 0.0;
	if (oldError != 0.0) {
		diffError = error - oldError;
		diffError = Saturate(diffError, pidConfig->maxAbsDiffError);
	}
	sumError += error;
	if (pidConfig->iFac > 0.0) {
		sumError = Saturate(sumError, (pidConfig->maxAbsITerm / pidConfig->iFac));
	}
	double pTerm = pidConfig->pFac * error;
	double iTerm = pidConfig->iFac * sumError;
	double dTerm = pidConfig->dFac * diffError;
	double output = pTerm + iTerm + dTerm; //  PID
	output = Saturate(output, pidConfig->maxAbsOutput);
	oldError = error;
	if (pidConfig->outputValuesForTuning) {
		SmartDashboard::PutNumber("Error: ", error);
		SmartDashboard::PutNumber("DiffError: ", diffError);
		SmartDashboard::PutNumber("SumError: ", sumError);
		SmartDashboard::PutNumber("PTerm: ", pTerm);
		SmartDashboard::PutNumber("ITerm: ", iTerm);
		SmartDashboard::PutNumber("DTerm: ", dTerm);
		SmartDashboard::PutNumber("Output: ", output);
	}
	return output;
}

double PIDControlLoop::Saturate(double value, double maxAbsValue) {
	//limits the value to maxAbsValue
	if (maxAbsValue > 0.0) {
		if (value > 0.0) {
			return std::min(value, maxAbsValue);
		} else {
			return std::max(value, -maxAbsValue);
		}
	} else {
		return value;
	}
}

bool PIDControlLoop::ControlLoopDone(double currentSensorValue) {
	//when the value returned is close enough to what we want
	if (initialSensorValue <= desiredSensorValue &&
		currentSensorValue >= desiredSensorValue) {
		return true;
	} else if (initialSensorValue >= desiredSensorValue &&
			   currentSensorValue <= desiredSensorValue) {
		return true;
	} else if (pidConfig->desiredAccuracy > 0.0 &&
			   fabs(desiredSensorValue - currentSensorValue)
				 <= pidConfig->desiredAccuracy) {
		return true;
	} else {
		return false;
	}
}
