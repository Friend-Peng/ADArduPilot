/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <AP_Math/AP_Math.h>
#include <AP_HAL/AP_HAL.h>
#include "AR_AttitudeControl.h"
#include <AP_GPS/AP_GPS.h>
#include <AP_Vehicle/AP_Vehicle.h>

extern const AP_HAL::HAL& hal;

const AP_Param::GroupInfo AR_AttitudeControl::var_info[] = {

    // @Param: _STR_RAT_P
    // @DisplayName: Steering control rate P gain
    // @Description: Steering control rate P gain.  Converts the turn rate error (in radians/sec) to a steering control output (in the range -1 to +1)
    // @Range: 0.000 2.000
    // @Increment: 0.001
    // @User: Standard

    // @Param: _STR_RAT_I
    // @DisplayName: Steering control I gain
    // @Description: Steering control I gain.  Corrects long term error between the desired turn rate (in rad/s) and actual
    // @Range: 0.000 2.000
    // @Increment: 0.001
    // @User: Standard

    // @Param: _STR_RAT_IMAX
    // @DisplayName: Steering control I gain maximum
    // @Description: Steering control I gain maximum.  Constrains the steering output (range -1 to +1) that the I term will generate
    // @Range: 0.000 1.000
    // @Increment: 0.01
    // @User: Standard

    // @Param: _STR_RAT_D
    // @DisplayName: Steering control D gain
    // @Description: Steering control D gain.  Compensates for short-term change in desired turn rate vs actual
    // @Range: 0.000 0.400
    // @Increment: 0.001
    // @User: Standard

    // @Param: _STR_RAT_FF
    // @DisplayName: Steering control feed forward
    // @Description: Steering control feed forward
    // @Range: 0.000 3.000
    // @Increment: 0.001
    // @User: Standard

    // @Param: _STR_RAT_FILT
    // @DisplayName: Steering control filter frequency
    // @Description: Steering control input filter.  Lower values reduce noise but add delay.
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _STR_RAT_FLTT
    // @DisplayName: Steering control Target filter frequency in Hz
    // @Description: Target filter frequency in Hz
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _STR_RAT_FLTE
    // @DisplayName: Steering control Error filter frequency in Hz
    // @Description: Error filter frequency in Hz
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _STR_RAT_FLTD
    // @DisplayName: Steering control Derivative term filter frequency in Hz
    // @Description: Derivative filter frequency in Hz
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _STR_RAT_SMAX
    // @DisplayName: Steering slew rate limit
    // @Description: Sets an upper limit on the slew rate produced by the combined P and D gains. If the amplitude of the control action produced by the rate feedback exceeds this value, then the D+P gain is reduced to respect the limit. This limits the amplitude of high frequency oscillations caused by an excessive gain. The limit should be set to no more than 25% of the actuators maximum slew rate to allow for load effects. Note: The gain will not be reduced to less than 10% of the nominal value. A value of zero will disable this feature.
    // @Range: 0 200
    // @Increment: 0.5
    // @User: Advanced

    AP_SUBGROUPINFO(_steer_rate_pid, "_STR_RAT_", 1, AR_AttitudeControl, AC_PID),

    // @Param: _SPEED_P
    // @DisplayName: Speed control P gain
    // @Description: Speed control P gain.  Converts the error between the desired speed (in m/s) and actual speed to a motor output (in the range -1 to +1)
    // @Range: 0.010 2.000
    // @Increment: 0.01
    // @User: Standard

    // @Param: _SPEED_I
    // @DisplayName: Speed control I gain
    // @Description: Speed control I gain.  Corrects long term error between the desired speed (in m/s) and actual speed
    // @Range: 0.000 2.000
    // @Increment: 0.01
    // @User: Standard

    // @Param: _SPEED_IMAX
    // @DisplayName: Speed control I gain maximum
    // @Description: Speed control I gain maximum.  Constrains the maximum motor output (range -1 to +1) that the I term will generate
    // @Range: 0.000 1.000
    // @Increment: 0.01
    // @User: Standard

    // @Param: _SPEED_D
    // @DisplayName: Speed control D gain
    // @Description: Speed control D gain.  Compensates for short-term change in desired speed vs actual
    // @Range: 0.000 0.400
    // @Increment: 0.001
    // @User: Standard

    // @Param: _SPEED_FF
    // @DisplayName: Speed control feed forward
    // @Description: Speed control feed forward
    // @Range: 0.000 0.500
    // @Increment: 0.001
    // @User: Standard

    // @Param: _SPEED_FILT
    // @DisplayName: Speed control filter frequency
    // @Description: Speed control input filter.  Lower values reduce noise but add delay.
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _SPEED_FLTT
    // @DisplayName: Speed control Target filter frequency in Hz
    // @Description: Target filter frequency in Hz
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _SPEED_FLTE
    // @DisplayName: Speed control Error filter frequency in Hz
    // @Description: Error filter frequency in Hz
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _SPEED_FLTD
    // @DisplayName: Speed control Derivative term filter frequency in Hz
    // @Description: Derivative filter frequency in Hz
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _SPEED_SMAX
    // @DisplayName: Speed control slew rate limit
    // @Description: Sets an upper limit on the slew rate produced by the combined P and D gains. If the amplitude of the control action produced by the rate feedback exceeds this value, then the D+P gain is reduced to respect the limit. This limits the amplitude of high frequency oscillations caused by an excessive gain. The limit should be set to no more than 25% of the actuators maximum slew rate to allow for load effects. Note: The gain will not be reduced to less than 10% of the nominal value. A value of zero will disable this feature.
    // @Range: 0 200
    // @Increment: 0.5
    // @User: Advanced

    AP_SUBGROUPINFO(_throttle_speed_pid, "_SPEED_", 2, AR_AttitudeControl, AC_PID),

    // @Param: _ACCEL_MAX
    // @DisplayName: Speed control acceleration (and deceleration) maximum in m/s/s
    // @Description: Speed control acceleration (and deceleration) maximum in m/s/s.  0 to disable acceleration limiting
    // @Range: 0.0 10.0
    // @Increment: 0.1
    // @Units: m/s/s
    // @User: Standard
    AP_GROUPINFO("_ACCEL_MAX", 3, AR_AttitudeControl, _throttle_accel_max, AR_ATTCONTROL_THR_ACCEL_MAX),

    // @Param: _BRAKE
    // @DisplayName: Speed control brake enable/disable
    // @Description: Speed control brake enable/disable. Allows sending a reversed output to the motors to slow the vehicle.
    // @Values: 0:Disable,1:Enable
    // @User: Standard
    AP_GROUPINFO("_BRAKE", 4, AR_AttitudeControl, _brake_enable, 1),

    // @Param: _STOP_SPEED
    // @DisplayName: Speed control stop speed
    // @Description: Speed control stop speed.  Motor outputs to zero once vehicle speed falls below this value
    // @Range: 0.00 0.50
    // @Increment: 0.01
    // @Units: m/s
    // @User: Standard
    AP_GROUPINFO("_STOP_SPEED", 5, AR_AttitudeControl, _stop_speed, AR_ATTCONTROL_STOP_SPEED_DEFAULT),

    // @Param: _STR_ANG_P
    // @DisplayName: Steering control angle P gain
    // @Description: Steering control angle P gain.  Converts the error between the desired heading/yaw (in radians) and actual heading/yaw to a desired turn rate (in rad/sec)
    // @Range: 1.000 10.000
    // @Increment: 0.1
    // @User: Standard
    AP_SUBGROUPINFO(_steer_angle_p, "_STR_ANG_", 6, AR_AttitudeControl, AC_P),

    // @Param: _STR_ACC_MAX
    // @DisplayName: Steering control angular acceleration maximum
    // @Description: Steering control angular acceleration maximum (in deg/s/s).  0 to disable acceleration limiting
    // @Range: 0 1000
    // @Increment: 0.1
    // @Units: deg/s/s
    // @User: Standard
    AP_GROUPINFO("_STR_ACC_MAX", 7, AR_AttitudeControl, _steer_accel_max, AR_ATTCONTROL_STEER_ACCEL_MAX),

    // @Param: _STR_RAT_MAX
    // @DisplayName: Steering control rotation rate maximum
    // @Description: Steering control rotation rate maximum in deg/s.  0 to remove rate limiting
    // @Range: 0 1000
    // @Increment: 0.1
    // @Units: deg/s
    // @User: Standard
    AP_GROUPINFO("_STR_RAT_MAX", 8, AR_AttitudeControl, _steer_rate_max, AR_ATTCONTROL_STEER_RATE_MAX),

    // @Param: _DECEL_MAX
    // @DisplayName: Speed control deceleration maximum in m/s/s
    // @Description: Speed control and deceleration maximum in m/s/s.  0 to use ATC_ACCEL_MAX for deceleration
    // @Range: 0.0 10.0
    // @Increment: 0.1
    // @Units: m/s/s
    // @User: Standard
    AP_GROUPINFO("_DECEL_MAX", 9, AR_AttitudeControl, _throttle_decel_max, 0.00f),

    // @Param: _BAL_P
    // @DisplayName: Pitch control P gain
    // @Description: Pitch control P gain for BalanceBots.  Converts the error between the desired pitch (in radians) and actual pitch to a motor output (in the range -1 to +1)
    // @Range: 0.000 2.000
    // @Increment: 0.01
    // @User: Standard

    // @Param: _BAL_I
    // @DisplayName: Pitch control I gain
    // @Description: Pitch control I gain for BalanceBots.  Corrects long term error between the desired pitch (in radians) and actual pitch
    // @Range: 0.000 2.000
    // @Increment: 0.01
    // @User: Standard

    // @Param: _BAL_IMAX
    // @DisplayName: Pitch control I gain maximum
    // @Description: Pitch control I gain maximum.  Constrains the maximum motor output (range -1 to +1) that the I term will generate
    // @Range: 0.000 1.000
    // @Increment: 0.01
    // @User: Standard

    // @Param: _BAL_D
    // @DisplayName: Pitch control D gain
    // @Description: Pitch control D gain.  Compensates for short-term change in desired pitch vs actual
    // @Range: 0.000 0.100
    // @Increment: 0.001
    // @User: Standard

    // @Param: _BAL_FF
    // @DisplayName: Pitch control feed forward
    // @Description: Pitch control feed forward
    // @Range: 0.000 0.500
    // @Increment: 0.001
    // @User: Standard

    // @Param: _BAL_FILT
    // @DisplayName: Pitch control filter frequency
    // @Description: Pitch control input filter.  Lower values reduce noise but add delay.
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _BAL_FLTT
    // @DisplayName: Pitch control Target filter frequency in Hz
    // @Description: Target filter frequency in Hz
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _BAL_FLTE
    // @DisplayName: Pitch control Error filter frequency in Hz
    // @Description: Error filter frequency in Hz
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _BAL_FLTD
    // @DisplayName: Pitch control Derivative term filter frequency in Hz
    // @Description: Derivative filter frequency in Hz
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _BAL_SMAX
    // @DisplayName: Pitch control slew rate limit
    // @Description: Sets an upper limit on the slew rate produced by the combined P and D gains. If the amplitude of the control action produced by the rate feedback exceeds this value, then the D+P gain is reduced to respect the limit. This limits the amplitude of high frequency oscillations caused by an excessive gain. The limit should be set to no more than 25% of the actuators maximum slew rate to allow for load effects. Note: The gain will not be reduced to less than 10% of the nominal value. A value of zero will disable this feature.
    // @Range: 0 200
    // @Increment: 0.5
    // @User: Advanced

    AP_SUBGROUPINFO(_pitch_to_throttle_pid, "_BAL_", 10, AR_AttitudeControl, AC_PID),

    // @Param: _BAL_SPD_FF
    // @DisplayName: Pitch control feed forward from speed
    // @Description: Pitch control feed forward from speed
    // @Range: 0.0 10.0
    // @Increment: 0.01
    // @User: Standard
    AP_GROUPINFO("_BAL_SPD_FF", 11, AR_AttitudeControl, _pitch_to_throttle_speed_ff, AR_ATTCONTROL_BAL_SPEED_FF),

    // @Param: _SAIL_P
    // @DisplayName: Sail Heel control P gain
    // @Description: Sail Heel control P gain for sailboats.  Converts the error between the desired heel angle (in radians) and actual heel to a main sail output (in the range -1 to +1)
    // @Range: 0.000 2.000
    // @Increment: 0.01
    // @User: Standard

    // @Param: _SAIL_I
    // @DisplayName: Sail Heel control I gain
    // @Description: Sail Heel control I gain for sailboats.  Corrects long term error between the desired heel angle (in radians) and actual
    // @Range: 0.000 2.000
    // @Increment: 0.01
    // @User: Standard

    // @Param: _SAIL_IMAX
    // @DisplayName: Sail Heel control I gain maximum
    // @Description: Sail Heel control I gain maximum.  Constrains the maximum I term contribution to the main sail output (range -1 to +1)
    // @Range: 0.000 1.000
    // @Increment: 0.01
    // @User: Standard

    // @Param: _SAIL_D
    // @DisplayName: Sail Heel control D gain
    // @Description: Sail Heel control D gain.  Compensates for short-term change in desired heel angle vs actual
    // @Range: 0.000 0.100
    // @Increment: 0.001
    // @User: Standard

    // @Param: _SAIL_FF
    // @DisplayName: Sail Heel control feed forward
    // @Description: Sail Heel control feed forward
    // @Range: 0.000 0.500
    // @Increment: 0.001
    // @User: Standard

    // @Param: _SAIL_FILT
    // @DisplayName: Sail Heel control filter frequency
    // @Description: Sail Heel control input filter.  Lower values reduce noise but add delay.
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _SAIL_FLTT
    // @DisplayName: Sail Heel Target filter frequency in Hz
    // @Description: Target filter frequency in Hz
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _SAIL_FLTE
    // @DisplayName: Sail Heel Error filter frequency in Hz
    // @Description: Error filter frequency in Hz
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _SAIL_FLTD
    // @DisplayName: Sail Heel Derivative term filter frequency in Hz
    // @Description: Derivative filter frequency in Hz
    // @Range: 0.000 100.000
    // @Increment: 0.1
    // @Units: Hz
    // @User: Standard

    // @Param: _SAIL_SMAX
    // @DisplayName: Sail heel slew rate limit
    // @Description: Sets an upper limit on the slew rate produced by the combined P and D gains. If the amplitude of the control action produced by the rate feedback exceeds this value, then the D+P gain is reduced to respect the limit. This limits the amplitude of high frequency oscillations caused by an excessive gain. The limit should be set to no more than 25% of the actuators maximum slew rate to allow for load effects. Note: The gain will not be reduced to less than 10% of the nominal value. A value of zero will disable this feature.
    // @Range: 0 200
    // @Increment: 0.5
    // @User: Advanced

    AP_SUBGROUPINFO(_sailboat_heel_pid, "_SAIL_", 12, AR_AttitudeControl, AC_PID),

    AP_GROUPEND
};

