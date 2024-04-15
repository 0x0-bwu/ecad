
import os
import numpy as np
import matplotlib.pyplot as plt

label = [f'M{n}' for n in range(1, 13)]
result = [161.198,166.913,161.96,161.525,167.298,161.888,170.348,174.621,163.845,169.878,174.393,163.915]
icepak = [162.99 ,168.39 ,163.61,156.11, 167.05 ,163.02 ,172.26 ,176.24 ,165.30 ,172.19, 176.19 ,165.29]

plt.title("Static Simulation Result Correlation")
plt.plot(label, result, label='Cauer', marker='^', color='blue')
plt.plot(label, icepak, label='Icepak', marker='s', color='red')
plt.ylabel('Temperature/Cel')
plt.legend()
plt.show()