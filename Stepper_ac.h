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
	Stepper_ac(const int step_pin, const int direction_pin, const int sensor_pin, const int ms1, const int ms2, const int motor_original_steps, int step_mode);
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
	// Movement functions
	void wait_till_reach_position(long m_steps, long m_cycles);
	void select_case (int vel_case, int steps_case) ;
	void move_n_steps_slow (unsigned int mov_steps);
	int move_n_steps_fast (unsigned int mov_steps);
	void ramp_down (int accel_factor);
	void ramp_up (int accel_factor);
	void move_motor(unsigned int cycles,unsigned int steps, int accel_factor, boolean direction);
	void got_to_position (unsigned int pos_cycles, unsigned int pos_steps);
	int get_version();
	//Properties
	int step_accuracy;
	int step_mode;
	long stepPosition;  // check if it can be int
	int stepCycle;
	// unsigned char Easy_steptab[];  // where does this comes from?? maybe a test?


  private:
	// PINS
	int _step_pin;
	int _direction_pin;
	int _sensor_pin;
	int _ms1;
	int _ms2;
	// Properties
	long _motor_total_possible_steps;		//! Numer of stpes of the motor (if using a different type of motor)
	long _stepPosition;				//! Position of stepper motor (relative to starting position as zero) 
	int _stepCycle;
	bool _direction;	
	int _step_accuracy;				//! Step accuracy is the number of steps we are counting each time me move. This property depends on the motor mode we selected
	int _step_mode;
	int _motor_original_steps;
};

#endif