void AR_AttitudeControl::pso_read(void)
{
    FILE *fp;
	AP_Vehicle::stRover data;
	
	
    fp=fopen("data_orig.txt","rb");
    if(!fp)
    {
        printf("pso_read open data_orig.txt error!\n");
        fclose(fp);
        exit(-1);
    }

    if(0==fread(&data ,sizeof(data),1,fp))
    {
        printf("pso_read read data_orig.txt error");
        fclose(fp);
        exit(-1);
    }
    fclose(fp);
	
	printf("Rover data:throttle_alpha=%f,throttle_eps=%f,steer_alpha=%f,steer_eps=%f \n", data.throttle_alpha,data.throttle_eps,data.steer_alpha,data.steer_eps);
	_throttle_speed_pid._alpha = data.throttle_alpha;
	_throttle_speed_pid._eps = data.throttle_eps;
	_steer_rate_pid._alpha =  data.steer_alpha;
	_steer_rate_pid._eps = data.steer_eps;
	
    return;
}

AR_AttitudeControl::AR_AttitudeControl(AP_AHRS &ahrs) :
    _ahrs(ahrs),
    _steer_angle_p(AR_ATTCONTROL_STEER_ANG_P),
    _steer_rate_pid(AR_ATTCONTROL_STEER_RATE_P, AR_ATTCONTROL_STEER_RATE_I, AR_ATTCONTROL_STEER_RATE_D, AR_ATTCONTROL_STEER_RATE_FF, AR_ATTCONTROL_STEER_RATE_IMAX, 0.0f, AR_ATTCONTROL_STEER_RATE_FILT, 0.0f, AR_ATTCONTROL_DT),
    _throttle_speed_pid(AR_ATTCONTROL_THR_SPEED_P, AR_ATTCONTROL_THR_SPEED_I, AR_ATTCONTROL_THR_SPEED_D, 0.0f, AR_ATTCONTROL_THR_SPEED_IMAX, 0.0f, AR_ATTCONTROL_THR_SPEED_FILT, 0.0f, AR_ATTCONTROL_DT),
    _pitch_to_throttle_pid(AR_ATTCONTROL_PITCH_THR_P, AR_ATTCONTROL_PITCH_THR_I, AR_ATTCONTROL_PITCH_THR_D, 0.0f, AR_ATTCONTROL_PITCH_THR_IMAX, 0.0f, AR_ATTCONTROL_PITCH_THR_FILT, 0.0f, AR_ATTCONTROL_DT),
    _sailboat_heel_pid(AR_ATTCONTROL_HEEL_SAIL_P, AR_ATTCONTROL_HEEL_SAIL_I, AR_ATTCONTROL_HEEL_SAIL_D, 0.0f, AR_ATTCONTROL_HEEL_SAIL_IMAX, 0.0f, AR_ATTCONTROL_HEEL_SAIL_FILT, 0.0f, AR_ATTCONTROL_DT)
    {
    AP_Param::setup_object_defaults(this, var_info);
	
//			pso_read();
	_throttle_speed_pid._alpha = 0.03;
	_throttle_speed_pid._eps = 0.17;
	_steer_rate_pid._alpha =  6.47;
	_steer_rate_pid._eps = 31.7;    
}

