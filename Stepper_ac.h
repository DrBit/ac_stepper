/********************************************
**  Name: Stepper_ac.h Library
**  Created: 27/06/2011
*********************************************
**  Quim Llums
**
**  http://www.skmcreatiu.com
**  http://blog.drbit.nl
**  http://drbit.nl
**
*/

#ifndef Stepper_ac_h
#define Stepper_ac_h

#include "WProgram.h"



class Stepper_ac
{
  public:
	// FUCNTIONS
	Stepper_ac(int step_pin, int direction_pin, int sensor_pin, int ms1, int ms2, int motor_original_steps, int step_mode);
	void set_init_position();
	void do_step();
	void count_step(bool _temp_direction);
	//int get_position();
	int get_steps();
	int get_steps_cycles();
	bool get_direction();
	void set_direction(bool direction);
	void move_step();
	void change_step_mode(int new_step_mode); 
	int get_step_accuracy();
	int get_step_mode();
	int get_steps_per_cycle();
	void initiate();
	bool sensor_check();
	void wait_till_reach_position(long m_steps, long m_cycles);
	//Properties
	int step_accuracy;
	int step_mode;
	long stepPosition;  // check if it can be int
	int stepCycle;
	unsigned char Easy_steptab[];


  private:
	// PINS
	int _step_pin;
	int _direction_pin;
	int _sensor_pin;
	int _ms1;
	int _ms2;
	// Properties
	unsigned long _motor_total_possible_steps;		//! Numer of stpes of the motor (if using a different type of motor)
	long _stepPosition;				//! Position of stepper motor (relative to starting position as zero) 
	int _stepCycle;
	bool _direction;	
	int _step_accuracy;				//! Step accuracy is the number of steps we are counting each time me move. This property depends on the motor mode we selected
	int _step_mode;
	int _motor_original_steps;
};

#endif