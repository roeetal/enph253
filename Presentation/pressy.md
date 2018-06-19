# ENPH 257 Presentation

From the website and a some talking to Dylan, here is what I _think_ is what is expected of us. Of course, this is all fluid, and can be changed if something needs changing. 

## General Structure

This is what I imagine as the general structure of the presentation:

For all the shit we discuss, we need to prepare to be interrogated hard.

- Intro (who are we, blah blah blah)
- Problem (I will email/talk to Dylan about this - do we pretend like there is this issue of abducted Ewoks, or just say "we have to navigate this course..."?)
- Why we have "our approach": We want to win, and to build a solid robot while doing so. "Short and to the point, yet necessary"
- How we implemented "our approach": First broke down the competition into risk categories etc.... and started designing from that prespective. High risk (detection, navigation) -> low risk (grabbing, electrical failure, software bug)... 
- What is "our approach": here are our designs.
  - Maybe this layout?:
  - High risk (mech, elec, software). eg. treads, objection detection, climbing, basket
  - Medium risk (mech, elec, software). modular pcbs etc.
  - Low risk (mech, elec, software). software errors etc. (implementation of object detection, different from the idea of object detection itself).
  <!---
  - Overview of Robot (quick, high level, pretty pics and what have you)
  - Mech Components (Overview / Reasons for choices / POC)
  - Elec Components (Overview / Reasons for choices / POC)
    - Block Diagram of electronics, pinouts of TINAH/RPi, etc
  - Software Components (Overview / Reasons for choices / POC)--->
- Conclusion

I think it is also a good idea to have some sort of package of calculations (as well as ones displayed in the slides) to give Dr. Dre and DJ Dyll. We will also have to fit analysis and speed of components in.

## Risk Assessment
If you see something that is missing fill it in please! 

- Can't find Ewoks
  - Poor detection
  - Too computationally expensive
- Can't Grab Ewoks
  - Alignment Issues
  - Overshooting ewoks / procedure required to readjust
- Not enough power
  - Consumption of RPi, TINAH, etc too high

==

This is all WIP - ill start doing block diagrams, filling in power point, etc tmmr




## Presentation

### Intro

Hello ... In case you are not aware, this is what the competition is about: Up ramp, over crevise, through arch and alarm system, up step, over flimsy bridge and onto a zipline, all the while pickin up ewoks. We want to build the best robot and win the competition. This is how we plan on doing so.

## Initial Analysis - Overview

At our first meeting, we come to the conclusion that, provided we are able to reliably build the electrical, mechanical, and software components of the robot, to win the competition, we would have to triumph at the hardest and most risky challenges. These are:

1. Navigating the course (quickly!). We have to figure out, at the points of complexity, steps and bridge, what is the best way to deal with it.
2. Detecting and picking up Ewoks.

### Driving

Navigation is a tricky problem, many different approaches, yet none of them can work alone. Tape follow, edges and bridge, ramps, crevises and steps. Deploying any ramp or attaching to a zipline required extra parts, and risk, and requires time. We thought the best approach would be to include those obstacles in the inherent skillset of the robot - a fucking tank.

### Ewok Retreival

Ewoks in all directions. Too many sensors required. So we using computer vision to locate and direct us towards ewoks. Another time consuming aspect is actually picking up the ewok. So we are planing to implement the tracking and picking up together. Drive towards ewoks while lowering arm (restricted 1 axis of rotation - safer) and pick it up, then return to navigating.

## General structure of robot

STM32 core MCU. Inputs: RPI, tape follower, whiskers. Outputs, Motors control. High level Diagrams, etc.

## Sub-system explanation - for each low level block diagrams + circuit modules.

### Driving

#### Tape Following

4 qrds, pid, stay on tape for most of the course.

#### Whiskers

important for edge detection, corners, bridge.

#### Stealth mode

IR becon detection. Outputs high for interrupt.

#### Treads

Driving and climbing.

### Ewoks

### Detection

RPI neural net...

#### Arm

arms lowers while driving to save time? and picks up ewoks and drops them in basket. Can also be used for center of mass control.

#### Basket

Hooks onto zipline and delivers retreived ewoks to start.

### Control

STM32 F4 MCU
- PWM for motor control
- Interrupts and analog for whiskers to stay on track.
- QRDS and PID for navigation
- Interrupt from IR detector + inputs to confirm frequency.
- Interrupt and heading from Pi to go to Ewok - also PID + memory to return to track.

## Power Budget
MCU and sensors -
RPI -
Motors -

## Timeline

/???????????
