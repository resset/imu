import time
import sys
import icm


FTDI_URL = 'ftdi://ftdi:2232:FTVE5I3T/1'


def main():
    imu = icm.Icm(FTDI_URL)
    imu.calibrate_zero(10000)

    previous_time = time.time()
    while True:
        # Calculate dt
        current_time = time.time()
        d_t = current_time - previous_time
        previous_time = current_time
        print(f'[t: {current_time:f} dt: {d_t:f} f: {1 / d_t:4.6f}]', end='')

        # Read accelerometer and gyroscope data. More than 2 kHz should
        # be achievable with 500 kHz clock.
        data = imu.read_data()

        print(
           f"  a({data['accel_xout']:6d} {data['accel_yout']:6d} "
           f"{data['accel_zout']:6d})  g({data['gyro_xout']:6d} "
           f"{data['gyro_yout']:6d} {data['gyro_zout']:6d})  "
           f"t: {data['temperature']:f}"
        )


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('')
    except BrokenPipeError:
        print('')