// return a steering servo output from -1.0 to +1.0 given a desired lateral acceleration rate in m/s/s.
// positive lateral acceleration is to the right.
float AR_AttitudeControl::get_steering_out_lat_accel(float desired_accel, bool motor_limit_left, bool motor_limit_right, float dt)
{
    // record desired accel for reporting purposes
    _steer_lat_accel_last_ms = AP_HAL::millis();
    _desired_lat_accel = desired_accel;

    // get speed forward
    float speed;
    if (!get_forward_speed(speed)) {
        // we expect caller will not try to control heading using rate control without a valid speed estimate
        // on failure to get speed we do not attempt to steer
        return 0.0f;
    }

    const float desired_rate = get_turn_rate_from_lat_accel(desired_accel, speed);

    return get_steering_out_rate(desired_rate, motor_limit_left, motor_limit_right, dt);
}

// return a steering servo output from -1 to +1 given a heading in radians
// set rate_max_rads to a non-zero number to apply a limit on the desired turn rate
// return value is normally in range -1.0 to +1.0 but can be higher or lower
float AR_AttitudeControl::get_steering_out_heading(float heading_rad, float rate_max_rads, bool motor_limit_left, bool motor_limit_right, float dt)
{
    // calculate the desired turn rate (in radians) from the angle error (also in radians)
    float desired_rate = get_turn_rate_from_heading(heading_rad, rate_max_rads);

    return get_steering_out_rate(desired_rate, motor_limit_left, motor_limit_right, dt);
}

