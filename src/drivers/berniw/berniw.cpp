/***************************************************************************

 file                 : berniw.cpp
 created              : Mon Apr 17 13:51:00 CET 2000
 copyright            : (C) 2000-2002 by Bernhard Wymann
 email                : berniw@bluewin.ch
 version              : $Id: berniw.cpp,v 1.38.2.4 2013/08/05 17:22:43 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "berniw.h"
#include <portability.h>

#ifdef DMALLOC
#include "dmalloc.h"
#endif

/* function prototypes */
static int InitFuncPt(int index, void *pt);
static void initTrack(int index, tTrack* track, void *carHandle,
		void **carParmHandle, tSituation * situation);
static void drive(int index, tCarElt* car, tSituation *situation);
static void newRace(int index, tCarElt* car, tSituation *situation);
static int pitcmd(int index, tCarElt* car, tSituation *s);
static void shutdown(int index);

/* Hwancheol */
static double target_speed_bn = 14;
static double calculate_CC(bool updown);
static double car_speed = 0.0;
static double* dist_to_ocar;
static double* speed_ocar;
static const char* botname[BOTS] = { "berniw 1", "berniw 2", "berniw 3",
		"berniw 4", "berniw 5", "berniw 6", "berniw 7", "berniw 8", "berniw 9",
		"berniw 10" };

static const char* botdesc[BOTS] = { "berniw 1", "berniw 2", "berniw 3",
		"berniw 4", "berniw 5", "berniw 6", "berniw 7", "berniw 8", "berniw 9",
		"berniw 10" };

/* Module entry point */
extern "C" int berniw(tModInfo *modInfo) {
	//char	buffer[BUFSIZE];

	for (int i = 0; i < BOTS; i++) {
		modInfo[i].name = strdup(botname[i]); /* name of the module (short) */
		modInfo[i].desc = strdup(botdesc[i]); /* description of the module (can be long) */
		modInfo[i].fctInit = InitFuncPt; /* init function */
		modInfo[i].gfId = ROB_IDENT; /* supported framework version */
		modInfo[i].index = i + 1;
	}
	return 0;
}

/* initialize function (callback) pointers for torcs */
static int InitFuncPt(int index, void *pt) {
	tRobotItf *itf = (tRobotItf *) pt;

	itf->rbNewTrack = initTrack; /* init new track */
	itf->rbNewRace = newRace; /* init new race */
	itf->rbDrive = drive; /* drive during race */
	itf->rbShutdown = shutdown; /* called for cleanup per driver */
	itf->rbPitCmd = pitcmd; /* pit command */
	itf->index = index;
	return 0;
}

static MyCar* mycar[BOTS] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL };
static OtherCar* ocar = NULL;
static TrackDesc* myTrackDesc = NULL;
static double currenttime;
static const tdble waitToTurn = 1.0; /* how long should i wait till i try to turn backwards */

/* release resources when the module gets unloaded */
static void shutdown(int index) {
	int i = index - 1;
	if (mycar[i] != NULL) {
		delete mycar[i];
		mycar[i] = NULL;
		//free(botdesc[i]);
		//free(botname[i]);
	}
	if (myTrackDesc != NULL) {
		delete myTrackDesc;
		myTrackDesc = NULL;
	}
	if (ocar != NULL) {
		delete[] ocar;
		ocar = NULL;
	}
	delete dist_to_ocar;
	delete speed_ocar;
}

/* initialize track data, called for every selected driver */
static void initTrack(int index, tTrack* track, void *carHandle,
		void **carParmHandle, tSituation * situation) {
	if ((myTrackDesc != NULL) && (myTrackDesc->getTorcsTrack() != track)) {
		delete myTrackDesc;
		myTrackDesc = NULL;
	}
	if (myTrackDesc == NULL) {
		myTrackDesc = new TrackDesc(track);
	}

	char buffer[BUFSIZE];
	char* trackname = strrchr(track->filename, '/') + 1;

	snprintf(buffer, BUFSIZE, "drivers/berniw/%d/%s", index, trackname);
	*carParmHandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);

	if (*carParmHandle == NULL) {
		snprintf(buffer, BUFSIZE, "drivers/berniw/%d/default.xml", index);
		*carParmHandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);
	}

	/* Load and set parameters */
	float fuel = GfParmGetNum(*carParmHandle, BERNIW_SECT_PRIV,
	BERNIW_ATT_FUELPERLAP, (char*) NULL,
			track->length * MyCar::MAX_FUEL_PER_METER);
	fuel *= (situation->_totLaps + 1.0);
	GfParmSetNum(*carParmHandle, SECT_CAR, PRM_FUEL, (char*) NULL,
			MIN(fuel, 100.0));
}

