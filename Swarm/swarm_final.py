import time

import cflib.crtp
from cflib.crazyflie.swarm import CachedCfFactory
from cflib.crazyflie.swarm import Swarm


def activate_led_bit_mask(scf):
    scf.cf.param.set_value('led.bitmask', 255)


def deactivate_led_bit_mask(scf):
    scf.cf.param.set_value('led.bitmask', 0)


def light_check(scf):
    activate_led_bit_mask(scf)
    time.sleep(2)
    deactivate_led_bit_mask(scf)
    time.sleep(2)


def take_off(scf):
    commander = scf.cf.high_level_commander

    commander.takeoff(1.0, 2.0)
    time.sleep(3)


""" def run_sequence(scf, params):
    commander = scf.cf.high_level_commander
    startTime = time.time()

    for _ in range(100):
        elapsed = time.time() - startTime

        xSetPoint =  """


uris = {
    # 'radio://0/57/2M/EE5C21CFA1',
    'radio://0/80/2M/EE5C21CFB1',
    'radio://0/80/2M/EE5C21CFC1',
    # 'radio://0/20/2M/E7E7E7E702',
    # 'radio://0/20/2M/E7E7E7E703',
    # 'radio://0/20/2M/E7E7E7E704',
    # Add more URIs if you want more copters in the swarm
}


if __name__ == '__main__':
    cflib.crtp.init_drivers()
    factory = CachedCfFactory(rw_cache='./cache')
    with Swarm(uris, factory=factory) as swarm:
        swarm.parallel_safe(light_check)
        swarm.reset_estimators()

        swarm.parallel_safe(run_sequence, params)

        # warm.sequential(hover_sequence)