// return a desired turn-rate given a desired heading in radians
float AR_AttitudeControl::get_turn_rate_from_heading(float heading_rad, float rate_max_rads) const
{
    const float yaw_error = wrap_PI(heading_rad - _ahrs.yaw);

    // Calculate the desired turn rate (in radians) from the angle error (also in radians)
    float desired_rate = _steer_angle_p.get_p(yaw_error);

    // limit desired_rate if a custom pivot turn rate is selected, otherwise use ATC_STR_RAT_MAX
    if (is_positive(rate_max_rads)) {
        desired_rate = constrain_float(desired_rate, -rate_max_rads, rate_max_rads);
    }

    return desired_rate;
}

// return a steering servo output from -1 to +1 given a
// desired yaw rate in radians/sec. Positive yaw is to the right.
float AR_AttitudeControl::get_steering_out_rate(float desired_rate, bool motor_limit_left, bool motor_limit_right, float dt)
{
    // sanity check dt
    dt = constrain_float(dt, 0.0f, 1.0f);

    // if not called recently, reset input filter and desired turn rate to actual turn rate (used for accel limiting)
    const uint32_t now = AP_HAL::millis();
    if ((_steer_turn_last_ms == 0) || ((now - _steer_turn_last_ms) > AR_ATTCONTROL_TIMEOUT_MS)) {
        _steer_rate_pid.reset_filter();
        _steer_rate_pid.reset_I();
        _desired_turn_rate = _ahrs.get_yaw_rate_earth();
    }
    _steer_turn_last_ms = now;

    // acceleration limit desired turn rate
    if (is_positive(_steer_accel_max)) {
        const float change_max = radians(_steer_accel_max) * dt;
        desired_rate = constrain_float(desired_rate, _desired_turn_rate - change_max, _desired_turn_rate + change_max);
    }
    _desired_turn_rate = desired_rate;

    // rate limit desired turn rate
    if (is_positive(_steer_rate_max)) {
        const float steer_rate_max_rad = radians(_steer_rate_max);
        _desired_turn_rate = constrain_float(_desired_turn_rate, -steer_rate_max_rad, steer_rate_max_rad);
    }

    // set PID's dt
    _steer_rate_pid.set_dt(dt);

	float adaptive_term = _steer_rate_pid.get_adaptive_term(100.0);
    float output = _steer_rate_pid.update_all(_desired_turn_rate, _ahrs.get_yaw_rate_earth(), (motor_limit_left || motor_limit_right)) + adaptive_term/100;
//		printf("111111111111111111111111111111     steer_output  %f,   adaptive term  %f \n", output, adaptive_term);
    output += _steer_rate_pid.get_ff();

	//get cost and writo to PSO
	uint32_t tnow = AP_HAL::millis();
	if(tnow>47000)
	{
		sum_steer +=fabs( output) / 359.493214;
		sum_throttle += fabs(throttle_energy) / 1340.019329;
	}
	if(tnow > 150000 && num_steer==0)
	{
		float cost = _steer_rate_pid.get_cost() / 0.016734;    //9.23
		float cost_energy = (sum_steer+sum_throttle) / 2;
	// _kp 0.200000   _ki 0.200000   _kd 0.000000
		printf("Steer control:   cost_steer %f, cost_energy = %f, sum_steer = %f, sum_throttle = %f\n",(double)cost, cost_energy, sum_steer, sum_throttle);

		AP::vehicle()->update_pso_cost(cost, 5);
		AP::vehicle()->update_pso_cost(cost_energy, 6);
		num_steer++;
/*		printf("roll error_collect = [...] and total  number is %d\n",count);
		for(k=0;k<count;k++)
		{
			printf("%f ",error_collect[k]);
		}
		printf("\n");   */
	}	
	
    // constrain and return final output
	
	
#if 1	
static uint32_t flag = 0;
static float throttle[10000] = {0}, steer[10000] = {0}, ethrottle[10000]={0}, esteer[10000]={0};
static uint32_t i = 0;
static uint32_t freq = 0;
uint32_t k = 0;
if(0==flag )  
{
	freq++;
	if( freq == 10)
	{
		throttle[i] = throttle_energy;
		steer[i] = output;
		ethrottle[i] = _throttle_speed_pid.get_error();
		esteer[i] = _steer_rate_pid.get_error();
		i++;
		freq = 0;
	}
}
if(tnow > 150000 && 0 == flag)
{
	flag++;
	printf("throttle = [...] and total  number is %d\n",i);
	for(k=0; k<i;k++)
	{
		 printf("%f ",throttle[k]);			
	}
	printf("\n");
	printf("steer = [...] and total  number is %d\n",i);
	for(k=0; k<i;k++)
	{
		 printf("%f ",steer[k]);			
	}
	printf("\n");
	printf("ethrottle = [...] and total  number is %d\n",i);
	for(k=0; k<i;k++)
	{
		 printf("%f ",ethrottle[k]);			
	}
	printf("\n");
	printf("esteer = [...] and total  number is %d\n",i);
	for(k=0; k<i;k++)
	{
		 printf("%f ",esteer[k]);			
	}
	printf("\n");   
}    
#endif
	
    return output;
}

