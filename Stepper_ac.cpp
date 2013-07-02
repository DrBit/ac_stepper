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

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "Stepper_ac.h"

//  #define DEBUG_acceleration		// Not recomended in real situation, motor wont respond correctly

#define lib_version 15
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
	_default_sensor_state = true;
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
	set_default_direcction (true);
	// Acceleration parameters DEFAULT
	// set_accel_profile(unsigned int init_timing, unsigned int ramp_inclination, unsigned int n_slopes_per_mode, unsigned int n_steps_per_slope)
	set_accel_profile(700, 14, 10, 50);
	// inlfuence in accelration
	// _init_timing = 700;			// Usually fine tunea nice inclination ramp (140 micro seconds is the MAX for the motors)
	// _ramp_inclination = 14;		// Less = more inclination = more aceleration  //  More = less inclination = less aceleration
	// influence in torque
	// _n_slopes_per_mode = 10;		// It also increases the inclination as more steps more divisions 
	// ABSOLUT MAX 20!!!!!!!!!!!!!
	// _n_steps_per_slope = 50;
	_remaining_steps = 0;
	_delay_slow_mode = 200;			// Default speed in slow mode

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
	if (_default_direction) {
		_direction= direction;
	}else{
		_direction= !direction;
	}
	
	if(direction){
		digitalWrite (_direction_pin, HIGH);
	}else{
		digitalWrite (_direction_pin, LOW);
	}
}

// Reads the actual direction of the motor
bool Stepper_ac::get_direction()
{
	bool direction = _direction;
	return direction;
}

void Stepper_ac::set_default_direcction (bool direction)
{
	_default_direction = direction;
}

