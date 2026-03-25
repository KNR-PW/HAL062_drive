#ifndef __PID_H__
#define __PID_H__

#define Ts 0.01
#define Fs 100

typedef struct
{
    float Kp;
    float Ki;
    float Kd;

    float max;
    float min;

    float eps;
    float eps_prev;

    float integ;
    float dif;

    float out;
}pid_reg;


void pid_calc(pid_reg *a, float err );


#endif