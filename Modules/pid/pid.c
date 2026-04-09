#include "pid.h"

void pid_calc(pid_reg *a, float err )
{
    a->eps = a->Kp * err;

    a->integ += a->Ki * 0.5 * (a->eps + a->eps_prev) * Ts;
    if (a->integ > a->max) a->integ = a->max;
    if (a->integ < a->min) a->integ = a->min;

    a->dif = a->Kd * (a->eps - a->eps_prev) * Fs;

    a->out = a->eps + a->integ + a->dif;
    a->eps_prev = a->eps;

    if (a->out > a->max) a->out = a->max;
    if (a->out < a->min) a->out = a->min;

}

void pid_init(pid_reg *a,float max,float min)
{
    a->max = max;
    a->min = min;
}