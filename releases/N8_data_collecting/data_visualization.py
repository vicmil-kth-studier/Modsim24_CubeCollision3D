"""
Plot the data that was recieved from the collision simulation
"""
import matplotlib.pyplot as plt
import numpy as np
import json
import sys
from pathlib import Path
import pathlib
sys.path.append(str(Path(__file__).resolve().parents[0])) 


parents = pathlib.Path(__file__).parents
dir_path = str(parents[0].resolve())
with open(dir_path + '/falling_cube.json') as f:
  data = json.load(f)

print(data)

x = data["time_data_s"]
y1 = data["total_energy_data_J"]
y2 = data["kinetic_energy_J"]
y3 = data["potential_energy_J"]

plt.plot(x, y1)
plt.plot(x, y2)
plt.plot(x, y3)

plt.title("Title")
plt.show()