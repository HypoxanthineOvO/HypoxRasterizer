import os
import numpy as np
import matplotlib.pyplot as plt

input_file = "./Tests/TriangleTestByPython/input.txt"
output_file = "./Tests/TriangleTestByPython/output.txt"

n = 10000
random_3n_array = np.random.rand(n, 3)
random_3n_array[:, -1] = 1

with open(input_file, 'w') as f:
    for value in random_3n_array:
        f.write(f"{value[0]} {value[1]} {value[2]}\n")

os.system("xmake")
os.system("./TriangleTestpy")

cpp_results = []
with open(output_file, 'r') as f:
    for line in f:
        cpp_results.append(bool(int(line.strip()))) 

inside_points = random_3n_array[np.array(cpp_results)]
outside_points = random_3n_array[~np.array(cpp_results)]
triangle_vertices = np.array([[0.25, 0.5], [0.75, 0.75], [0.75, 0.25]])
plt.scatter(inside_points[:, 0], inside_points[:, 1], color='green', label='Inside Triangle')
plt.scatter(outside_points[:, 0], outside_points[:, 1], color='blue', label='Outside Triangle')
triangle = plt.Polygon(triangle_vertices, fill=None, edgecolor='red', linewidth=2)
plt.gca().add_patch(triangle)
plt.legend()
plt.title("Points Inside and Outside Triangle")
plt.xlabel("X")
plt.ylabel("Y")

plt.savefig('TriangleInside.png')
