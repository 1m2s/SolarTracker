#ifndef CALIBRATION_H
#define CALIBRATION_H
/*if we turn the tracker around the base once and record the compass x , y values
in a perfect world youd get a circle centered on (0,0) but on the real tracker u get an 
oval(due to iron near the sensor prob due to stepper motor that bends the field) fix is to substract the center
thats shifted away from the centre (Magnetised parts that turn with the sensor) fix is to strech it back
*/

//used in Heading.cpp11,12,17 float centredX = magRawX * MAG_SCALE - MX_CENTRE;
const float MX_CENTRE = 2.08f; //where the real centre sits on x
const float MY_CENTRE = 14.05f; //on y
const float MZ_CENTRE = -7.0f; //on z didnt change from old values as on a level base z barley matters

//used in Heading.cpp14,15 
const float SOFT_XX = 0.9529f;  //how much X shrinks <1
const float SOFT_XY = 0.1789f; //how much y stretches >1
const float SOFT_YY = 1.0830f; //oval tilt and theres only once because the matrix is symmetric 

//used in Correction.cpp23 return wrapDeg(sensorHeadingDeg + IMU_TO_ARROW_DEG + MAG_DECLINATION_DEG);
const float IMU_TO_ARROW_DEG = -14.0f; //as sensor measure the xaxis's pointing but we need the base arrow points so the sensor sits about 14deg turned relavtive to the arrow
const float MAG_DECLINATION_DEG = 3.9f; //the compass points to magnetic north and the sun math uses true noth so magentic north is 3.9east of true north so we add 3.9deg


#endif