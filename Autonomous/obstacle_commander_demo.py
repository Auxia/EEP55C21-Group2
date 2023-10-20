# -*- coding: utf-8 -*-
#
#     ||          ____  _ __
#  +------+      / __ )(_) /_______________ _____  ___
#  | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
#  +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
#   ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
#
#  Copyright (C) 2017 Bitcraze AB
#
#  Crazyflie Nano Quadcopter Client
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
"""
This script shows the basic use of the MotionCommander class.

Simple example that connects to the crazyflie at `URI` and runs a
sequence. This script requires some kind of location system, it has been
tested with (and designed for) the flow deck.

The MotionCommander uses velocity setpoints.

Change the URI variable to your Crazyflie configuration.
"""
import logging
import time

import cflib.crtp
from cflib.crazyflie import Crazyflie
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie
from cflib.positioning.motion_commander import MotionCommander
from cflib.utils import uri_helper

URI = uri_helper.uri_from_env(default='radio://0/57/2M/EE5C21CFA1')

# Only output errors from the logging framework
logging.basicConfig(level=logging.ERROR)


if __name__ == '__main__':
    # Initialize the low-level drivers
    cflib.crtp.init_drivers()

    with SyncCrazyflie(URI, cf=Crazyflie(rw_cache='./cache')) as scf:
        # We take off when the commander is created
        with MotionCommander(scf) as mc:

            side_length = 0.55  # Adjust this value as needed
            vel = 0.6
            rope_triangle_length = 0.2

            time.sleep(1)

            print("up")

            mc.up(0.5, velocity=vel)

            print("lower circle")
            mc.circle_right(side_length, velocity=vel)

            print("upper circle")
            mc.circle_left(side_length, velocity=vel)

            mc.stop()

            print("rope")
            mc.back(0.4, velocity=vel)

            print("go above rope")

            mc.move_distance(0, -rope_triangle_length,
                             rope_triangle_length, velocity=vel)
            time.sleep(1)

            mc.move_distance(0, -rope_triangle_length, -
                             rope_triangle_length, velocity=vel)
            time.sleep(1)

            # mc.back(0.1, velocity=vel)

            print("end")

            # Stop the motion
            mc.stop()            # We land when the MotionCommander goes out of scope
