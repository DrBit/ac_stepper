/********************************************
**  Name: Stepper_ac.cpp Library
**  Created: 27/06/2011
*********************************************
**  Quim Llums
**
**  http://www.skmcreatiu.com
**  http://blog.drbit.nl
**  http://drbit.nl
**
*/

#include "WProgram.h"
#include "Stepper_ac.h"			

#define lib_version 12	   		   

// SETUP	
// int step_pin -- Pin where the step control is connected
// int direction_pin -- Pin where the direction control is connected
// int sensor_pin -- Pin where the sensor output is connected (if any), in case of no sensor type 0
// int ms1 -- Pin where the MS1 controller is connected
// int ms2 -- Pin where the MS2 controller is connected
// unsigned long motor_steps -- Total of steps of the motor
// int step_mode		 -- 4 modes available: 1=FULL, 2=HALF, 4=QUARTER, 8=EIGHTH
// ********************* ADD const in pin values to waste less RAM *******************
Stepper_ac::Stepper_ac(const int step_pin, const int direction_pin, const int sensor_pin, const int ms1, const int ms2, const int motor_original_steps, int step_mode)
{
	// PINS SETUP 
	pinMode(step_pin, OUTPUT);
	pinMode(direction_pin, OUTPUT);
	pinMode(step_pin, OUTPUT);
	if (sensor_pin != 0) pinMode(sensor_pin, INPUT);	// if sensor pin is 0 means there is no sensor pin for this motor. so we don't init any port
	if (ms1 != 0) pinMode(ms1, OUTPUT);					// if ms1 pin is 0 means there is no ms1 pin for this motor. so we don't init any port
	if (ms2 != 0) pinMode(ms2, OUTPUT);					// if ms1 pin is 0 means there is no ms1 pin for this motor. so we don't init any port
	_step_pin = step_pin;
	_direction_pin = direction_pin;
	_sensor_pin = sensor_pin;
	_ms1 = ms1;
	_ms2 = ms2;
	// Properties
	_step_mode = step_mode;
	_motor_original_steps = motor_original_steps;
	_motor_total_possible_steps = (_motor_original_steps * 8);	// Total number of steps we can do in one complete turn (8 is because the easy driver can divide each step into 8)
	stepPosition = 0;
	_stepPosition = 0;
	stepCycle = 0;
	_stepCycle = 0;
	// INITS (setup)
	change_step_mode(step_mode);
}

// 4 modes available: 1=FULL, 2=HALF, 4=QUARTER, 8=EIGHTH
void Stepper_ac::change_step_mode(int new_step_mode) 
{
	// Configuration of the pins of the easy drivers, settings in the datasheet
	if (new_step_mode == 1) {
		digitalWrite (_ms1, LOW);
		digitalWrite (_ms2, LOW);
	}else if (new_step_mode == 2) {
		digitalWrite (_ms1, HIGH);
		digitalWrite (_ms2, LOW);
	}else if (new_step_mode == 4) {
		digitalWrite (_ms1, LOW);
		digitalWrite (_ms2, HIGH);
	}else if (new_step_mode == 8) {
		digitalWrite (_ms1, HIGH);
		digitalWrite (_ms2, HIGH);
	}
	_step_mode = new_step_mode;		// Update variable containing the step mode of the motor
	_step_accuracy = _motor_total_possible_steps / (_motor_original_steps * new_step_mode);		// Accuracy we can achieve with the actual motor step mode
	// we should have an unifing system for counting, depending on the mode we conut 2 eachtime, 4 or 8. This way it doesnt
	// matter the mode in which we are setiing up the motor and we will have only one system for counting distance
	
	// We should move the motor in a place where we can ne sure there will be no errors when changing mode
	 
	// in mode 8 we count by one   * 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 *
	// in mode 4 we count by 2     * 1     , 3     , 5     , 7     *
	// in mode 2 we count by 4     * 1             , 5             *
	// in Full mode we count by 8  * 1                             *

	// Example:
	// if we are in step 4 (mode 8) and we want to go to (mode 2) we should move the motor to 
	// step position 3 or 5 in order to avoid adding errors to the counting system
	
	
	// _motor_total_possible_steps = (motor_original_steps * step_mode);
}