// get latest desired turn rate in rad/sec (recorded during calls to get_steering_out_rate)
float AR_AttitudeControl::get_desired_turn_rate() const
{
    // return zero if no recent calls to turn rate controller
    if ((_steer_turn_last_ms == 0) || ((AP_HAL::millis() - _steer_turn_last_ms) > AR_ATTCONTROL_TIMEOUT_MS)) {
        return 0.0f;
    }
    return _desired_turn_rate;
}

// get latest desired lateral acceleration in m/s/s (recorded during calls to get_steering_out_lat_accel)
float AR_AttitudeControl::get_desired_lat_accel() const
{
    // return zero if no recent calls to lateral acceleration controller
    if ((_steer_lat_accel_last_ms == 0) || ((AP_HAL::millis() - _steer_lat_accel_last_ms) > AR_ATTCONTROL_TIMEOUT_MS)) {
        return 0.0f;
    }
    return _desired_lat_accel;
}

// get actual lateral acceleration in m/s/s.  returns true on success
bool AR_AttitudeControl::get_lat_accel(float &lat_accel) const
{
    float speed;
    if (!get_forward_speed(speed)) {
        return false;
    }
    lat_accel = speed * _ahrs.get_yaw_rate_earth();
    return true;
}

// calculate the turn rate in rad/sec given a lateral acceleration (in m/s/s) and speed (in m/s)
float AR_AttitudeControl::get_turn_rate_from_lat_accel(float lat_accel, float speed) const
{
    // enforce minimum speed to stop oscillations when first starting to move
    if (fabsf(speed) < AR_ATTCONTROL_STEER_SPEED_MIN) {
        if (is_negative(speed)) {
            speed = -AR_ATTCONTROL_STEER_SPEED_MIN;
        } else {
            speed = AR_ATTCONTROL_STEER_SPEED_MIN;
        }
    }

    return lat_accel / speed;
}