/* initialize driver for the race, called for every selected driver */
static void newRace(int index, tCarElt* car, tSituation *situation) {
	if (ocar != NULL)
		delete[] ocar;
	dist_to_ocar = new double();
	speed_ocar = new double();
	ocar = new OtherCar[situation->_ncars];
	for (int i = 0; i < situation->_ncars; i++) {
		ocar[i].init(myTrackDesc, situation->cars[i], situation);
	}

	if (mycar[index - 1] != NULL)
		delete mycar[index - 1];
	mycar[index - 1] = new MyCar(myTrackDesc, car, situation);

	currenttime = situation->currentTime;
}

/* controls the car */
static void drive(int index, tCarElt* car, tSituation *situation) {
	*dist_to_ocar = 10000000.0;
	tdble angle;
	tdble brake;
	tdble b1; /* brake value in case we are to fast HERE and NOW */
	tdble b2; /* brake value for some brake point in front of us */
	tdble b3; /* brake value for control (avoid loosing control) */
	tdble b4; /* brake value for avoiding high angle of attack */
	tdble b5;							// Brake for the pit;
	tdble steer, targetAngle, shiftaccel;

	MyCar* myc = mycar[index - 1];
	Pathfinder* mpf = myc->getPathfinderPtr();

	b1 = b2 = b3 = b4 = b5 = 0.0;
	shiftaccel = 0.0;
	double raced_dist = myc->getCarPtr()->race.distRaced;
	double raced_dist_o = 0;
	/* update some values needed */
	myc->update(myTrackDesc, car, situation);
	if (car->pub.trkPos.toLeft < car->pub.trkPos.toRight)
		myc->isonLeft = true;
	else
		myc->isonLeft = false;
	car_speed = myc->getSpeed();
	/* decide how we want to drive */
	if (car->_dammage < myc->undamaged / 3 && myc->bmode != myc->NORMAL) {
		myc->loadBehaviour(myc->NORMAL);
	} else if (car->_dammage > myc->undamaged / 3
			&& car->_dammage < (myc->undamaged * 2) / 3
			&& myc->bmode != myc->CAREFUL) {
		myc->loadBehaviour(myc->CAREFUL);
	} else if (car->_dammage > (myc->undamaged * 2) / 3
			&& myc->bmode != myc->SLOW) {
		myc->loadBehaviour(myc->SLOW);
	}

	/* update the other cars just once */
	if (currenttime != situation->currentTime) {
		currenttime = situation->currentTime;
		double temp = *dist_to_ocar;
		for (int i = 0; i < situation->_ncars; i++) {
			ocar[i].update();
			temp = sqrt(
					pow((myc->getCurrentPos()->x - ocar[i].getCurrentPos()->x),
							2.0)
							+ pow(
									(myc->getCurrentPos()->y
											- ocar[i].getCurrentPos()->y),
									2.0));
			raced_dist_o = ocar[i].getCarPtr()->race.distRaced;
			if (ocar[i].getCarPtr()->pub.trkPos.toLeft
					< ocar[i].getCarPtr()->pub.trkPos.toRight)
				ocar[i].isonLeft = true;
			else
				ocar[i].isonLeft = false;
			if (temp != 0 && (raced_dist_o - raced_dist) > 0 && myc->isonLeft == ocar[i].isonLeft) {
				*speed_ocar = ocar[i].getCarPtr()->_speed_x;
				*dist_to_ocar = MAX(temp, *dist_to_ocar);
			}
		}
	}
	/* startmode */
	if (myc->trtime < 5.0 && myc->bmode != myc->START) {
		myc->loadBehaviour(myc->START);
		myc->startmode = true;
	}
	if (myc->startmode && myc->trtime > 5.0) {
		myc->startmode = false;
		myc->loadBehaviour(myc->NORMAL);
	}

	/* compute path according to the situation */
	mpf->plan(myc->getCurrentSegId(), car, situation, myc, ocar);

	/* clear ctrl structure with zeros and set the current gear */
	memset(&car->ctrl, 0, sizeof(tCarCtrl));
	car->_gearCmd = car->_gear;

	/* uncommenting the following line causes pitstop on every lap */
	//if (!mpf->getPitStop()) mpf->setPitStop(true, myc->getCurrentSegId());
	/* compute fuel consumption */
	if (myc->getCurrentSegId() >= 0 && myc->getCurrentSegId() < 5
			&& !myc->fuelchecked) {
		if (car->race.laps > 0) {
			myc->fuelperlap = MAX(myc->fuelperlap,
					(myc->lastfuel + myc->lastpitfuel - car->priv.fuel));
		}
		myc->lastfuel = car->priv.fuel;
		myc->lastpitfuel = 0.0;
		myc->fuelchecked = true;
	} else if (myc->getCurrentSegId() > 5) {
		myc->fuelchecked = false;
	}

	/* decide if we need a pit stop */
	if (!mpf->getPitStop() && (car->_remainingLaps - car->_lapsBehindLeader) > 0
			&& (car->_dammage > myc->MAXDAMMAGE
					|| (car->priv.fuel < 1.5 * myc->fuelperlap
							&& car->priv.fuel
									< (car->_remainingLaps
											- car->_lapsBehindLeader)
											* myc->fuelperlap))) {
		mpf->setPitStop(true, myc->getCurrentSegId());
	}

	if (mpf->getPitStop()) {
		car->_raceCmd = RM_CMD_PIT_ASKED;
		// Check if we are almost in the pit to set brake to the max to avoid overrun.
		tdble dl, dw;
		RtDistToPit(car, myTrackDesc->getTorcsTrack(), &dl, &dw);
		if (dl < 1.0f) {
			b5 = 1.0f;
		}
	}

	/* steer to next target point */
//	targetAngle = atan2(myc->destpathseg->getLoc()->y - car->_pos_Y,
//			myc->destpathseg->getLoc()->x - car->_pos_X);
//	targetAngle -= car->_yaw;
//	NORM_PI_PI(targetAngle);
//	steer = targetAngle / car->_steerLock;
	float length = 0.0;
	float angle_ = 0.0;
	float dist = 0.0;
	int check = 0;
	tTrackSeg *seg = car->_trkPos.seg;
	float track_info;
	float lookahead_const = 17.0;	// m
	float lookahead_factor = 0.33;	// 1/s
	float lookahead_ = lookahead_const + car->_speed_x * lookahead_factor;
	v2d re;
	v2d a;

	/* LKAS */
	// enhanced steering (based on track info. and current position)
	if (car->_trkPos.seg->type == TR_STR)
		length = car->_trkPos.seg->length - car->_trkPos.toStart;
	else
		length = (car->_trkPos.seg->arc - car->_trkPos.toStart)
				* car->_trkPos.seg->radius;

	while (length < lookahead_) {
		seg = seg->next;
		length += seg->length;
	}

	length = lookahead_ - length + seg->length;
	if (car->_trkPos.toRight < car->_trkPos.toLeft) {
		a.x = (seg->vertex[TR_SL].x * 4 / 15 + seg->vertex[TR_SR].x * 11 / 15);
		a.y = (seg->vertex[TR_SL].y * 4 / 15 + seg->vertex[TR_SR].y * 11 / 15);
	} else {
		a.x = (seg->vertex[TR_SL].x * 11 / 15 + seg->vertex[TR_SR].x * 4 / 15);
		a.y = (seg->vertex[TR_SL].y * 11 / 15 + seg->vertex[TR_SR].y * 4 / 15);
	}

	if (seg->type == TR_STR) {
		v2d d;
		d.x = (seg->vertex[TR_EL].x - seg->vertex[TR_SL].x) / seg->length;
		d.y = (seg->vertex[TR_EL].y - seg->vertex[TR_SL].y) / seg->length;
		re = a + d * length;
	} else {
		v2d c;
		c.x = seg->center.x;
		c.y = seg->center.y;
		float arc = length / seg->radius;
		float arcsign = (seg->type == TR_RGT) ? -1 : 1;
		arc = arc * arcsign;
		re = a.rotate(c, arc);
	}

	angle_ = atan2(re.y - car->_pos_Y, re.x - car->_pos_X);
	angle_ -= car->_yaw;
	NORM_PI_PI(angle_);

	steer = angle_;
	/* brakes */
	tdble brakecoeff = 1.0
			/ (2.0 * g * myc->currentseg->getKfriction() * myc->CFRICTION);
	tdble brakespeed, brakedist;
	tdble lookahead = 0.0;
	int i = myc->getCurrentSegId();
	brake = 0.0;

	while (lookahead < brakecoeff * myc->getSpeedSqr()) {
		lookahead += mpf->getPathSeg(i)->getLength();
		brakespeed = myc->getSpeedSqr() - mpf->getPathSeg(i)->getSpeedsqr();
		if (brakespeed > 0.0) {
			tdble gm, qb, qs;
			gm =
					myTrackDesc->getSegmentPtr(myc->getCurrentSegId())->getKfriction()
							* myc->CFRICTION
							* myTrackDesc->getSegmentPtr(myc->getCurrentSegId())->getKalpha();
			qs = mpf->getPathSeg(i)->getSpeedsqr();
			brakedist = brakespeed
					* (myc->mass
							/ (2.0 * gm * g * myc->mass
									+ qs * (gm * myc->ca + myc->cw)));

			if (brakedist > lookahead - myc->getWheelTrack()) {
				qb = brakespeed * brakecoeff / brakedist;
				if (qb > b2) {
					b2 = qb;
				}
			}
		}
		i = (i + 1 + mpf->getnPathSeg()) % mpf->getnPathSeg();
	}

	if (myc->getSpeedSqr() > myc->currentpathseg->getSpeedsqr()) {
		b1 = (myc->getSpeedSqr() - myc->currentpathseg->getSpeedsqr())
				/ (myc->getSpeedSqr());
	}

	/* try to avoid flying */
	if (myc->getDeltaPitch() > myc->MAXALLOWEDPITCH
			&& myc->getSpeed() > myc->FLYSPEED) {
		b4 = 1.0;
	}

	if (!mpf->getPitStop()) {
		steer = steer + MIN(0.1, myc->derror*0.02) * myc->getErrorSgn();
		if (fabs(steer) > 1.0)
			steer /= fabs(steer);
	}

	/* check if we are on the way */
	if (myc->getSpeed() > myc->TURNSPEED && myc->tr_mode == 0) {
		if (myc->derror > myc->PATHERR) {
			v3d r;
			myc->getDir()->crossProduct(myc->currentpathseg->getDir(), &r);
			if (r.z * myc->getErrorSgn() >= 0.0) {
				targetAngle = atan2(myc->currentpathseg->getDir()->y,
						myc->currentpathseg->getDir()->x);
				targetAngle -= car->_yaw;
				NORM_PI_PI(targetAngle);
				double toborder = MAX(1.0,
						myc->currentseg->getWidth() / 2.0
								- fabs(
										myTrackDesc->distToMiddle(
												myc->getCurrentSegId(),
												myc->getCurrentPos())));
				b3 = (myc->getSpeed() / myc->STABLESPEED)
						* (myc->derror - myc->PATHERR) / toborder;
			}
		}
	}

	/* try to control angular velocity */
	double omega = myc->getSpeed() / myc->currentpathseg->getRadius();
	steer += 0.1 * (omega - myc->getCarPtr()->_yaw_rate);

	/* anti blocking and brake code */
	if (b1 > b2)
		brake = b1;
	else
		brake = b2;
	if (brake < b3)
		brake = b3;
	if (brake < b4) {
		brake = MIN(1.0, b4);
		tdble abs_mean;
		abs_mean = (car->_wheelSpinVel(REAR_LFT)+ car->_wheelSpinVel(REAR_RGT))*car->_wheelRadius(REAR_LFT)/myc->getSpeed();
		abs_mean /= 2.0;
		brake = brake * abs_mean;
	} else {
		brake = MIN(1.0, brake);
		tdble abs_min = 1.0;
		for (int i = 0; i < 4; i++) {
			tdble slip = car->_wheelSpinVel(i)* car->_wheelRadius(i) / myc->getSpeed();
			if (slip < abs_min) abs_min = slip;
		}
		brake = brake * abs_min;
	}

	float weight = myc->mass * G;
	float maxForce = weight + myc->ca * myc->MAX_SPEED * myc->MAX_SPEED;
	float force = weight + myc->ca * myc->getSpeedSqr();
	brake = brake * MIN(1.0, force / maxForce);
	if (b5 > 0.0f) {
		brake = b5;
	}

	// Gear changing.
	if (myc->tr_mode == 0) {
		if (car->_gear <= 0) {
			car->_gearCmd = 1;
		} else {
			float gr_up = car->_gearRatio[car->_gear + car->_gearOffset];
			float omega = car->_enginerpmRedLine / gr_up;
			float wr = car->_wheelRadius(2);

			if (omega * wr * myc->SHIFT < car->_speed_x) {
				car->_gearCmd++;
			} else {
				float gr_down = car->_gearRatio[car->_gear + car->_gearOffset
						- 1];
				omega = car->_enginerpmRedLine / gr_down;
				if (car->_gear > 1
						&& omega * wr * myc->SHIFT
								> car->_speed_x + myc->SHIFT_MARGIN) {
					car->_gearCmd--;
				}
			}
		}
	}

	tdble cerror, cerrorh;
	cerrorh = sqrt(
			car->_speed_x * car->_speed_x + car->_speed_y * car->_speed_y);
	if (cerrorh > myc->TURNSPEED)
		cerror = fabs(car->_speed_x) / cerrorh;
	else
		cerror = 1.0;

	/* acceleration / brake execution */
	if (myc->tr_mode == 0) {
		if (brake > 0.0) {
			myc->accel = 0.0;
			car->_accelCmd = myc->accel;
			car->_brakeCmd = brake * cerror;
		} else {
			if (myc->getSpeedSqr()
					< mpf->getPathSeg(myc->getCurrentSegId())->getSpeedsqr()) {
				if (myc->accel < myc->ACCELLIMIT) {
					myc->accel += myc->ACCELINC;
				}
				car->_accelCmd = myc->accel / cerror;
			} else {
				if (myc->accel > 0.0) {
					myc->accel -= myc->ACCELINC;
				}
				car->_accelCmd = myc->accel = MIN(myc->accel / cerror,
						shiftaccel / cerror);
			}
			tdble slipspeed = myc->querySlipSpeed(car);
			if (slipspeed > myc->TCL_SLIP) {
				car->_accelCmd = car->_accelCmd
						- MIN(car->_accelCmd,
								(slipspeed - myc->TCL_SLIP) / myc->TCL_RANGE);
			}
		}
	}
	/* for test // Hwancheol */
	target_speed_bn = car->pub.target_speed;
	car->_accelCmd = calculate_CC(true);
	car->_brakeCmd = calculate_CC(false);

	/* check if we are stuck, try to get unstuck */
	tdble bx = myc->getDir()->x, by = myc->getDir()->y;
	tdble cx = myc->currentseg->getMiddle()->x - car->_pos_X, cy =
			myc->currentseg->getMiddle()->y - car->_pos_Y;
	tdble parallel = (cx * bx + cy * by)
			/ (sqrt(cx * cx + cy * cy) * sqrt(bx * bx + by * by));

	if ((myc->getSpeed() < myc->TURNSPEED)
			&& (parallel < cos(90.0 * PI / 180.0))
			&& (mpf->dist2D(myc->getCurrentPos(),
					mpf->getPathSeg(myc->getCurrentSegId())->getLoc())
					> myc->TURNTOL)) {
		myc->turnaround += situation->deltaTime;
	} else
		myc->turnaround = 0.0;
	if ((myc->turnaround >= waitToTurn) || (myc->tr_mode >= 1)) {
		if (myc->tr_mode == 0) {
			myc->tr_mode = 1;
		}
		if ((car->_gearCmd > -1) && (myc->tr_mode < 2)) {
			car->_accelCmd = 0.0;
			if (myc->tr_mode == 1) {
				car->_gearCmd--;
			}
			car->_brakeCmd = 1.0;
		} else {
			myc->tr_mode = 2;
			if (parallel < cos(90.0 * PI / 180.0)
					&& (mpf->dist2D(myc->getCurrentPos(),
							mpf->getPathSeg(myc->getCurrentSegId())->getLoc())
							> myc->TURNTOL)) {
				angle = queryAngleToTrack(car);
				car->_steerCmd = (-angle > 0.0) ? 1.0 : -1.0;
				car->_brakeCmd = 0.0;

				if (myc->accel < 1.0) {
					myc->accel += myc->ACCELINC;
				}
				car->_accelCmd = myc->accel;
				tdble slipspeed = myc->querySlipSpeed(car);
				if (slipspeed < -myc->TCL_SLIP) {
					car->_accelCmd = car->_accelCmd
							- MIN(car->_accelCmd,
									(myc->TCL_SLIP - slipspeed)
											/ myc->TCL_RANGE);
				}
			} else {
				if (myc->getSpeed() < 1.0) {
					myc->turnaround = 0;
					myc->tr_mode = 0;
					myc->loadBehaviour(myc->START);
					myc->startmode = true;
					myc->trtime = 0.0;
				}
				car->_brakeCmd = 1.0;
				car->_steerCmd = 0.0;
				car->_accelCmd = 0.0;
			}
		}
	}

	if (myc->tr_mode == 0)
		car->_steerCmd = steer;
}