int Stepper_ac::get_step_mode()
{
	step_mode = _step_mode;
	return step_mode;
}

int Stepper_ac::get_step_accuracy()
{
	step_accuracy = _step_accuracy;
	return step_accuracy;
}

int Stepper_ac::get_steps_per_cycle()
{
	return (_step_accuracy * _motor_original_steps);
}

void Stepper_ac::set_init_position()
{
	// Set position of the motor to 0;
	_stepPosition = 0;
	_stepCycle = 0;
}

// This function returns the number of cycles the motor did positive or negative also counts for left right
int Stepper_ac::get_steps()
{
	stepPosition = _stepPosition;
	return stepPosition;
}

// This function returns the number of cycles the motor did. Example
// motor of 200 steps did 230 steps clockwise so = 1 cycle + 30 steps
int Stepper_ac::get_steps_cycles()
{
	stepCycle = _stepCycle;
	return stepCycle;
}

// Sets direction of the motor movement
void Stepper_ac::set_direction(bool direction)
{
	_direction=direction;
	if(_direction){
		digitalWrite (_direction_pin, HIGH);
	}else{
		digitalWrite (_direction_pin, LOW);
	}	
}

// Reads the actual direction of the motor
bool Stepper_ac::get_direction()
{
	return digitalRead(_direction_pin);
}

// Order to make the motor move one step (with coun ting steps enabled)
void Stepper_ac::do_step()
{
  // Just as information, variable _stepPosition doesn't do anything in the code
  // It is just a mere information of position (in steps) to know where the motor is
  // And because its a private variable, each instance of motor has its own position
  // static unsigned char counter = 0;
  // Update position
  count_step(_direction);

  // Change the counter in order to add steps at it
  // like using a full 4 pins driver it would need 
  // at least 4 steps, or 8 for doubled steps
  // Since we are using easy driver we need to count 0 to 1 only (true false, just a normal pulse)
  // Stay within the steptab
  // counter &= B00000001; //Force count 0 to 1 because we have only 2 steps
  move_step();
}

// FIX BUG
void Stepper_ac::count_step(bool _temp_direction)
{
	if(!_temp_direction){
		_stepPosition = _stepPosition + _step_accuracy;
	}else{
		_stepPosition = _stepPosition - _step_accuracy;
	}
	
	if (_stepCycle == 0) {		// If we are in the first turn we see if we gowing forward or backwards
		if (_stepPosition > _motor_total_possible_steps){		// we chek the upper limit (if there was no limit (0) nothing will happen)
			_stepCycle++; 
			_stepPosition = _stepPosition - _motor_total_possible_steps;
		}else if (_stepPosition < (_motor_total_possible_steps - (2*_motor_total_possible_steps))) {	// we chek the lower limit, -1 
			_stepCycle--;
			_stepPosition = _motor_total_possible_steps + _stepPosition;
		}
		
	}else if (_stepCycle > 0) {  // Means we go forwards so count in the positive range
		if (_stepPosition > _motor_total_possible_steps){			// we chek the upper limit (if there was no limit (0) nothing will happen)
			_stepCycle++; 
			_stepPosition = _stepPosition - _motor_total_possible_steps;
		}else if (_stepPosition < 1) {  // we chek the lower limit, -1 
			_stepCycle--;
			_stepPosition = _motor_total_possible_steps + _stepPosition;
		}
	}/*else if (_stepCycle < 0) {  // Means we go backwards so count in the negative range
		if (_stepPosition <= (-_motor_total_possible_steps)){		// In negative range this is the lower limit
			_stepCycle--; 
			_stepPosition = -_step_accuracy;
		}else if (_stepPosition <= 0) {	// In negative range this is the upper limit
			_stepCycle++;
			_stepPosition = -_motor_total_possible_steps;
		}
	}*/
}

int theDelay = 1;

