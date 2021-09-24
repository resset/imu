import time
import sys
import icm


FTDI_URL = 'ftdi://ftdi:2232:FTVE5I3T/1'


def main():
    imu = icm.Icm(FTDI_URL)
    imu.calibrate_zero(1000)

    previous_time = time.time()
    while True:
        # Calculate dt
        current_time = time.time()
        d_t = current_time - previous_time
        previous_time = current_time
        print('[t: %f dt: %f]' % (current_time, d_t), end='')

        # Read accelerometer and gyroscope data. More than 2 kHz should
        # be achievable with 500 kHz clock.
        data = imu.read_data()
        print(
            '%6d %6d %6d %6d %6d %6d t: %f'
            % (
                data['accel_xout'], data['accel_yout'], data['accel_zout'],
                data['gyro_xout'], data['gyro_yout'], data['gyro_zout'],
                data['temperature'],
            )
        )


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('')
    except BrokenPipeError:
        print('')
