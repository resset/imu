import time
import sys
from pyftdi import spi # pylint: disable=import-error


FTDI_URL = 'ftdi://ftdi:2232:FTVE5I3T/1'

# IMU-6-click module
ICM20689_SMPLRT_DIV = 0x19
ICM20689_CONFIG = 0x1A
ICM20689_GYRO_CONFIG = 0x1B
ICM20689_ACCEL_CONFIG = 0x1C
ICM20689_ACCEL_CONFIG2 = 0x1D
ICM20689_FIFO_EN = 0x23
ICM20689_INT_ENABLE = 0x38
ICM20689_ACCEL_XOUT_H = 0x3B
ICM20689_ACCEL_XOUT_L = 0x3C
ICM20689_ACCEL_YOUT_H = 0x3D
ICM20689_ACCEL_YOUT_L = 0x3E
ICM20689_ACCEL_ZOUT_H = 0x3F
ICM20689_ACCEL_ZOUT_L = 0x40
ICM20689_TEMP_OUT_H = 0x41
ICM20689_TEMP_OUT_L = 0x42
ICM20689_GYRO_XOUT_H = 0x43
ICM20689_GYRO_XOUT_L = 0x44
ICM20689_GYRO_YOUT_H = 0x45
ICM20689_GYRO_YOUT_L = 0x46
ICM20689_GYRO_ZOUT_H = 0x47
ICM20689_GYRO_ZOUT_L = 0x48
ICM20689_SIGNAL_PATH_RESET = 0x68
ICM20689_USER_CTRL = 0x6A
ICM20689_PWR_MGMT_1 = 0x6B
ICM20689_PWR_MGMT_2 = 0x6C
ICM20689_WHO_AM_I = 0x75

ICM20689_VAL_WHO_AM_I = 0x98

ICM20689_READ_MASK  = 0x80
ICM20689_WRITE_MASK = 0x00


def mpu_init():
    print('Connecting to ICM20689... ', end='')
    sys.stdout.flush()

    try:
        spicc = spi.SpiController()
        spicc.configure(FTDI_URL)
        bus = spicc.get_port(cs=0, freq=5E5, mode=0)
    except: # pylint: disable=bare-except
        print('FAILED.')
        return None

    print('done.')
    print('Resetting and configuring ICM20689... ', end='')
    sys.stdout.flush()

    # Reset ICM20689. Note: apparently this is needed for SPI
    # connection in MPU6000 only, but it shouldn't hurt on I2C either.
    bus.write([ICM20689_PWR_MGMT_1, 0x80])
    time.sleep(0.1)
    # Here we check if the reset is done.
    while True:
        rxbuf = bus.exchange([ICM20689_PWR_MGMT_1 | ICM20689_READ_MASK], 1)
        if rxbuf[0] & 0x80 == 0:
            break
    # Last step is to reset analog to digital paths of all sensors.
    bus.write([ICM20689_SIGNAL_PATH_RESET, 0x07])
    time.sleep(0.1)

    # Disable I2C interface and FIFO.
    bus.write([ICM20689_USER_CTRL, 0x10])
    bus.write([ICM20689_FIFO_EN, 0x00])

    # Disable sleep mode and set clock source to gyro X.
    bus.write([ICM20689_PWR_MGMT_1, 0x01])

    # Disable standby modes.
    bus.write([ICM20689_PWR_MGMT_2, 0x00])

    # Disable interrupts.
    bus.write([ICM20689_INT_ENABLE, 0x00])

    # Set gyroscope sensitivity to +/- 1000 deg/s.
    bus.write([ICM20689_GYRO_CONFIG, 0x10])

    # Set accelerometer sensitivity to +/- 8 g.
    bus.write([ICM20689_ACCEL_CONFIG, 0x10])

    # Set low pass filter cutoff frequency (DLPF_CFG). We set 41 Hz for gyro.
    # NOTE: it is preferable not to use MPU's filter. External software
    # filter (eg. biquad) in embedded environment will have better performance.
    bus.write([ICM20689_CONFIG, 0x03])

    # Set data rate (if DLPF_CFG == 0 then 8 kHz is divided, otherwise 1 kHz).
    # Since we use LPF, our data rate is 1 kHz.
    bus.write([ICM20689_SMPLRT_DIV, 0x00])

    # Test for ICM20689.
    rxbuf = bus.exchange([ICM20689_WHO_AM_I | ICM20689_READ_MASK], 1)

    # This should be a check of registers written.
    if rxbuf[0] == ICM20689_VAL_WHO_AM_I:
        print('done.')
        return bus

    print('FAILED.')
    return None


def mpu_read(bus, address):
    return bus.read_from(address, 1)[0]


def mpu_read_data(bus):
    data = bus.exchange([ICM20689_ACCEL_XOUT_H | 0x80], 14)

    values = {
        'accel_xout': data[0] << 8 | data[1],
        'accel_yout': data[2] << 8 | data[3],
        'accel_zout': data[4] << 8 | data[5],
        'temperature': 0,
        'gyro_xout': data[8] << 8 | data[9],
        'gyro_yout': data[10] << 8 | data[11],
        'gyro_zout': data[12] << 8 | data[13],
    }

    for key, val in values.items():
        if val > 32768:
            values[key] = val - 65536

    values['temperature'] = ((data[6] << 8 | data[7]) - 0) / 326.8 + 25.0

    return values


def mpu_calibrate_zero(bus):
    accel_x_sum = 0
    accel_y_sum = 0
    accel_z_sum = 0
    gyro_x_sum = 0
    gyro_y_sum = 0
    gyro_z_sum = 0

    print('Gathering calibration data... ', end='')
    sys.stdout.flush()

    for _ in range(0, 100):
        data = mpu_read_data(bus)
        accel_x_sum += data['accel_xout']
        accel_y_sum += data['accel_yout']
        accel_z_sum += data['accel_zout']
        gyro_x_sum += data['gyro_xout']
        gyro_y_sum += data['gyro_yout']
        gyro_z_sum += data['gyro_zout']

    print('done.')

    return {
        'accel_x_offset': accel_x_sum / 100,
        'accel_y_offset': accel_y_sum / 100,
        'accel_z_offset': accel_z_sum / 100,
        'gyro_x_offset': gyro_x_sum / 100,
        'gyro_y_offset': gyro_y_sum / 100,
        'gyro_z_offset': gyro_z_sum / 100,
    }


def main():
    bus_handler = mpu_init()

    if bus_handler:
        calibration_data = mpu_calibrate_zero(bus_handler)

        previous_time = time.time()
        while True:
            # Calculate dt
            current_time = time.time()
            d_t = current_time - previous_time
            previous_time = current_time
            print('[t: %f dt: %f]' % (current_time, d_t), end='')

            # Read accelerometer and gyroscope data. More than 2 kHz should
            # be achievable with 500 kHz clock.
            data = mpu_read_data(bus_handler)
            print(
                '%6d %6d %6d %6d %6d %6d t: %f'
                % (
                    data['accel_xout'] - calibration_data['accel_x_offset'],
                    data['accel_yout'] - calibration_data['accel_y_offset'],
                    data['accel_zout'],
                    data['gyro_xout'] - calibration_data['gyro_x_offset'],
                    data['gyro_yout'] - calibration_data['gyro_y_offset'],
                    data['gyro_zout'] - calibration_data['gyro_z_offset'],
                    data['temperature'],
                )
            )


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('')
