import time
import math
import statistics
import pandas as pd
import matplotlib.pyplot as plt
from collections import deque
from pyftdi import i2c


class BMP280:

    # Register addresses.
    _REG_DIG_T1_LSB = 0x88
    _REG_DIG_T1_MSB = 0x89
    _REG_DIG_T2_LSB = 0x8A
    _REG_DIG_T2_MSB = 0x8B
    _REG_DIG_T3_LSB = 0x8C
    _REG_DIG_T3_MSB = 0x8D
    _REG_DIG_P1_LSB = 0x8E
    _REG_DIG_P1_MSB = 0x8F
    _REG_DIG_P2_LSB = 0x90
    _REG_DIG_P2_MSB = 0x91
    _REG_DIG_P3_LSB = 0x92
    _REG_DIG_P3_MSB = 0x93
    _REG_DIG_P4_LSB = 0x94
    _REG_DIG_P4_MSB = 0x95
    _REG_DIG_P5_LSB = 0x96
    _REG_DIG_P5_MSB = 0x97
    _REG_DIG_P6_LSB = 0x98
    _REG_DIG_P6_MSB = 0x99
    _REG_DIG_P7_LSB = 0x9A
    _REG_DIG_P7_MSB = 0x9B
    _REG_DIG_P8_LSB = 0x9C
    _REG_DIG_P8_MSB = 0x9D
    _REG_DIG_P9_LSB = 0x9E
    _REG_DIG_P9_MSB = 0x9F
    _REG_ID         = 0xD0
    _REG_RESET      = 0xE0
    _REG_STATUS     = 0xF3
    _REG_CTRL_MEAS  = 0xF4
    _REG_CONFIG     = 0xF5
    _REG_PRESS_MSB  = 0xF7
    _REG_PRESS_LSB  = 0xF8
    _REG_PRESS_XLSB = 0xF9
    _REG_TEMP_MSB   = 0xFA
    _REG_TEMP_LSB   = 0xFB
    _REG_TEMP_XLSB  = 0xFC

    # Register values.
    _VAL_ID    = 0x58
    _VAL_RESET = 0xB6

    # Other constants.
    _ADDR_LOW       = 0x76  # If pin SDO is tied to GND then LSB is 0.
    _ADDR_HIGH      = 0x77  # If pin SDO is tied to VCC then LSB is 1.
    _ADC_T_MIN      = 0x00000
    _ADC_T_MAX      = 0xFFFF0
    _ADC_P_MIN      = 0x00000
    _ADC_P_MAX      = 0xFFFF0
    _MIN_TEMP_INT   = -4000
    _MAX_TEMP_INT   = 8500
    _MIN_PRES_64INT = 30000 * 256
    _MAX_PRES_64INT = 110000 * 256

    def __init__(self, ftdi_url='ftdi://ftdi:2232:FTVE5I3T/2'):
        self.ftdi_url = ftdi_url
        self.bus = None
        self.zeroed = False
        self.accel_x_offset = None
        self.accel_y_offset = None
        self.accel_z_offset = None
        self.gyro_x_offset = None
        self.gyro_y_offset = None
        self.gyro_z_offset = None

        self.cal_data = {}
        self.alt_started = False
        self.p_ref = None

        i2ccc = i2c.I2cController()
        i2ccc.configure(self.ftdi_url)

        # This will throw UsbToolsError exception when there is no FTDI device.
        self.bus = i2ccc.get_port(BMP280._ADDR_LOW)

        # Reset chip.
        self.bus.write([BMP280._REG_RESET, BMP280._VAL_RESET])
        time.sleep(0.1)

        # Test for BMP280.

        rxbuf = self.bus.exchange([BMP280._REG_ID], 1)
        # print(hex(rxbuf[0]), hex(BMP280._VAL_ID))
        if rxbuf[0] != BMP280._VAL_ID:
            return

        # config register setup.
        # t_sb = 000 (0.5 ms)
        # filter = 111 (x16) (0 = OFF, 1 = x2, ... >=4 = x16)
        # Bit 1 cannot be written, hence this read before the write.
        rxbuf = self.bus.exchange([BMP280._REG_CONFIG], 1)
        self.bus.write([BMP280._REG_CONFIG, 0x10 | (rxbuf[0] & 0x03)])

        # ctrl_meas register setup.
        # osrs_t = 010 (x2, 17-bit, 0.0025 deg. C)
        # osrs_p = 101 (x16, 20-bit, 0.16 Pa)
        # mode = 11 (normal)
        # With settings like this and IIR filter coefficient equaling 16,
        # output data rate is 26.3 Hz.
        self.bus.write([BMP280._REG_CTRL_MEAS, 0x57])

        # Calibration data readout.
        rxbuf = self.bus.exchange([BMP280._REG_DIG_T1_LSB], 24)
        self.cal_data['dig_T1'] = int.from_bytes(rxbuf[0:2], 'little', signed=False)
        self.cal_data['dig_T2'] = int.from_bytes(rxbuf[2:4], 'little', signed=True)
        self.cal_data['dig_T3'] = int.from_bytes(rxbuf[4:6], 'little', signed=True)
        self.cal_data['dig_P1'] = int.from_bytes(rxbuf[6:8], 'little', signed=False)
        self.cal_data['dig_P2'] = int.from_bytes(rxbuf[8:10], 'little', signed=True)
        self.cal_data['dig_P3'] = int.from_bytes(rxbuf[10:12], 'little', signed=True)
        self.cal_data['dig_P4'] = int.from_bytes(rxbuf[12:14], 'little', signed=True)
        self.cal_data['dig_P5'] = int.from_bytes(rxbuf[14:16], 'little', signed=True)
        self.cal_data['dig_P6'] = int.from_bytes(rxbuf[16:18], 'little', signed=True)
        self.cal_data['dig_P7'] = int.from_bytes(rxbuf[18:20], 'little', signed=True)
        self.cal_data['dig_P8'] = int.from_bytes(rxbuf[20:22], 'little', signed=True)
        self.cal_data['dig_P9'] = int.from_bytes(rxbuf[22:24], 'little', signed=True)

    def calibrate_zero(self, diff_from_mean):
        buff = deque(maxlen=100)
        temp, press = self.read_data()  # Discard first readout.
        time.sleep(0.1)
        while True:
            temp, press = self.read_data()
            buff.append(press)
            mean = statistics.mean(buff)
            if len(buff) == 100 and mean > press - diff_from_mean:
                self.p_ref = mean
                return

    def read_data(self):
        rxbuf = self.bus.exchange([BMP280._REG_PRESS_MSB], 6)

        p_adc = rxbuf[0] << 12 | rxbuf[1] << 4 | rxbuf[2] >> 4
        t_adc = rxbuf[3] << 12 | rxbuf[4] << 4 | rxbuf[5] >> 4

        if (p_adc < BMP280._ADC_P_MIN or p_adc > BMP280._ADC_P_MAX
                or t_adc < BMP280._ADC_T_MIN or t_adc > BMP280._ADC_T_MAX):
            return None, None

        var1 = (t_adc / 16384.0 - self.cal_data['dig_T1'] / 1024.0) * self.cal_data['dig_T2']
        var2 = t_adc / 131072.0 - self.cal_data['dig_T1'] / 8192.0
        var2 = var2 * var2 * self.cal_data['dig_T3']
        t_fine = (var1 + var2)
        temperature = t_fine / 5120.0

        var1 = t_fine / 2.0 - 64000.0
        var2 = var1 * var1 * self.cal_data['dig_P6'] / 32768.0
        var2 = var2 + var1 * self.cal_data['dig_P5'] * 2
        var2 = var2 / 4.0 + self.cal_data['dig_P4'] * 65536.0
        var1 = (self.cal_data['dig_P3'] * var1 * var1 / 524288.0 + self.cal_data['dig_P2'] * var1) / 524288.0
        var1 = (1.0 + var1 / 32768.0) * self.cal_data['dig_P1']
        pressure = 1048576.0 - p_adc
        pressure = (pressure - var2 / 4096.0) * 6250.0 / var1
        var1 = self.cal_data['dig_P9'] * pressure * pressure / 2147483648.0
        var2 = pressure * self.cal_data['dig_P8'] / 32768.0
        pressure = pressure + (var1 + var2 + self.cal_data['dig_P7']) / 16.0

        return temperature, pressure

    def altitude(self, temperature, pressure):
        # This is the barometric formula. The same as the famous:
        # H = 44330 * (1 - (P / P0) ^ (1 / 5.255))
        # 44330 is calculated by assuming constant temperature of 15 deg.
        # Celsius. Our formula is temperature-compensated.
        return ((math.pow(pressure / self.p_ref, 0.1902665) - 1)
                        * (temperature + 273.15)) / -0.0065