/* pitstop callback */
static int pitcmd(int index, tCarElt* car, tSituation *s) {
	MyCar* myc = mycar[index - 1];
	Pathfinder* mpf = myc->getPathfinderPtr();

	car->_pitFuel =
			MAX(
					MIN((car->_remainingLaps+1.0)*myc->fuelperlap - car->_fuel, car->_tank - car->_fuel),
					0.0);
	myc->lastpitfuel = MAX(car->_pitFuel, 0.0);
	car->_pitRepair = car->_dammage;
	mpf->setPitStop(false, myc->getCurrentSegId());
	myc->loadBehaviour(myc->START);
	myc->startmode = true;
	myc->trtime = 0.0;

	return ROB_PIT_IM; /* return immediately */
}

double calculate_CC(bool updown) {

	const double KP = 0.5;
	const double KP_2 = 1.0;
	const double TARGET_DIST = 15;
	double error = car_speed - target_speed_bn;
	double error_2 = 0.0;
	double pid = error * KP;
	/* Adaptive Cruise Control */
	if (*dist_to_ocar <= 200 && *dist_to_ocar > 0) {
		error_2 = TARGET_DIST - *dist_to_ocar;
		pid = pow(2.5, fabs(error_2) * KP_2) / 2;
	}
	if (updown) {
		if (error_2 < 0.0 || error < 0.0) {
			if (error_2 < 0.0 && *speed_ocar * 1.3 > car_speed) {
				double y = (-12.5) * *dist_to_ocar + 1250;
				MAX(y, 0);
				target_speed_bn += *dist_to_ocar * (1 / (y + 1));
				if (target_speed_bn > 13)
					target_speed_bn = 13;
			}
			return MIN(fabs(pid), 1.0);
		}
		return 0;
	} else {
		if (error_2 > 0.0 || error > 0.0) {
			if (error_2 > 0.0) {
				target_speed_bn -= 0.1;
			}
			return MIN(fabs(pid), 1.0);
		}
		return 0;
	}

}