// This order will move the motor one step in the designed direction but without updating the counter
void Stepper_ac::move_step()
{
	// Using arduino code might take too long to put the pin on and off  (OLD approach)
	// This is where we send to the easy driver a pulse with the right Width of Minimum 1.0 �s
	digitalWrite (_step_pin, HIGH);
	delayMicroseconds(theDelay);   // This is a trick to generate exactly 1uS delay in Arduino
	digitalWrite (_step_pin, LOW);
	delayMicroseconds(theDelay);   // This is a trick to generate exactly 1uS delay in Arduino
	// Output the fast way
	// SM_PORT |= ((temp<<4)&0xF0);
	// SM_PORT &= ((temp<<4)|0x0F);
}

// function to find the initial position of a motor thru the sensor
void Stepper_ac::initiate()
{
	set_direction (false);   // Goes back till we find the sensor
	while (sensor_check())
	{
		do_step();
	}
	set_init_position();
}

// Checks whether the sensor atached to the motor detects something
bool Stepper_ac::sensor_check()
{
	// Because its a barrier sensor the pin its inverted, so we have 0 when we have a sensor
	// hence the "!" in front of the digitalRead, so we invert the signal ** RECHEK!!!
	if (_sensor_pin != 0) {    // If _sensor_pin == 0 means we dont have any sensor for this motor. we will return 0 Alwais
		return (!digitalRead(_sensor_pin));
	} else {
		return false;
	}
}

void Stepper_ac::wait_till_reach_position(long m_steps,long m_cycles) 
{
	while ((get_steps() != m_steps) && (get_steps_cycles() != m_cycles)) {
		// wait doing nothing until we areach the desired position 
	} 
}


// ************************************************************
// ** NEW move function
// ************************************************************

void Stepper_ac::got_to_position (unsigned int pos_cycles, unsigned int pos_steps) {
	boolean move = true;
	//error chgecking if steps are greater that 1600 (max).
	int cycles_to_move = pos_cycles - get_steps_cycles();
	int steps_to_move =  pos_steps - get_steps();

	// determine the direction X axis
	boolean direction;
	if (cycles_to_move < 0) {
		// Direction false
		direction = false;
	}else if (cycles_to_move > 0) {
		//  direction true
		direction = true;
	}else{  			// Means its = to 0
		// we need to check the steps in order to determine the direction
		if (steps_to_move < 0) {
			direction = false;
		}else if (steps_to_move > 0){
		   direction = true;
		}else { 		// in case steps_to_move is == 0 we dont care as we will not be moving anywhere
			move = false;
		}
	}

	if (move) {
		if (direction) {
			if (steps_to_move > 0) {
				// Keeps as it is
			} else {
				// because its a negative number we add it to the total and we will get a positive number
				steps_to_move = 1600 + steps_to_move;
				cycles_to_move --;
			}
		} else {
			if (steps_to_move < 0) {	
				// Numbers stay as they are. 
			} else {
				steps_to_move = steps_to_move -1600;
				cycles_to_move ++;
			}
			//We change them from negative to positive as the direction will control the motion.
			// Prepare numbers to be send
			cycles_to_move = -cycles_to_move;
			steps_to_move = -steps_to_move;
		}
		
		// Acceleration 40 seems to work pretty well
		const int acceleration = 40; 
		move_motor(cycles_to_move, steps_to_move, acceleration, direction);
	}
}



