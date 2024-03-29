# Datasheet test data
dig_T1 = 27504
dig_T2 = 26435
dig_T3 = -1000
dig_P1 = 36477
dig_P2 = -10685
dig_P3 = 3024
dig_P4 = 2855
dig_P5 = 140
dig_P6 = -7
dig_P7 = 15500
dig_P8 = -14600
dig_P9 = 6000
t_adc = 415148
p_adc = 519888

# My test - sensor #1
dig_T1 = 27377
dig_T2 = 25596
dig_T3 = 50
dig_P1 = 37748
dig_P2 = -10650
dig_P3 = 3024
dig_P4 = 7972
dig_P5 = -212
dig_P6 = -7
dig_P7 = 15500
dig_P8 = -14600
dig_P9 = 6000
t_adc = 533744
p_adc = 325383

# My test - sensor #2
# dig_T1 = 27782
# dig_T2 = 25943
# dig_T3 = 50
# dig_P1 = 38273
# dig_P2 = -10526
# dig_P3 = 3024
# dig_P4 = 7253
# dig_P5 = -217
# dig_P6 = -7
# dig_P7 = 15500
# dig_P8 = -14600
# dig_P9 = 6000
# t_adc = 538600
# p_adc = 328749

# Pimoroni test data (no readouts)
# dig_T1 = 28009
# dig_T2 = 25654
# dig_T3 = 50
# dig_P1 = 39145
# dig_P2 = -10750
# dig_P3 = 3024
# dig_P4 = 5667
# dig_P5 = -120
# dig_P6 = -7
# dig_P7 = 15500
# dig_P8 = -14600
# dig_P9 = 6000

print('### FP ###')

var1 = (t_adc / 16384.0 - dig_T1 / 1024.0) * dig_T2
var2 = t_adc / 131072.0 - dig_T1 / 8192.0
var2 = var2 * var2 * dig_T3
t_fine = (var1 + var2)
temperature = t_fine / 5120.0
print('temperature:', temperature, '[deg. C]')

var1 = t_fine / 2.0 - 64000.0
var2 = var1 * var1 * dig_P6 / 32768.0
var2 = var2 + var1 * dig_P5 * 2
var2 = var2 / 4.0 + dig_P4 * 65536.0
var1 = (dig_P3 * var1 * var1 / 524288.0 + dig_P2 * var1) / 524288.0
var1 = (1.0 + var1 / 32768.0) * dig_P1
pressure = 1048576.0 - p_adc
pressure = (pressure - var2 / 4096.0) * 6250.0 / var1
var1 = dig_P9 * pressure * pressure / 2147483648.0
var2 = pressure * dig_P8 / 32768.0
pressure = pressure + (var1 + var2 + dig_P7) / 16.0

print('pressure:   ', pressure, '[Pa]')
print()

print('### INT ###')

var1 = ((((t_adc >> 3) - (dig_T1 << 1))) * (dig_T2)) >> 11
var2 = (((((t_adc >> 4) - (dig_T1)) * ((t_adc >> 4) - (dig_T1))) >> 12) * (dig_T3)) >> 14
t_fine = (var1 + var2)
temperature = (t_fine * 5 + 128) >> 8

print('temperature:', temperature, '[deg. C * 10]')

var1 = (t_fine) - 128000
var2 = var1 * var1 * dig_P6
var2 = var2 + ((var1 * dig_P5) << 17)
var2 = var2 + ((dig_P4) << 35)
var1 = ((var1 * var1 * dig_P3) >> 8) + ((var1 * dig_P2) << 12)
var1 = ((((1) << 47) + var1)) * (dig_P1) >> 33
if var1 == 0:
    print('ERROR')
pressure = 1048576 - p_adc
 # Force integer calculation
pressure = int((((pressure << 31) - var2) * 3125) / var1)
var1 = ((dig_P9) * (pressure >> 13) * (pressure >> 13)) >> 25
var2 = ((dig_P8) * pressure) >> 19
pressure = ((pressure + var1 + var2) >> 8) + ((dig_P7) << 4)

print('pressure:   ', pressure, '[Pa * 256]')
print('pressure:   ', int(pressure / 256), '[Pa]')