void Stepper_ac::set_default_sensor_state (bool default_sensor_state)
{
	_default_sensor_state = default_sensor_state;
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


void Stepper_ac::count_step(bool _temp_direction)
{
	
	if(_temp_direction){
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
	}*/ // IMPLEMENT
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
	if (_sensor_pin != 0) {    // If _sensor_pin == 0 means we dont have any sensor for this motor. we will return 0 Always
		if (_default_sensor_state) {
			return (digitalRead(_sensor_pin));
		}else{
			return (!digitalRead(_sensor_pin));
		}
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
		direction = false;		// Direction false
	}else if (cycles_to_move > 0) {
		direction = true;		//  direction true
	}else{  			// Means its = to 0
		// we need to check the steps in order to determine the direction if there are no cycles to move
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
		const int acceleration = 50; 
		// OLD procedure
		// move_motor(cycles_to_move, steps_to_move, acceleration, direction);
		// NEW procedure
		
		
		if (_default_direction) {
			direction = direction;
		}else{
			direction = !direction;
		}

		move_motor_accel(cycles_to_move,steps_to_move, direction);
	}
}



/***** Acceleration profile, gives to acceleration the right parameters  *****/

void Stepper_ac::set_accel_profile(unsigned int init_timing, unsigned int ramp_inclination, unsigned int n_slopes_per_mode, unsigned int n_steps_per_slope)
{
	// Acceleration parameters
	_init_timing = init_timing;
	_ramp_inclination = ramp_inclination;
	_n_slopes_per_mode = n_slopes_per_mode;		// MAX 20
	_n_steps_per_slope = n_steps_per_slope;
	calculate_profile ();
}


/*
void Stepper_ac::set_acce_and_speed_profile(unsigned int init_speed, unsigned int max_speed, unsigned int acceleration)
{
	// Acceleration parameters
	_init_speed = init_speed;
	_max_speed = max_speed;
	_acceleration = acceleration;		// MAX 20
	calculate_acceleration ();
}

/*
void Stepper_ac::calculate_profile () {
	#if defined DEBUG_acceleration
	#endif

	// max absolute delay between steps is 160, less than that will have almost no torke;	
	//      ___________Max_speed_____
	//    _/
	//  _/ _acceleration steps (less = faster)
	// /____ _ _ _ _ _ INIT SPEED_ _ _ _ _ _ 
	//
	//////////////////////////////////////////

	unsigned int _differential_speed = _init_speed - _max_speed;
	unsigned int _delay_between_steps = _differential_speed / _acceleration ;
	unsigned int  _delay1useceveryNsteps = _acceleration /(_differential_speed % _acceleration);
	float  _delay1useceveryNsteps = (_acceleration /(_differential_speed % _acceleration)) - _delay1useceveryNsteps;

	// The result is the amount of micro seconds we have to decrease in each step
	// if ms is > 0
	// example 0.2 microseconds per step
	// in this case we know that each time we do a step we increase

	// if ms is < 
	int stepping_rest = 0
	for (int c = _acceleration; c < =; z --) {
		do_step();
		delayMicroseconds (delay_delay_between_steps);
		stepping_rest ++;
		if ((_delay1useceveryNsteps == stepping_rest){
			delayMicroseconds (1);
			stepping_rest = 0;
			if (_delay1useceveryNsteps >= 1) {
				_delay1useceveryNsteps += _delay1useceveryNsteps;
			}
		}
	}

}
*/




/***** Calculate profile, calculates and stores all delays in a matrix  *****/
void Stepper_ac::calculate_profile () {
	// (_n_slopes_per_mode)+1  Always plus 1 for the virtual step
	// Number 3 stands for number of gears (or motor modes) we have 4 (0,1,2,3)
	// _acceleration_curve[4][(_n_slopes_per_mode+1)];			// Bi-dimensional array
	#if defined DEBUG_acceleration
	Serial.println((_n_slopes_per_mode+1));
	#endif
	
	// max absolute delay between steps is 160, less than that will have almost no torke;
	
	//      ___________Max_speed_____
	//    _/
	//  _/ _ramp_inclination
	// /____ _ _ _ _ _ _ _ _ _ _ _ 
	//
	//////////////////////////////////////////
	// each slope is at the same time smoothed in small linear increments
	unsigned int timing	= _init_timing;
	//int motor_mode = init_motor_mode;
	for (int m_mode=3; m_mode>=0; m_mode--) {
		// precalculate the first timing
		timing = timing + (timing / _ramp_inclination);		// This is a virtual timing requiered once 
															// in each gear(motor_mode) to ramp all slopes in velocity
		for (int counter_ramp=0 ; counter_ramp <= _n_slopes_per_mode; counter_ramp++){
			_acceleration_curve[m_mode][counter_ramp] = timing;

			#if defined DEBUG_acceleration
			Serial.print(timing);
			Serial.print(" at mode ");
			Serial.print(m_mode);
			Serial.print(" index ");
			Serial.print(counter_ramp);
			Serial.print(" DATA recorded = ");
			Serial.println(_acceleration_curve[m_mode][counter_ramp]);
			#endif

			timing = timing - (timing / _ramp_inclination);			
		}
		timing = timing * 2;
	}

	#if defined DEBUG_acceleration
	for (int m_mode=3; m_mode>=0; m_mode--) {
		for (int a=0; a <= _n_slopes_per_mode; a++){
			Serial.print("m_mode ");
			Serial.print(m_mode);
			Serial.print(" index ");
			Serial.print(a);
			Serial.print(" DATA: ");
			Serial.println (_acceleration_curve[m_mode][a]);
		}
	}
	#endif
}

/***** Main fucntion, move motor with defined acceleration a certain number of steps and cicles at a designed direction  *****/
unsigned int Stepper_ac::ramp_up_accelerated()
{
	// Ramp UP
	#if defined DEBUG_acceleration
	unsigned int steps_done=0;
	#endif
	int indexed_m_mode = 3;		// temp var for get indexed data
	unsigned int temp_delay;
	for (int m_mode=8; m_mode>=1; m_mode = m_mode/2) {
		change_step_mode(m_mode);
		// Virtualitzation of previous delay each time we change step mode
		int previous_delay = _acceleration_curve[indexed_m_mode][0];		// Each first parameter is just virtualitzation
		// calculate exact delay per step
		for (int counter_ramp=1 ; counter_ramp <= _n_slopes_per_mode; counter_ramp++){
			unsigned int accel_del = _acceleration_curve[indexed_m_mode][counter_ramp];	// Fetch delay to achieve in this slope
			#if defined DEBUG_acceleration
			Serial.print(accel_del);
			Serial.print(" at mode ");
			Serial.print(indexed_m_mode);
			Serial.print(" index ");
			Serial.println(counter_ramp);
			#endif
			unsigned int decrement = (previous_delay-accel_del)/_n_steps_per_slope;	// We are accelerating so previous delay  
																		// is always grater than the actual
			temp_delay = previous_delay;					// We need a container for the delay increments
			previous_delay = accel_del;									// Recording previous delay for next time
			for (int a = 0; a < _n_steps_per_slope; a++){
				do_step();
				delayMicroseconds (temp_delay);
				temp_delay = temp_delay-decrement;						// decrement delay each step (needs floating adjustment?)
				#if defined DEBUG_acceleration
				steps_done++;
				#endif
			}
			//_remaining_steps =+ (_n_steps_per_slope % get_step_accuracy());
		}
		indexed_m_mode--;		// Prepare for next motor mode index
	}
	#if defined DEBUG_acceleration
	Serial.print("steps during ramp_up = ");
	Serial.println(steps_done);
	#endif
	return temp_delay;			// Return max velocity achieved
}

/***** Move N steps at the max velocity ACCELERATED*****/
unsigned int Stepper_ac::move_n_steps_fast_accelerated (unsigned int mov_steps, unsigned int inner_delay) {
	// change_step_mode(1);		// Change mode 1
	#if defined DEBUG_acceleration
	unsigned long steps=0;
	#endif
	for (int a = 0; a < (mov_steps/get_step_accuracy()); a++) {
		do_step();
		delayMicroseconds (inner_delay);
		#if defined DEBUG_acceleration
		steps++;
		#endif
	}
	#if defined DEBUG_acceleration
	Serial.print("steps done during max_v = ");
	Serial.println(steps);
	#endif
	return mov_steps%get_step_accuracy();
}
	

void Stepper_ac::ramp_down_accelerated(){
	// Ramp DOWN
	int indexed_m_mode = 0;
	#if defined DEBUG_acceleration
	unsigned int steps_done=0;
	#endif
	for (int m_mode = 1; m_mode<= 8; m_mode = m_mode*2) {
		change_step_mode(m_mode);
		// Virtualitzation of previous delay each time we change step mode
		int previous_delay = _acceleration_curve[indexed_m_mode][_n_slopes_per_mode];	// Each first parameter is just virtualitzation
		// calculate exact delay per step
		for (int counter_ramp=_n_slopes_per_mode-1; counter_ramp>=0; counter_ramp--){
			unsigned int accel_del = _acceleration_curve[indexed_m_mode][counter_ramp];	// Fetch delay to achieve in this slope
			#if defined DEBUG_acceleration
			Serial.print(accel_del);
			Serial.print(" at mode ");
			Serial.print(indexed_m_mode);
			Serial.print(" index ");
			Serial.println(counter_ramp);
			#endif
			unsigned int increment = (accel_del-previous_delay)/_n_steps_per_slope;	// We are deaccelerating so previous delay  
																						// is always smaller than the actual
			unsigned int temp_delay = previous_delay;					// We need a container for the delay increments
			previous_delay = accel_del;									// Recording previous delay for next time
			for (int a = 0; a < _n_steps_per_slope; a++){
				do_step();
				delayMicroseconds (temp_delay);
				temp_delay = temp_delay+increment;						// increment delay each step (needs floating adjustment?)
				#if defined DEBUG_acceleration
				steps_done++;
				#endif
			}
			//_remaining_steps =+ (_n_steps_per_slope % get_step_accuracy());
		}
	}
	indexed_m_mode++;		// Prepare for next motor mode index
	#if defined DEBUG_acceleration
	Serial.print("steps during ramp_down = ");
	Serial.println(steps_done);
	#endif
}
	
/***** NEW!! Main fucntion, ACCELERATION  *****/
void Stepper_ac::move_motor_accel(unsigned int cycles,unsigned int steps, boolean direction)
{
	// we set direction
	set_direction (direction);
	// Some previous calculations
	// 4 modes
	// _n_slopes_per_mode = n_slopes_per_mode;
	// _n_steps_per_slope = n_steps_per_slope;
	//unsigned int total_steps_cycles_for_aceleration = (4*_n_slopes_per_mode*_n_steps_per_slope)*2;
	//unsigned int total_steps_cycles_for_aceleration = ((_n_steps_per_slope)+(_n_steps_per_slope*2)+(_n_steps_per_slope*4)+(_n_steps_per_slope*8))*_n_slopes_per_mode*2;
	unsigned int total_steps_cycles_for_aceleration = (_n_steps_per_slope + (_n_steps_per_slope*2) + (_n_steps_per_slope*4) + (_n_steps_per_slope*8))*_n_slopes_per_mode*2;
	#if defined DEBUG_acceleration
	Serial.print("Calculation of steps taken for acceleration = ");
	Serial.println(total_steps_cycles_for_aceleration);
	#endif
	unsigned int total_steps_for_aceleration = total_steps_cycles_for_aceleration % 1600;
	unsigned int total_cycles_for_aceleration = total_steps_cycles_for_aceleration / 1600;
	
	if (true) {
	//if ((total_cycles_for_aceleration > cycles) || ((total_steps_for_aceleration > steps) && (total_cycles_for_aceleration == cycles))) {		// Meaning that we need to move less than we need to just acelerate
		// Calculation total amount of steps
		unsigned int total_steps_to_move = (cycles*1600)+steps;
		// We should check if the result is bigger than unsigned int or we would have problems
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
		
		//////////////////////////////////
		// Movement algorithm
		//////////////////////////////////
		// we start the ramp up and achieve certain velocity, we record this velocity in delay form
		int unsigned max_v_delay = ramp_up_accelerated();
		
		#if defined DEBUG_acceleration
		Serial.print("steps calculated during max_v = ");
		Serial.println(steps);
		#endif
		// at a max velocity we move the desired steps and cycles
		_remaining_steps = move_n_steps_fast_accelerated (steps, max_v_delay);
		// inside "remaining_steps" there is the steps we couldnt do beacuse of a lack of resolution
		// we will move them slowly at the end because now we are going at max speed.
		
		#if defined DEBUG_acceleration		
		Serial.print("cycles during max_v = ");
		Serial.println(cycles);
		#endif
		// Keep movement for the desired cycles also at max speed
		for (int a = cycles; a > 0; a--) {
			move_n_steps_fast_accelerated (1600, max_v_delay);			
		}

		// once we moved all steps we start the ramp down
		ramp_down_accelerated();
		
		#if defined DEBUG_acceleration		
		Serial.print("remaining steps at max_v = ");
		Serial.println(_remaining_steps);
		#endif
		//Now that we are almost stop and the resolution of the motor is max, we move the steps we missed at max speed
		move_n_steps_slow (_remaining_steps);

		//
		//END
	}
}




/***** Move N steps at the max velocity *****/
int Stepper_ac::move_n_steps_fast (unsigned int mov_steps) {
	// change_step_mode(1);		// Change mode 1
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

	unsigned int accel_steps_period = 45;
	int accel_slopes = 21;
	int increments_of_microseconds = 8;
	unsigned int temporal_delay = _delay_slow_mode;

	if (mov_steps > (accel_slopes*2*accel_steps_period)) {
		/// aceleration
		for (int slopes_done = 1; slopes_done <=accel_slopes; slopes_done++) {
			for (unsigned int steps_done = 1; steps_done <= accel_steps_period; steps_done++) {
				do_step();
				delayMicroseconds (temporal_delay);
			}
			temporal_delay = temporal_delay - increments_of_microseconds;		// iNCREMENTS ON SPEED PER SLOPE
		}

		/// full speed
		for (unsigned int steps_at_max= 1; steps_at_max <= (mov_steps-(accel_slopes*2*accel_steps_period)); steps_at_max++) {
				do_step();
				delayMicroseconds (temporal_delay);
		}

		// deceleration
		for (int slopes_done = 1; slopes_done <=accel_slopes; slopes_done++) {

			temporal_delay = temporal_delay + increments_of_microseconds;		// iNCREMENTS ON SPEED PER SLOPE
			for (unsigned int steps_done = 1; steps_done <= accel_steps_period; steps_done++) {
				do_step();
				delayMicroseconds (temporal_delay);
			}
		}
	}else{
		for (unsigned int a = 0; a < mov_steps; a++) {
			do_step();
			delayMicroseconds (_delay_slow_mode);
		}
	}	

}

void Stepper_ac::set_speed_in_slow_mode (unsigned int delay_slow_mode) {
	_delay_slow_mode = delay_slow_mode;
}



int Stepper_ac::get_version() {
	return lib_version;
}