/***** Main fucntion, move motor a certain number of steps and cicles at a designed direction  *****/
void Stepper_ac::move_motor(unsigned int cycles,unsigned int steps, int accel_factor, boolean direction)
{
	// we set direction
	set_direction (!direction);
	// accel factor standard = 40
	// Some previos calculations
	// 920*2 = 1840
	unsigned int total_steps_cycles_for_aceleration = (accel_factor + (accel_factor*2) + (accel_factor*4) + (accel_factor*8*2))*2;
	unsigned int total_steps_for_aceleration = total_steps_cycles_for_aceleration % 1600;
	unsigned int total_cycles_for_aceleration = total_steps_cycles_for_aceleration / 1600;
	
	if ((total_cycles_for_aceleration > cycles) || ((total_steps_for_aceleration > steps) && (total_cycles_for_aceleration == cycles))) {		// Meaning that we need to move less than we need to just acelerate
		// Calculation total amount of steps
		unsigned int total_steps_to_move = (cycles*1600)+steps;
		// Moving....
		move_n_steps_slow (total_steps_to_move);
	}else{
		// firs we prepare all calculations so we dont need to calculate in the middle
		// removing fixed steps to accelerate from the total steps to move
		if (steps > total_steps_for_aceleration) {				// If we have less steps that we got to rest then whe need to substract one cycle and use those steps to acomplish the rest
			steps = steps - total_steps_for_aceleration;
		}else{
			total_steps_for_aceleration = total_steps_for_aceleration - steps;
			steps = 1600;
			cycles --;
			steps = steps - total_steps_for_aceleration;
		}
		cycles = cycles - total_cycles_for_aceleration;

		// we start the ramp up and achieve certain velocity
		ramp_up (accel_factor);												// 920 steps FIXED

		// at a certain velocity we move the steps
		int remaining_steps = move_n_steps_fast (steps) ;
		// inside "remaining_steps" there is the steps we couldnt do beacuse of a lack of resolution
		// we will move them slowlty at the end because now we are going at max speed.

		// then we move all cycles also at max speed
		for (int a = cycles; a > 0; a--) {
			move_n_steps_fast (1600) ;			
		}

		// once we moved all steps we start the ramp down
		ramp_down(accel_factor);											// 920 steps FIXED

		//Now that we are almost stop and the resolution of the motor is max, we move the steps we missed at max speed
		move_n_steps_slow (remaining_steps);
	}
}

/***** Ramping up, achieve velocity  *****/
void Stepper_ac::ramp_up (int accel_factor) {
	for (int i = 1; i< 6; i++) {
		select_case (i, accel_factor);
	}
}

/***** Ramping down, decreasing velocity  *****/
void Stepper_ac::ramp_down (int accel_factor) {
	for (int i = 5; i> 0; i--) {
		select_case (i, accel_factor);
	} 
}

/***** Move N steps at the max velocity *****/
int Stepper_ac::move_n_steps_fast (unsigned int mov_steps) {
	change_step_mode(1);		// Change mode 1
	for (int a = 0; a < mov_steps/get_step_accuracy(); a++) {
		do_step();
		delayMicroseconds (490);
	}
	return (mov_steps % get_step_accuracy());
}

/***** Move N steps at the min velocity *****/
void Stepper_ac::move_n_steps_slow (unsigned int mov_steps){
	change_step_mode(8);		// Change mode 8 steps 
	// Executing steps
	for (int a = 0; a < mov_steps; a++) {
		do_step();
		delayMicroseconds (260);
	}
}

// ************************************************************
// ** Extra functions for ramping and testing
// ************************************************************

/***** Ramping cases for changin velocities *****/
void Stepper_ac::select_case (int vel_case, int steps_case) {
	//const int steps_case = 40;  // ol variable for defining number of steps to do in each velocity step

	// for acelerating or deacelerating (for both = result*2)
	// formula = steps_case + (steps_case*2) + (steps_case*4) + (steps_case*8*2) 
	
	if (vel_case == 1) {
		change_step_mode(8);		// Change mode 8
		for (int a = 0; a < steps_case; a++) {
			do_step();
			delayMicroseconds (330);
		}
	}
	if (vel_case == 2) {
		change_step_mode(4);		// Change mode 4
		for (int a = 0; a < steps_case; a++) {
			do_step();
			delayMicroseconds (350);
		}
	}
	if (vel_case == 3) {
		change_step_mode(2);		// Change mode 2
		for (int a = 0; a < steps_case; a++) {
			do_step();
			delayMicroseconds (450);
		}
	}
	if (vel_case == 4) {
		change_step_mode(1);		// Change mode 1
		for (int a = 0; a < steps_case; a++) {
			do_step();
			delayMicroseconds (560);
		}
	}
	if (vel_case == 5) {
		change_step_mode(1);		// Change mode 1
		for (int a = 0; a < steps_case; a++) {
			do_step();
			delayMicroseconds (500);
		}
	}
}


int Stepper_ac::get_version() {
	return lib_version;
}