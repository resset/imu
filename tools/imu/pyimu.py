import time
import sys
from pyftdi import i2c # pylint: disable=import-error


FTDI_URL = "ftdi://ftdi:2232:FTVE5I3T/2"

# GY-87 module
# BMP180 7-bit address: 0x77
# MPU6050 7-bit address: 0x68
MPU6050_ADDR = 0x68

MPU6050_SMPRT_DIV = 0x19
MPU6050_CONFIG = 0x1A
MPU6050_GYRO_CONFIG = 0x1B
MPU6050_ACCEL_CONFIG = 0x1C
MPU6050_FIFO_EN = 0x23
MPU6050_INT_ENABLE = 0x38
MPU6050_SIGNAL_PATH_RESET = 0x68
MPU6050_USER_CTRL = 0x6A
MPU6050_PWR_MGMT_1 = 0x6B
MPU6050_PWR_MGMT_2 = 0x6C
MPU6050_WHO_AM_I = 0x75
MPU6050_WHO_AM_I_IDENTITY = 0x68

MPU6050_ACCEL_XOUT_H = 0x3B
MPU6050_ACCEL_YOUT_H = 0x3D
MPU6050_ACCEL_ZOUT_H = 0x3F
MPU6050_GYRO_XOUT_H = 0x43
MPU6050_GYRO_YOUT_H = 0x45
MPU6050_GYRO_ZOUT_H = 0x47


def mpu_init():
    print("Connecting to MPU6050... ", end="")
    sys.stdout.flush()

    try:
        i2cc = i2c.I2cController()
        i2cc.configure(FTDI_URL)
        bus = i2cc.get_port(MPU6050_ADDR)
    except: # pylint: disable=bare-except
        print("FAILED.")
        return None

    print("done.")
    print("Resetting and configuring MPU6050... ", end="")
    sys.stdout.flush()

    # Reset MPU6050. Note: apparently this is needed for SPI
    # connection in MPU6000 only, but it shouldn't hurt on I2C either.
    bus.write_to(MPU6050_PWR_MGMT_1, b"\x80")
    time.sleep(0.1)
    # Here we check if the reset is done.
    while True:
        rxbuf = bus.read_from(MPU6050_PWR_MGMT_1, 1)
        if rxbuf[0] & 0x80 == 0:
            break
    # Last step is to reset analog to digital paths of all sensors.
    bus.write_to(MPU6050_SIGNAL_PATH_RESET, b"\x07")
    time.sleep(0.1)

    # Disable sleep mode and set clock source to gyro X.
    bus.write_to(MPU6050_PWR_MGMT_1, b"\x01")

    # Disable standby modes.
    bus.write_to(MPU6050_PWR_MGMT_2, b"\x00")

    # Disable interrupts
    bus.write_to(MPU6050_INT_ENABLE, b"\x00")

    # Disable FIFO
    bus.write_to(MPU6050_FIFO_EN, b"\x00")
    bus.write_to(MPU6050_USER_CTRL, b"\x00")

    # Set gyroscope sensitivity to +/- 1000 deg/s.
    bus.write_to(MPU6050_GYRO_CONFIG, b"\x10")

    # Set accelerometer sensitivity to +/- 8 g.
    bus.write_to(MPU6050_ACCEL_CONFIG, b"\x10")

    # Set low pass filter cutoff frequency (DLPF_CFG). We set 42 Hz.
    # NOTE: it is preferable not to use MPU's filter. External software
    # filter (eg. biquad) in embedded environment will have better performance.
    bus.write_to(MPU6050_CONFIG, b"\x03")

    # Set data rate (if DLPF_CFG == 0 then 8 kHz is divided, otherwise 1 kHz).
    # Since we use LPF, our data rate is 1 kHz.
    bus.write_to(MPU6050_SMPRT_DIV, b"\x00")

    # Test for MPU6050.
    rxbuf = bus.read_from(MPU6050_WHO_AM_I, 1)

    # This should be a check of registers written.
    if rxbuf[0] == MPU6050_WHO_AM_I_IDENTITY:
        print("done.")
        return bus

    print("FAILED.")
    return None


def mpu_read(bus, address):
    return bus.read_from(address, 1)[0]


def mpu_read_data(bus):
    data = bus.read_from(MPU6050_ACCEL_XOUT_H, 14)

    values = {
        "accel_xout": data[0] << 8 | data[1],
        "accel_yout": data[2] << 8 | data[3],
        "accel_zout": data[4] << 8 | data[5],
        "temperature": 0,
        "gyro_xout": data[8] << 8 | data[9],
        "gyro_yout": data[10] << 8 | data[11],
        "gyro_zout": data[12] << 8 | data[13],
    }

    for key, val in values.items():
        if val > 32768:
            values[key] = val - 65536

    values["temperature"] = (data[6] << 8 | data[7]) / 3400 + 3.653

    return values


def mpu_calibrate_zero(bus):
    accel_x_sum = 0
    accel_y_sum = 0
    accel_z_sum = 0
    gyro_x_sum = 0
    gyro_y_sum = 0
    gyro_z_sum = 0

    print("Gathering calibration data... ", end="")
    sys.stdout.flush()

    for i in range(0, 100):
        data = mpu_read_data(bus)
        accel_x_sum += data["accel_xout"]
        accel_y_sum += data["accel_yout"]
        accel_z_sum += data["accel_zout"]
        gyro_x_sum += data["gyro_xout"]
        gyro_y_sum += data["gyro_yout"]
        gyro_z_sum += data["gyro_zout"]

    print("done.")

    return {
        "accel_x_offset": accel_x_sum / 100,
        "accel_y_offset": accel_y_sum / 100,
        "accel_z_offset": accel_z_sum / 100,
        "gyro_x_offset": gyro_x_sum / 100,
        "gyro_y_offset": gyro_y_sum / 100,
        "gyro_z_offset": gyro_z_sum / 100,
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
            print("[t: %f dt: %f]" % (current_time, d_t), end="")

            # Read accelerometer and gyroscope data. 50 Hz should be achievable
            data = mpu_read_data(bus_handler)
            print(
                "%6d %6d %6d %6d %6d %6d t: %f"
                % (
                    data["accel_xout"] - calibration_data["accel_x_offset"],
                    data["accel_yout"] - calibration_data["accel_y_offset"],
                    data["accel_zout"],
                    data["gyro_xout"] - calibration_data["gyro_x_offset"],
                    data["gyro_yout"] - calibration_data["gyro_y_offset"],
                    data["gyro_zout"] - calibration_data["gyro_z_offset"],
                    data["temperature"],
                )
            )


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
      print('')
