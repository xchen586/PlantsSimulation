import numpy as np
from PIL import Image
import matplotlib.pyplot as plt
import matplotlib.cm as cm

# Parameters
#filename = "your_raw_file.bin"  # Path to your raw file
#width = 512                    # Width of the 2D array/image
#height = 512                   # Height of the 2D array/image
#dtype = np.uint8              # Data type (e.g. np.uint8, np.float32)

width = 600
height = 600

# Clamp 范围
vmin = 0    # 你可以根据数据实际情况修
vmax = 200  # 你可以根据数据实际情况修改

filename = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\RoadObjInfo\\12_4_2_slopes.raw'

dtype = np.int32  # or np.float32, np.int16, etc.

# === 步骤 1: 读取原始 2D 数据 ===
data = np.fromfile(filename, dtype=dtype)
array_2d = data.reshape((height, width))  # (rows, cols)

#vmax = array_2d.max()

#output_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\12_4_2\\12_4_2_slopes_gradient_{vmin}_{vmax}.png'
#output_original_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\12_4_2\\12_4_2_slopes_gradient_{vmin}_{vmax}_red.png'

output_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\12_4_2\\12_4_2_slopes_new.png'
output_original_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\12_4_2\\12_4_2_slopes_new_red.png'


# === 步骤 2: 归一化到 [0, 255] 作为红色通道 ===
min_val = array_2d.min()
max_val = array_2d.max()

normalized = ((array_2d - min_val) / (max_val - min_val) * 255).astype(np.uint8)

# === 步骤 3: 创建 RGB 图像，仅红色通道有值 ===
rgb_image = np.zeros((height, width, 3), dtype=np.uint8)
rgb_image[..., 0] = normalized  # 红色通道


'''
# === 步骤 2: Clamp + Normalize 到 [0, 255] ===
clamped = np.clip(array_2d, vmin, vmax)
normalized = (clamped - vmin) / (vmax - vmin)  # 归一化到 [0,1]
scaled = (normalized * 255).astype(np.uint8)

# === 步骤 3: 构造 RGB 图像，红色通道填入数值 ===
rgb_image = np.zeros((height, width, 3), dtype=np.uint8)
rgb_image[..., 0] = scaled  # 红色通道

'''

'''
# === Step 2: Clamp and normalize to [0, 1] ===
clamped = np.clip(array_2d, vmin, vmax)
normalized = (clamped - vmin) / (vmax - vmin)  # values in [0, 1]

# === Step 3: Blue-to-Red RGB mapping ===
# We'll go from Blue (0, 0, 255) → Red (255, 0, 0)
# i.e., R = norm * 255, G = 0, B = (1 - norm) * 255

r = (normalized * 255).astype(np.uint8)
g = np.zeros_like(r, dtype=np.uint8)
b = ((1 - normalized) * 255).astype(np.uint8)

# Stack into RGB image
rgb_image = np.stack([r, g, b], axis=2)
'''

'''
# === 第二步：裁剪并归一化 ===
clamped = np.clip(array_2d, vmin, vmax)
normalized = (clamped - vmin) / (vmax - vmin)  # 映射到 [0, 1]

# === 第三步：使用 colormap 映射为 RGB 颜色 ===
colormap = plt.colormaps['jet']  # 你也可以试 'plasma'、'viridis'、'turbo' 等
rgb_image = (colormap(normalized)[:, :, :3] * 255).astype(np.uint8)  # 取 RGB，去掉 alpha
'''

# === 步骤 4: 保存原始红色通道图像 ===
Image.fromarray(rgb_image, mode='RGB').save(output_original_path)

# === 步骤 5: 沿 y = -x 对称翻转图像 ===
flipped = np.transpose(rgb_image, (1, 0, 2))  # 转置行列
#flipped = np.flipud(flipped)                 # 上下翻转

# === 步骤 6: 保存翻转后的图像 ===
Image.fromarray(flipped, mode='RGB').save(output_path)