// return a throttle output from -1 to +1 given a desired speed in m/s (use negative speeds to travel backwards)
//   motor_limit should be true if motors have hit their upper or lower limits
//   cruise speed should be in m/s, cruise throttle should be a number from -1 to +1
float AR_AttitudeControl::get_throttle_out_speed(float desired_speed, bool motor_limit_low, bool motor_limit_high, float cruise_speed, float cruise_throttle, float dt)
{
    // sanity check dt
    dt = constrain_float(dt, 0.0f, 1.0f);

    // get speed forward
    float speed;
    if (!get_forward_speed(speed)) {
        // we expect caller will not try to control heading using rate control without a valid speed estimate
        // on failure to get speed we do not attempt to steer
        return 0.0f;
    }

    // if not called recently, reset input filter and desired speed to actual speed (used for accel limiting)
    if (!speed_control_active()) {
        _throttle_speed_pid.reset_filter();
        _throttle_speed_pid.reset_I();
        _desired_speed = speed;
    }
    _speed_last_ms = AP_HAL::millis();

    // acceleration limit desired speed
    _desired_speed = get_desired_speed_accel_limited(desired_speed, dt);

    // set PID's dt
    _throttle_speed_pid.set_dt(dt);

    // calculate base throttle (protect against divide by zero)
    float throttle_base = 0.0f;
    if (is_positive(cruise_speed) && is_positive(cruise_throttle)) {
        throttle_base = desired_speed * (cruise_throttle / cruise_speed);
    }

    // calculate final output
	float adaptive_term = _throttle_speed_pid.get_adaptive_term(1.0);
    float throttle_out = _throttle_speed_pid.update_all(desired_speed, speed, (_throttle_limit_low || _throttle_limit_high)) + adaptive_term;
	//	printf("111111111111111111111111111111     throttle_out  %f,   adaptive term  %f \n", throttle_out, adaptive_term);

	//get cost and writo to PSO
	uint32_t tnow = AP_HAL::millis();
	if(tnow > 150000 && num_throttle==0)
	{
		float cost = _throttle_speed_pid.get_cost() / 0.03867;    //9.23

	// _kp 0.100000   _ki 0.200000   _kd 0.000000
		printf("Throttle control:   cost_throttle %f\n",(double)cost);

		AP::vehicle()->update_pso_cost(cost, 1);
		num_throttle++;
/*		printf("roll error_collect = [...] and total  number is %d\n",count);
		for(k=0;k<count;k++)
		{
			printf("%f ",error_collect[k]);
		}
		printf("\n");   */
	}	
    throttle_out += _throttle_speed_pid.get_ff();
    throttle_out += throttle_base;
	throttle_energy = throttle_out;

    // clear local limit flags used to stop i-term build-up as we stop reversed outputs going to motors
    _throttle_limit_low = false;
    _throttle_limit_high = false;

    // protect against reverse output being sent to the motors unless braking has been enabled
    if (!_brake_enable) {
        // if both desired speed and actual speed are positive, do not allow negative values
        if ((desired_speed >= 0.0f) && (throttle_out <= 0.0f)) {
            throttle_out = 0.0f;
            _throttle_limit_low = true;
        } else if ((desired_speed <= 0.0f) && (throttle_out >= 0.0f)) {
            throttle_out = 0.0f;
            _throttle_limit_high = true;
        }
    }
	
    // final output throttle in range -1 to 1
    return throttle_out;
}

