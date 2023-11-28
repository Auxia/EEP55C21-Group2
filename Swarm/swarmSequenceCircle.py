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
A script to fly 5 Crazyflies in formation. One stays in the center and the
other four fly around it in a circle. Mainly intended to be used with the
Flow deck.
The starting positions are vital and should be oriented like this

     >

^    +    v

     <

The distance from the center to the perimeter of the circle is around 0.5 m

"""
import math
import time

import cflib.crtp
from cflib.crazyflie.swarm import CachedCfFactory
from cflib.crazyflie.swarm import Swarm

# Change uris according to your setup
URI0 = 'radio://0/80/2M/EE5C21CFB1'
URI1 = 'radio://0/80/2M/EE5C21CFC1'

# d: diameter of circle
# z: altitude
params0 = {'d': 0.4, 'z': 0.6, 'startBottom': True}
params1 = {'d': 0.4, 'z': 0.6+0.3, 'startBottom': False}
largeD = 0.7


# uris = {
#     URI0,
#     URI1
# }

uris = {
    # 'radio://0/57/2M/EE5C21CFA1',
    'radio://0/80/2M/EE5C21CFB1',
    'radio://0/80/2M/EE5C21CFC1',
    # 'radio://0/22/2M/EE5C21CFC0',
    # Add more URIs if you want more copters in the swarm
}


params = {
    URI0: [params0],
    URI1: [params1],
}


def poshold(cf, t, z):
    steps = t * 10

    for r in range(steps):
        cf.commander.send_hover_setpoint(0, 0, 0, z)
        time.sleep(0.1)


def run_sequence(scf, params):
    cf = scf.cf

    # Number of setpoints sent per second
    fs = 10
    fsi = 1.0 / fs

    # Base altitude in meters
    base = 0.2

    corr = 8

    d = params['d']
    z = params['z']

    # go up

    poshold(cf, 2, base)

    ramp = fs * 2
    for r in range(ramp):
        cf.commander.send_hover_setpoint(0, 0, 0, base + r * (z - base) / ramp)
        time.sleep(fsi)

    poshold(cf, 2, z)

    # revolution in the xy plane

    # for _ in range(2):
    #     # The time for one revolution
    #     circle_time = 8

    #     steps = circle_time * fs
    #     for _ in range(steps):
    #         cf.commander.send_hover_setpoint(d * comp * math.pi / circle_time,
    #                                          0, 360.0 / circle_time, z)
    #         time.sleep(fsi)

    circle_time = 6
    large_circle_number = 4
    steps = circle_time*fs
    for j in range(large_circle_number):
        for i in range(steps):
            if params['startBottom']:
                print(i)
                cf.commander.send_hover_setpoint(
                    largeD*math.pi/(circle_time*large_circle_number), corr*(2*math.pi/steps)*(d/2)*math.cos(2*math.pi*i/steps), 360.0/(circle_time*large_circle_number), z+(d/2)*(1-math.cos(2*math.pi*i/steps)))
            else:
                cf.commander.send_hover_setpoint(
                    largeD*math.pi/(circle_time*large_circle_number), -corr*(2*math.pi/steps)*(d/2)*math.cos(2*math.pi*i/steps), 360.0/(circle_time*large_circle_number), z+(d/2)*(1+math.cos(2*math.pi*i/steps)))

            time.sleep(fsi)

    if params['startBottom']:
        poshold(cf, 2, z)
    else:
        poshold(cf, 2, z+d)

    # go back to base

    for r in range(ramp):
        if params['startBottom']:
            cf.commander.send_hover_setpoint(0, 0, 0,
                                             base + (ramp - r) * (z - base) / ramp)
        else:
            cf.commander.send_hover_setpoint(0, 0, 0,
                                             base + (ramp - r) * (z+d - base) / ramp)

        time.sleep(fsi)

    poshold(cf, 1, base)

    cf.commander.send_stop_setpoint()
    # Hand control over to the high level commander to avoid timeout and locking of the Crazyflie
    cf.commander.send_notify_setpoint_stop()


if __name__ == '__main__':
    cflib.crtp.init_drivers()

    factory = CachedCfFactory(rw_cache='./cache')
    with Swarm(uris, factory=factory) as swarm:
        swarm.reset_estimators()
        swarm.parallel(run_sequence, args_dict=params)
