// -*- Mode: c++ -*-
/***************************************************************************

    file                 : opponent.h
    created              : Thu Apr 22 01:20:19 CET 2003
    copyright            : (C) 2003-2005 Bernhard Wymann, Christos Dimitrakakis
    email                : berniw@bluewin.ch
    version              : $Id: opponent.h,v 1.7.2.2 2011/12/31 02:51:47 berniw Exp $

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _OPPONENT_H_
#define _OPPONENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>

#include "linalg.h"
#include "driver.h"
#include "cardata.h"

#define OPP_IGNORE		0
#define OPP_FRONT		(1<<0)
#define OPP_BACK		(1<<1)
#define OPP_SIDE		(1<<2)
#define OPP_COLL		(1<<3)
#define OPP_LETPASS		(1<<4)
#define OPP_FRONT_FAST	(1<<5)

#ifdef USE_OLETHROS_NAMESPACE
namespace olethros
{
#endif
	class Driver;

	// Opponent maintains the data for one opponent RELATIVE to the drivers car.
	class Opponent {
	public:
		Opponent();

		void setCarPtr(tCarElt *car) { this->car = car; }
		void setCarDataPtr(SingleCardata *cardata) { this->cardata = cardata; }
		static void setTrackPtr(tTrack *track) { Opponent::track = track; }

		tCarElt *getCarPtr() { return car; }
		int getState() { return state; }
		float getCatchDist() { return catchdist; }
		float getDistance() { return distance; }
		float getSideDist() { return sidedist; }
		float getWidth() { return cardata->getWidthOnTrack(); }
		float getSpeed() { return cardata->getSpeedInTrackDirection(); }
		float getOverlapTimer() { return overlaptimer; }
		void update(tSituation *s, Driver *driver);
		float getBrakeOvertake () { return brake_overtake_filter; }
		void brakeForCollision () { brake_overtake_filter = 1.0; }

	private:
		float getDistToSegStart();
		void updateOverlapTimer(tSituation *s, tCarElt *mycar);

		float distance;		///< approximation of the real distance, negative if the opponent is behind.
		float catchdist;	///< distance needed to catch the opponent (linear estimate).
		float sidedist;		///< approx distance of center of gravity of the cars.
		int state;			///< State variable to characterize the relation to the opponent, e. g. opponent is behind.
		float overlaptimer;
		float brake_overtake_filter; ///< Check to see.

		tCarElt *car;
		SingleCardata *cardata;		// Pointer to global data about this opponent.

		// class variables.
		static tTrack *track;

		// constants.
		static const float FRONTCOLLDIST;
		static const float BACKCOLLDIST;
		static const float LENGTH_MARGIN;
		static const float SIDE_MARGIN;
		static const float EXACT_DIST;
		static const float LAP_BACK_TIME_PENALTY;
		static const float OVERLAP_WAIT_TIME;
		static const float SPEED_PASS_MARGIN;
		static const float TIME_MARGIN;
	};


	// The Opponents class holds an array of all Opponents.
	class Opponents {
	public:
		Opponents(tSituation *s, Driver *driver, Cardata *cardata);
		~Opponents();

		void update(tSituation *s, Driver *driver);
		Opponent *getOpponentPtr() { return opponent; }
		/// Return total number of opponents
		int getNOpponents() { return nopponents; }
		/// Return total number of \em racing opponents behind.
		int getNOpponentsBehind() { return nopponents_behind; }
		/// Return total number of \em racing opponents in front.
		int getNOpponentsInFront() { return nopponents_infront;}


	private:
		Opponent *opponent;
		int nopponents;
		int nopponents_behind;
		int nopponents_infront;
	};
#ifdef USE_OLETHROS_NAMESPACE
}
#endif

#endif // _OPPONENT_H_