// return a throttle output from -1 to +1 to perform a controlled stop.  returns true once the vehicle has stopped
float AR_AttitudeControl::get_throttle_out_stop(bool motor_limit_low, bool motor_limit_high, float cruise_speed, float cruise_throttle, float dt, bool &stopped)
{
    // get current system time
    const uint32_t now = AP_HAL::millis();

    // if we were stopped in the last 300ms, assume we are still stopped
    bool _stopped = (_stop_last_ms != 0) && (now - _stop_last_ms) < 300;

    // get deceleration limited speed
    float desired_speed_limited = get_desired_speed_accel_limited(0.0f, dt);

    // get speed forward
    float speed;
    if (!get_forward_speed(speed)) {
        // could not get speed so assume stopped
        _stopped = true;
    } else {
        // if desired speed is zero and vehicle drops below _stop_speed consider it stopped
        if (is_zero(desired_speed_limited) && fabsf(speed) <= fabsf(_stop_speed)) {
            _stopped = true;
        }
    }

    // set stopped status for caller
    stopped = _stopped;

    // if stopped return zero
    if (stopped) {
        // update last time we thought we were stopped
        _stop_last_ms = now;
        // set last time speed controller was run so accelerations are limited
        _speed_last_ms = now;
        return 0.0f;
    }

    // clear stopped system time
    _stop_last_ms = 0;
    // run speed controller to bring vehicle to stop
    return get_throttle_out_speed(desired_speed_limited, motor_limit_low, motor_limit_high, cruise_speed, cruise_throttle, dt);
}

// balancebot pitch to throttle controller
// returns a throttle output from -100 to +100 given a desired pitch angle and vehicle's current speed (from wheel encoders)
// desired_pitch is in radians, veh_speed_pct is supplied as a percentage (-100 to +100) of vehicle's top speed
float AR_AttitudeControl::get_throttle_out_from_pitch(float desired_pitch, float vehicle_speed_pct, bool motor_limit_low, bool motor_limit_high, float dt)
{
    // sanity check dt
    dt = constrain_float(dt, 0.0f, 1.0f);

    // if not called recently, reset input filter
    const uint32_t now = AP_HAL::millis();
    if ((_balance_last_ms == 0) || ((now - _balance_last_ms) > AR_ATTCONTROL_TIMEOUT_MS)) {
        _pitch_to_throttle_pid.reset_filter();
        _pitch_to_throttle_pid.reset_I();
    }
    _balance_last_ms = now;

    // set PID's dt
    _pitch_to_throttle_pid.set_dt(dt);

    // add feed forward from speed
    float output = vehicle_speed_pct * 0.01f * _pitch_to_throttle_speed_ff;
    output += _pitch_to_throttle_pid.update_all(desired_pitch, _ahrs.pitch, (motor_limit_low || motor_limit_high));
    output += _pitch_to_throttle_pid.get_ff();
	printf("33333333333333333333333333333333\n");

    // constrain and return final output
    return output;
}

// get latest desired pitch in radians for reporting purposes
float AR_AttitudeControl::get_desired_pitch() const
{
    // if not called recently, return 0
    if ((_balance_last_ms == 0) || ((AP_HAL::millis() - _balance_last_ms) > AR_ATTCONTROL_TIMEOUT_MS)) {
        return 0.0f;
    }

    return _pitch_to_throttle_pid.get_pid_info().target;
}

