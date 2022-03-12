import matplotlib.pyplot as plt
import numpy as np
from scipy.interpolate import interp1d

x_array = [2, 4, 8, 16, 20, 40]
y_array = [41.1074, 16.874, 8.6416, 4.5123, 4.49402, 2.75582]
plt.figure(figsize=(13.33,7.5))

plt.plot(x_array, y_array, 'ro', ms=5.0, label='Completion Time')

for i in range(0, 6):
    y_array[i] = x_array[i]*y_array[i]

plt.plot(x_array, y_array, 'bo', ms=5.0, label='Total Processor Time')

x = np.linspace(0, 42, 100)
y = np.linspace(41.1074*2, 41.1074*2, 100)
plt.plot(x, y, 'b--')

plt.legend(fontsize=13, loc=0)

plt.xlabel('Number of Nodes' , fontsize=16)
plt.ylabel('Time (Seconds)', fontsize=16)

x1,x2,y1,y2 = plt.axis()  
plt.axis((0,42,y1,y2))

plt.savefig('plot.png', bbox_inches='tight')