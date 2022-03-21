import matplotlib.pyplot as plt
import numpy as np
from scipy.interpolate import interp1d

x_array = [2, 4, 8, 16, 20, 40]
y_array = [101.266, 58.3955, 45.395, 37.3023, 34.4435, 31.764]
plt.figure(figsize=(13.33,7.5))

plt.plot(x_array, y_array, 'ro', ms=5.0, label='Completion Time')

plt.legend(fontsize=13, loc=0)

plt.xlabel('Number of Nodes' , fontsize=16)
plt.ylabel('Time (Seconds)', fontsize=16)

x1,x2,y1,y2 = plt.axis()  
plt.axis((0,42,y1,y2))

plt.savefig('plot.png', bbox_inches='tight')