// Sailboat heel(roll) angle controller releases sail to keep at maximum heel angle
// but does not attempt to reach maximum heel angle, ie only lets sails out, does not pull them in
float AR_AttitudeControl::get_sail_out_from_heel(float desired_heel, float dt)
{
    // sanity check dt
    dt = constrain_float(dt, 0.0f, 1.0f);

    // if not called recently, reset input filter
    const uint32_t now = AP_HAL::millis();
    if ((_heel_controller_last_ms == 0) || ((now - _heel_controller_last_ms) > AR_ATTCONTROL_TIMEOUT_MS)) {
        _sailboat_heel_pid.reset_filter();
        _sailboat_heel_pid.reset_I();
    }
    _heel_controller_last_ms = now;

    // set PID's dt
    _sailboat_heel_pid.set_dt(dt);

    _sailboat_heel_pid.update_all(desired_heel, fabsf(_ahrs.roll));
printf("444444444444444444444444444444444444\n");
    // get feed-forward
    const float ff = _sailboat_heel_pid.get_ff();

    // get p, constrain to be zero or negative
    float p = _sailboat_heel_pid.get_p();
    if (is_positive(p)) {
        p = 0.0f;
    }

    // get i, constrain to be zero or negative
    float i = _sailboat_heel_pid.get_i();
    if (is_positive(i)) {
        i = 0.0f;
        _sailboat_heel_pid.reset_I();
    }

    // get d
    const float d = _sailboat_heel_pid.get_d();

    // constrain and return final output
    return (ff + p + i + d) * -1.0f;
}

// get forward speed in m/s (earth-frame horizontal velocity but only along vehicle x-axis).  returns true on success
bool AR_AttitudeControl::get_forward_speed(float &speed) const
{
    Vector3f velocity;
    if (!_ahrs.get_velocity_NED(velocity)) {
        // use less accurate GPS, assuming entire length is along forward/back axis of vehicle
        if (AP::gps().status() >= AP_GPS::GPS_OK_FIX_3D) {
            if (abs(wrap_180_cd(_ahrs.yaw_sensor - AP::gps().ground_course_cd())) <= 9000) {
                speed = AP::gps().ground_speed();
            } else {
                speed = -AP::gps().ground_speed();
            }
            return true;
        } else {
            return false;
        }
    }
    // calculate forward speed velocity into body frame
    speed = velocity.x*_ahrs.cos_yaw() + velocity.y*_ahrs.sin_yaw();
    return true;
}

float AR_AttitudeControl::get_decel_max() const
{
    if (is_positive(_throttle_decel_max)) {
        return _throttle_decel_max;
    } else {
        return _throttle_accel_max;
    }
}

// check if speed controller active
bool AR_AttitudeControl::speed_control_active() const
{
    // active if there have been recent calls to speed controller
    if ((_speed_last_ms == 0) || ((AP_HAL::millis() - _speed_last_ms) > AR_ATTCONTROL_TIMEOUT_MS)) {
        return false;
    }
    return true;
}

// get latest desired speed recorded during call to get_throttle_out_speed.  For reporting purposes only
float AR_AttitudeControl::get_desired_speed() const
{
    // return zero if no recent calls to speed controller
    if (!speed_control_active()) {
        return 0.0f;
    }
    return _desired_speed;
}

// get acceleration limited desired speed
float AR_AttitudeControl::get_desired_speed_accel_limited(float desired_speed, float dt) const
{
    // return input value if no recent calls to speed controller
    // apply no limiting when ATC_ACCEL_MAX is set to zero
    if (!speed_control_active() || !is_positive(_throttle_accel_max)) {
        return desired_speed;
    }

    // sanity check dt
    dt = constrain_float(dt, 0.0f, 1.0f);

    // use previous desired speed as basis for accel limiting
    float speed_prev = _desired_speed;

    // if no recent calls to speed controller limit based on current speed
    if (!speed_control_active()) {
        get_forward_speed(speed_prev);
    }

    // acceleration limit desired speed
    float speed_change_max;
    if (fabsf(desired_speed) < fabsf(_desired_speed) && is_positive(_throttle_decel_max)) {
        speed_change_max = _throttle_decel_max * dt;
    } else {
        speed_change_max = _throttle_accel_max * dt;
    }
    return constrain_float(desired_speed, speed_prev - speed_change_max, speed_prev + speed_change_max);
}

// get minimum stopping distance (in meters) given a speed (in m/s)
float AR_AttitudeControl::get_stopping_distance(float speed) const
{
    // get maximum vehicle deceleration
    const float accel_max = get_accel_max();

    // avoid divide by zero
    if ((accel_max <= 0.0f) || is_zero(speed)) {
        return 0.0f;
    }

    // assume linear deceleration
    return 0.5f * sq(speed) / accel_max;
}

// relax I terms of throttle and steering controllers
void AR_AttitudeControl::relax_I()
{
    _steer_rate_pid.reset_I();
    _throttle_speed_pid.reset_I();
    _pitch_to_throttle_pid.reset_I();
}
