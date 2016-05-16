# -*- coding: utf-8 -*-

def bar_calc(c1, c2, c3, c4, c5, c6, d1, d2):
    dt = d2 - (c5 << 8)
    print 'dt = %s' % dt

    temp = 2000 + ((dt * c6) >> 23)
    print 'temp = %s' % temp

    off = (c2 << 16) + ((c4 * dt) >> 7)
    #off = (c2 << 16)
    print 'off = %s' % off

    sens = (c1 << 15) + ((c3 * dt) >> 8)
    #sens = ((c4 * dt) >> 7)
    print 'sens = %s' % sens

    p = (((d1 * sens) >> 21) - off) >> 15
    print 'p = %s' % p

    print ''
    print 'Temperature: %s [°C]' % (temp / 100.0) 
    print 'Pressure: %s [mbar,hPa]' % (p / 100.0) # 1013.25
    print ''

def bar_calc_parse(string):
    lines = string.split("\n")
    for i in range(len(lines)):
        if lines[i][0:2] == 'c1':
          c1 = int(lines[i][4:])
        if lines[i][0:2] == 'c2':
          c2 = int(lines[i][4:])
        if lines[i][0:2] == 'c3':
          c3 = int(lines[i][4:])
        if lines[i][0:2] == 'c4':
          c4 = int(lines[i][4:])
        if lines[i][0:2] == 'c5':
          c5 = int(lines[i][4:])
        if lines[i][0:2] == 'c6':
          c6 = int(lines[i][4:])
        if lines[i][0:2] == 'd1':
          d1 = int(lines[i][4:])
        if lines[i][0:2] == 'd2':
          d2 = int(lines[i][4:])
    bar_calc(c1, c2, c3, c4, c5, c6, d1, d2)
    pass

# Sample data from MS5611 manual.
c1 = 40127
c2 = 36924
c3 = 23317
c4 = 23282
c5 = 33464
c6 = 28312
d1 = 9085466
d2 = 8569150
#bar_calc(c1, c2, c3, c4, c5, c6, d1, d2)

# Sample data from MS5611 manual, here processed in different way.
data = """
c1: 40127
c2: 36924
c3: 23317
c4: 23282
c5: 33464
c6: 28312
d1: 9085466
d2: 8569150
"""
bar_calc_parse(data)
