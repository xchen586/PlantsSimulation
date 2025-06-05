import numpy as np
import matplotlib.pyplot as plt

# File parameters — customize these
filename = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\RoadObjInfo\\12_4_2_slopes.raw'
width = 600
height = 600
dtype = np.int32  # or np.float32, np.int16, etc.

# Load binary file
data = np.fromfile(filename, dtype=dtype)

# Reshape to 2D
array = data.reshape((height, width))

# Display as map
plt.imshow(array, cmap='gray')  # Try cmap='viridis' or 'terrain' for color
plt.colorbar()
plt.title("2D Array Map View")
plt.show()