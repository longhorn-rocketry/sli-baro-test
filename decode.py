import struct
import sys

if len(sys.argv) != 2:
    print("Usage: python decode.py <filename>")
    sys.exit(1)

points = 0

with open(sys.argv[1], "rb") as f:
    f_time = open("time.dat", "w")
    f_pressure = open("pressure.dat", "w")
    f_temp = open("temperature.dat", "w")

    while True:
        try:
            time = struct.unpack('>f', f.read(4))
            pressure = struct.unpack('>f', f.read(4))
            temp = struct.unpack('>f', f.read(4))

            f_time.write("%s\n" % time[0])
            f_pressure.write("%s\n" % pressure[0])
            f_temp.write("%s\n" % temp[0])

            points += 1
        except struct.error:
            break

    f_time.close()
    f_pressure.close()
    f_temp.close()

print("Successfully decoded %s telemetry points" % points)