class Estimator:
    def __init__(self, error_estimate, error_measurement, initial_estimate):
        self._prev_error_estimate = error_estimate
        self._error_meas = error_measurement
        self._prev_estimate = initial_estimate

    def estimate(self, measurement):
        k_gain = self._prev_error_estimate / (self._prev_error_estimate + self._error_meas)
        current_estimate = self._prev_estimate + k_gain * (measurement - self._prev_estimate)
        self._prev_error_estimate = (1 - k_gain) * self._prev_error_estimate + abs(self._prev_estimate - current_estimate) * 0.9
        # Process noise?
        # https://www.rocketryforum.com/threads/arduino-altimeter-bmp280-inbuilt-noise-filtering-settings.151941/#post-1961277
        # + abs(self._prev_estimate - current_estimate) * 0.01
        self._prev_estimate = current_estimate
        return current_estimate


class SimpleFilter:
    def __init__(self):
        self.buff = deque(maxlen=10)

    def filter(self, measurement):
        self.buff.append(measurement)
        return statistics.mean(self.buff)


try:
    b = BMP280()
    b.calibrate_zero(0.48)

    est = Estimator(1.0, 1.0, 0.0)
    flt = SimpleFilter()

    altitude_raw = []
    altitude_filtered = []
    while True:
        temp, press = b.read_data()
        alt = b.altitude(temp, press)
        if alt:
            alt_f = flt.filter(alt)
            alt_f = est.estimate(alt)
            print(f'temperature: {temp:.2f} [deg. C]')
            print(f'pressure:    {press:.2f} [Pa]')
            print(f'altitude:    {alt * 100.0:.2f} {alt_f * 100:.2f} [cm]')
            altitude_raw.append(alt)
            altitude_filtered.append(alt_f)
        time.sleep(0.04)
except KeyboardInterrupt:
    df = pd.DataFrame({'raw': altitude_raw, 'filtered': altitude_filtered})
    print(df.to_json())
    df.plot(kind='line')
    plt.show()
