#ifndef CALIBRATION_H
#define CALIBRATION_H
/*if we turn the tracker around the base once and record the compass x , y values
in a perfect world youd get a circle centered on (0,0) but on the real tracker u get a
circle thats shifted away from (0,0) (magnetised parts that turn with the sensor) fix is to substract the centre
and its squashed into an oval (iron near the sensor, prob the stepper motor, bends the field) fix is to stretch it back
*/

//used in Heading.cpp11,12,17 float centredX = magRawX * MAG_SCALE - MX_CENTRE;
const float MX_CENTRE = 5.81f; //where the real centre sits on x
const float MY_CENTRE = 53.55f; //on y
const float MZ_CENTRE = -28.0f; //on z never measured, old 2020 value -7 times 4 for 16-bit, on a level base z barely matters

//used in Heading.cpp14,15 
const float SOFT_XX = 0.9373f;  //how much x shrinks <1
const float SOFT_XY = 0.1896f; //oval tilt and theres only one because the matrix is symmetric
const float SOFT_YY = 1.1053f; //how much y stretches >1

//used in Correction.cpp23 return wrapDeg(sensorHeadingDeg + IMU_TO_ARROW_DEG + MAG_DECLINATION_DEG);
const float IMU_TO_ARROW_DEG = -14.0f; //as sensor measure the xaxis's pointing but we need the base arrow points so the sensor sits about 14deg turned relavtive to the arrow
const float MAG_DECLINATION_DEG = 3.9f; //the compass points to magnetic north and the sun math uses true noth so magentic north is 3.9east of true north so we add 3.9deg


#endif