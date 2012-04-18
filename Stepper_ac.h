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

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif



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
	void set_default_direcction (bool direction);
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
	// Newly added motion functions
	void set_accel_profile(unsigned int init_timing, unsigned int ramp_inclination, unsigned int n_slopes_per_mode, unsigned int n_steps_per_slope);
	void calculate_profile ();
	unsigned int ramp_up_accelerated();
	unsigned int move_n_steps_fast_accelerated (unsigned int mov_steps,unsigned int inner_delay);
	void ramp_down_accelerated();
	void move_motor_accel(unsigned int cycles,unsigned int steps, boolean direction);
	void set_speed_in_slow_mode (unsigned int delay_slow_mode);
	//Properties
	int step_accuracy;
	int step_mode;
	long stepPosition;  // check if it can be int
	int stepCycle;

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
    bool _default_direction;	
	int _step_accuracy;				//! Step accuracy is the number of steps we are counting each time me move. This property depends on the motor mode we selected
	int _step_mode;
	int _motor_original_steps;
	// Acceleration parameters
	unsigned int _init_timing;
	unsigned int _ramp_inclination;
	unsigned int _n_slopes_per_mode;
	unsigned int _n_steps_per_slope;	
	unsigned int _acceleration_curve[4][21];
	unsigned int _remaining_steps;
	unsigned int _delay_slow_mode;
};

#endif