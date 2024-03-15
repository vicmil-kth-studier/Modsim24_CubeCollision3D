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
with open(dir_path + '/data_example.json') as f:
  data = json.load(f)

print(data)

x = data["time_data_s"]
y = data["energy_J"]

plt.plot(x, y)

plt.title("Title")
plt.show()