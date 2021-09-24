import time
from pyftdi import spi  # pylint: disable=import-error


FTDI_URL = 'ftdi://ftdi:2232:FTVE5I3T/1'


class Icm:
    _ICM20689_SMPLRT_DIV = 0x19
    _ICM20689_CONFIG = 0x1A
    _ICM20689_GYRO_CONFIG = 0x1B
    _ICM20689_ACCEL_CONFIG = 0x1C
    _ICM20689_ACCEL_CONFIG2 = 0x1D
    _ICM20689_FIFO_EN = 0x23
    _ICM20689_INT_ENABLE = 0x38
    _ICM20689_ACCEL_XOUT_H = 0x3B
    _ICM20689_ACCEL_XOUT_L = 0x3C
    _ICM20689_ACCEL_YOUT_H = 0x3D
    _ICM20689_ACCEL_YOUT_L = 0x3E
    _ICM20689_ACCEL_ZOUT_H = 0x3F
    _ICM20689_ACCEL_ZOUT_L = 0x40
    _ICM20689_TEMP_OUT_H = 0x41
    _ICM20689_TEMP_OUT_L = 0x42
    _ICM20689_GYRO_XOUT_H = 0x43
    _ICM20689_GYRO_XOUT_L = 0x44
    _ICM20689_GYRO_YOUT_H = 0x45
    _ICM20689_GYRO_YOUT_L = 0x46
    _ICM20689_GYRO_ZOUT_H = 0x47
    _ICM20689_GYRO_ZOUT_L = 0x48
    _ICM20689_SIGNAL_PATH_RESET = 0x68
    _ICM20689_USER_CTRL = 0x6A
    _ICM20689_PWR_MGMT_1 = 0x6B
    _ICM20689_PWR_MGMT_2 = 0x6C
    _ICM20689_WHO_AM_I = 0x75

    _ICM20689_VAL_WHO_AM_I = 0x98

    _ICM20689_READ_MASK = 0x80
    _ICM20689_WRITE_MASK = 0x00

    def __init__(self, ftdi_url='ftdi://ftdi:2232:FTVE5I3T/1'):
        self.ftdi_url = ftdi_url
        self.bus = None
        self.zeroed = False
        self.accel_x_offset = None
        self.accel_y_offset = None
        self.accel_z_offset = None
        self.gyro_x_offset = None
        self.gyro_y_offset = None
        self.gyro_z_offset = None

        spicc = spi.SpiController()
        spicc.configure(self.ftdi_url)
        # This will throw UsbToolsError exception when there is no FTDI device.
        self.bus = spicc.get_port(cs=0, freq=5E5, mode=0)

        # Reset ICM20689. Note: apparently this is needed for SPI
        # connection in MPU6000 only, but it shouldn't hurt on I2C either.
        self.bus.write([self._ICM20689_PWR_MGMT_1, 0x80])
        time.sleep(0.1)
        # Here we check if the reset is done.
        while True:
            rxbuf = self.bus.exchange(
                [self._ICM20689_PWR_MGMT_1 | self._ICM20689_READ_MASK], 1)
            if rxbuf[0] & 0x80 == 0:
                break
        # Last step is to reset analog to digital paths of all sensors.
        self.bus.write([self._ICM20689_SIGNAL_PATH_RESET, 0x07])
        time.sleep(0.1)

        # Disable I2C interface and FIFO.
        self.bus.write([self._ICM20689_USER_CTRL, 0x10])
        self.bus.write([self._ICM20689_FIFO_EN, 0x00])

        # Disable sleep mode and set clock source to gyro X.
        self.bus.write([self._ICM20689_PWR_MGMT_1, 0x01])

        # Disable standby modes.
        self.bus.write([self._ICM20689_PWR_MGMT_2, 0x00])

        # Disable interrupts.
        self.bus.write([self._ICM20689_INT_ENABLE, 0x00])

        # Set gyroscope sensitivity to +/- 1000 deg/s.
        self.bus.write([self._ICM20689_GYRO_CONFIG, 0x10])

        # Set accelerometer sensitivity to +/- 8 g.
        self.bus.write([self._ICM20689_ACCEL_CONFIG, 0x10])

        # Set low pass filter cutoff frequency (DLPF_CFG). We set 41 Hz for gyro.
        # NOTE: it is preferable not to use MPU's filter. External software
        # filter (eg. biquad) in embedded environment will have better performance.
        self.bus.write([self._ICM20689_CONFIG, 0x03])

        # Set data rate (if DLPF_CFG == 0 then 8 kHz is divided, otherwise 1 kHz).
        # Since we use LPF, our data rate is 1 kHz.
        self.bus.write([self._ICM20689_SMPLRT_DIV, 0x00])

        # Test for ICM20689.
        rxbuf = self.bus.exchange(
            [self._ICM20689_WHO_AM_I | self._ICM20689_READ_MASK], 1)

        # This should be a check of registers written.
        if rxbuf[0] != self._ICM20689_VAL_WHO_AM_I:
            raise Exception('ICM20689: identification register value does not match.')

    def calibrate_zero(self, samples_number=100):
        accel_x_sum = 0
        accel_y_sum = 0
        accel_z_sum = 0
        gyro_x_sum = 0
        gyro_y_sum = 0
        gyro_z_sum = 0

        self.zeroed = False
        for _ in range(0, samples_number):
            data = self.read_data()
            accel_x_sum += data['accel_xout']
            accel_y_sum += data['accel_yout']
            accel_z_sum += data['accel_zout']
            gyro_x_sum += data['gyro_xout']
            gyro_y_sum += data['gyro_yout']
            gyro_z_sum += data['gyro_zout']

        self.accel_x_offset = accel_x_sum / 100
        self.accel_y_offset = accel_y_sum / 100
        self.accel_z_offset = accel_z_sum / 100
        self.gyro_x_offset = gyro_x_sum / 100
        self.gyro_y_offset = gyro_y_sum / 100
        self.gyro_z_offset = gyro_z_sum / 100

        self.zeroed = True

    def read_data(self):
        data = self.bus.exchange(
            [self._ICM20689_ACCEL_XOUT_H | self._ICM20689_READ_MASK], 14)

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

        if self.zeroed:
            values['accel_xout'] = values['accel_xout'] - self.accel_x_offset
            values['accel_yout'] = values['accel_yout'] - self.accel_y_offset
            values['accel_zout'] = values['accel_zout'] - self.accel_z_offset
            values['gyro_xout'] = values['gyro_xout'] - self.gyro_x_offset
            values['gyro_yout'] = values['gyro_yout'] - self.gyro_y_offset
            values['gyro_zout'] = values['gyro_zout'] - self.gyro_z_offset

        values['temperature'] = ((data[6] << 8 | data[7]) - 0) / 326.8 + 25.0

        return values
