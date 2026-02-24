import struct
import numpy as np
from PIL import Image
import sys
import os

def load_distance_field(file_path):
    """
    Load binary file according to C++ SaveToFile/LoadFromFile logic.
    File Format:
    1. width (int, 4 bytes)
    2. height (int, 4 bytes)
    3. meterPerPixel (float, 4 bytes)
    4. data (float array, width * height * 4 bytes)
    
    Data storage order: Column-major (index = col * height + row)
    """
    if not os.path.exists(file_path):
        print(f"Error: File not found - {file_path}")
        return None, None, None

    try:
        with open(file_path, 'rb') as f:
            # 1. Read file header
            width_data = f.read(4)
            if len(width_data) < 4: raise ValueError("File corrupted: Cannot read width")
            width = struct.unpack('i', width_data)[0]

            height_data = f.read(4)
            if len(height_data) < 4: raise ValueError("File corrupted: Cannot read height")
            height = struct.unpack('i', height_data)[0]

            meter_data = f.read(4)
            if len(meter_data) < 4: raise ValueError("File corrupted: Cannot read meterPerPixel")
            meter_per_pixel = struct.unpack('f', meter_data)[0]

            print(f"File info: width={width}, height={height}, scale={meter_per_pixel:.4f} m/pixel")

            if width <= 0 or height <= 0:
                print("Error: Invalid dimensions")
                return None, None, None

            # 2. Read data body
            expected_float_count = width * height
            expected_bytes = expected_float_count * 4
            
            data_bytes = f.read(expected_bytes)
            
            if len(data_bytes) != expected_bytes:
                print(f"Warning: Insufficient data read. Expected {expected_bytes} bytes, got {len(data_bytes)} bytes.")
                if len(data_bytes) == 0:
                    return None, None, None

            # Convert binary data to float array
            raw_data = struct.unpack(f'{len(data_bytes)//4}f', data_bytes)
            
            # 3. Reshape array
            # C++ storage logic: index = col * height + row (Column-major)
            np_array_col_major = np.array(raw_data, dtype=np.float32).reshape((width, height))
            
            # Transpose to image format (height, width)
            distance_map = np_array_col_major.T
            
            return distance_map, width, height

    except Exception as e:
        print(f"Error reading file: {e}")
        return None, None, None

def normalize_and_colorize(distance_map, reverse=False):
    """
    Convert distance map to RGB image.
    
    Rules (reverse=False, default):
    - 0.0 -> Black (0, 0, 0)
    - 1.0 -> Red (255, 0, 0)
    
    Rules (reverse=True):
    - 0.0 -> Red (255, 0, 0)
    - 1.0 -> Black (0, 0, 0)
    
    - Intermediate values linearly interpolated
    - Values > 1.0 clipped accordingly
    """
    if distance_map is None:
        return None

    h, w = distance_map.shape
    
    # Create output array (H, W, 3)
    rgb_image = np.zeros((h, w, 3), dtype=np.uint8)
    
    # Get valid data mask
    valid_mask = np.isfinite(distance_map)
    valid_data = distance_map[valid_mask]
    
    if valid_data.size == 0:
        print("Warning: No valid distance data")
        return Image.fromarray(rgb_image)

    target_max = 1.0
    
    # Normalize and clip
    normalized = valid_data / target_max
    normalized = np.clip(normalized, 0.0, 1.0)

    # Apply reverse if needed
    if reverse:
        # 0.0 -> 1.0 (red), 1.0 -> 0.0 (black)
        normalized = 1.0 - normalized

    # Map to Red channel
    red_channel = (normalized * 255).astype(np.uint8)
    
    rgb_image[valid_mask, 0] = red_channel # R
    rgb_image[valid_mask, 1] = 0           # G
    rgb_image[valid_mask, 2] = 0           # B
    
    return Image.fromarray(rgb_image)

def main():
    # ==========================================
    # 配置区域：在此处直接修改文件路径和参数
    # ==========================================
    
    # 输入文件路径 (C++ 生成的二进制文件)
    # 可以使用相对路径 (如 "data.bin") 或绝对路径 (如 "C:/Users/Name/data.bin")
    INPUT_FILE = "D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\12_4_2\\12_4_2_world_road_distance.dat"
    
    # 输出图片路径
    OUTPUT_FILE = "D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\12_4_2\\12_4_2_world_road_distance.png"
    
    # 颜色反转选项
    # False: 0.0=黑色, 1.0=红色 (默认)
    # True:  0.0=红色, 1.0=黑色
    REVERSE = True
    
    # ==========================================
    # 以下代码无需修改
    # ==========================================

    print(f"Loading file: {INPUT_FILE} ...")
    data, w, h = load_distance_field(INPUT_FILE)

    if data is None:
        print("Load failed, exiting.")
        return

    print(f"Data loaded successfully, dimensions: {w}x{h}")
    print(f"Data range: min={data.min():.4f}, max={data.max():.4f}")
    
    over_1_count = np.sum(data > 1.0)
    if over_1_count > 0:
        print(f"Note: {over_1_count} pixels have values greater than 1.0, will be clipped.")

    # Print color mapping mode
    if REVERSE:
        print("Color mode: REVERSE (0.0=Red, 1.0=Black)")
    else:
        print("Color mode: NORMAL (0.0=Black, 1.0=Red)")

    print("Generating RGB image...")
    img = normalize_and_colorize(data, reverse=REVERSE)

    if img:
        img.save(OUTPUT_FILE)
        print(f"Image saved successfully: {OUTPUT_FILE}")
    else:
        print("Image generation failed.")

if __name__ == "__main__":
    main()
    
  
    
    