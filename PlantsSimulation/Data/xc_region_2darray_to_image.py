import os

import numpy as np
from PIL import Image
import matplotlib.pyplot as plt
from pathlib import Path


def visualize_region_map(raw_file_path, row, column, dtype='int32', output_path='region_map.png', 
                         transposed=False, hasAlpha=False, alphaTransparent=False,  output_width=None, output_height=None,
                         rotation=0):
    """
    Read a raw binary file containing region IDs and create a colored visualization.
    
    Parameters:
    -----------
    raw_file_path : str
        Path to the raw binary file containing the region map
    row : int
        Number of rows in the array
    column : int
        Number of columns in the array
    dtype : str, optional
        Data type of the array elements (default: 'int32')
    output_path : str, optional
        Path for the output PNG file (default: 'region_map.png')
    transposed : bool, optional
        If True, transpose the array to swap x and y coordinates (default: False)
    hasAlpha : bool, optional
        If True, create RGBA image with alpha channel; if False, create RGB image (default: False)
    output_width : int, optional
        Width of the output image in pixels. If None, uses column count of the array (default: None)
    output_height : int, optional
        Height of the output image in pixels. If None, uses row count of the array (default: None)
    rotation : float, optional
        Angle (in degrees) to rotate the output image counterclockwise. Default: 0 (no rotation)
        Common values: 0, 90, 180, 270
    
    Returns:
    --------
    numpy.ndarray
        The region array that was visualized
    """
    
    # Read the raw binary file
    data = np.fromfile(raw_file_path, dtype=dtype)
    
    # Reshape to row x column array
    region_array = data.reshape((row, column))
    
    # Transpose if requested
    if transposed:
        region_array = region_array.T  # 转置后形状变为 (column, row)
    
    # Get unique region IDs (excluding 0 which means no region)
    unique_regions = np.unique(region_array)
    unique_regions = unique_regions[unique_regions != 0]  # Remove 0 if present
    
    print(f"Array shape: {region_array.shape} (rows x columns)")
    print(f"Number of unique regions: {len(unique_regions)}")
    print(f"Region IDs: {unique_regions}")
    
    # Create a color map
    cmap = plt.get_cmap('tab20')  # Good for up to 20 distinct colors
    if len(unique_regions) > 20:
        cmap = plt.get_cmap('hsv')  # For more regions
    
    # Create RGB image (shape: row x column x 3)
    img_array = np.zeros((region_array.shape[0], region_array.shape[1], 3), dtype=np.uint8)
    
    # Assign colors to each region
    for idx, region_id in enumerate(unique_regions):
        color = cmap(idx / max(len(unique_regions) - 1, 1))[:3]  # Get RGB, exclude alpha
        color_rgb = (np.array(color) * 255).astype(np.uint8)
        mask = region_array == region_id
        img_array[mask] = color_rgb
    
    # Handle alpha channel if required
    if hasAlpha:
        alpha = np.ones((region_array.shape[0], region_array.shape[1]), dtype=np.uint8) * 255
        alpha[region_array == 0] = 0  # Set region 0 to transparent
        if not alphaTransparent:
            alpha[region_array == 0] = 255  # Set region 0 to 255 (opaque) if not transparent
        img_array = np.dstack((img_array, alpha))
        mode = 'RGBA'
    else:
        mode = 'RGB'
    
    # Create base image from array
    img = Image.fromarray(img_array, mode=mode)
    
    # Determine output size (default to original array dimensions if not specified)
    target_width = output_width if output_width is not None else region_array.shape[1]
    target_height = output_height if output_height is not None else region_array.shape[0]
    
    # Resize image to target dimensions (use high-quality resampling)
    img = img.resize((target_width, target_height), resample=Image.LANCZOS)
    
    # Apply rotation (PIL's rotate uses counterclockwise angle in degrees)
    if rotation != 0:
        img = img.rotate(rotation, expand=True)  # expand=True ensures no cropping after rotation
    
    # Save rotated image
    img.save(output_path)
    
    print(f"Region map saved to: {output_path} (size: {img.size[0]}x{img.size[1]} pixels, rotated {rotation}°)")
    
    return region_array


def visualize_region_map_with_legend(raw_file_path, row, column, dtype='int32', 
                                     output_path='region_map.png', dpi=100, transposed=False,
                                     output_width=None, output_height=None, rotation=0):
    """
    Similar to visualize_region_map but includes a legend showing region IDs.
    
    Parameters:
    -----------
    raw_file_path : str
        Path to the raw binary file containing the region map
    row : int
        Number of rows in the array
    column : int
        Number of columns in the array
    dtype : str, optional
        Data type of the array elements (default: 'int32')
    output_path : str, optional
        Path for the output PNG file (default: 'region_map.png')
    dpi : int, optional
        Resolution of the output image (default: 100)
    transposed : bool, optional
        If True, transpose the array to swap x and y coordinates (default: False)
    output_width : int, optional
        Width of the output image in pixels. If None, uses column count of the array (default: None)
    output_height : int, optional
        Height of the output image in pixels. If None, uses row count of the array (default: None)
    rotation : float, optional
        Angle (in degrees) to rotate the output image counterclockwise. Default: 0 (no rotation)
        Common values: 0, 90, 180, 270
    
    Returns:
    --------
    numpy.ndarray
        The region array that was visualized
    """
    
    # Read the raw binary file
    data = np.fromfile(raw_file_path, dtype=dtype)
    region_array = data.reshape((row, column))  # 重塑为 row x column 形状
    
    # Transpose if requested
    if transposed:
        region_array = region_array.T  # 转置后形状变为 (column, row)
    
    # Apply rotation to the region array (for matplotlib rendering)
    # Convert rotation angle to number of 90-degree steps (counterclockwise)
    if rotation % 360 != 0:
        rotation_steps = int((rotation % 360) / 90)  # 90° → 1 step, 180° → 2 steps, etc.
        region_array = np.rot90(region_array, k=rotation_steps)  # 旋转数组以匹配图像方向
    
    # Get unique region IDs
    unique_regions = np.unique(region_array)
    
    print(f"Array shape (after rotation): {region_array.shape} (rows x columns)")
    print(f"Number of unique regions: {len(unique_regions[unique_regions != 0])}")
    print(f"Region IDs: {unique_regions[unique_regions != 0]}")
    
    # Determine output size (adjust for rotation if needed)
    # For 90/270 rotation, width/height swap; for 180, remains the same
    original_target_width = output_width if output_width is not None else region_array.shape[1]
    original_target_height = output_height if output_height is not None else region_array.shape[0]
    
    # Adjust target dimensions for rotation (90/270 degrees swap width and height)
    if rotation % 180 == 90:  # 90 or 270 degrees
        target_width, target_height = original_target_height, original_target_width
    else:
        target_width, target_height = original_target_width, original_target_height
    
    # Calculate figure size based on target dimensions and dpi
    figsize = (target_width / dpi, target_height / dpi)
    fig, ax = plt.subplots(figsize=figsize)
    
    # Create custom colormap
    cmap = plt.get_cmap('tab20')
    if len(unique_regions) > 20:
        cmap = plt.get_cmap('hsv')
    
    # Display the image (interpolation='nearest' to preserve sharp boundaries)
    im = ax.imshow(region_array, cmap=cmap, interpolation='nearest')
    
    # Add colorbar with region IDs
    cbar = plt.colorbar(im, ax=ax)
    cbar.set_label('Region ID', rotation=270, labelpad=15)
    
    ax.set_title(f'Region Map (Rotated {rotation}°)')
    ax.set_xlabel('Column Index')
    ax.set_ylabel('Row Index')
    
    # Adjust layout to fit legend and labels
    plt.tight_layout()
    
    # Save image with target dimensions
    plt.savefig(output_path, dpi=dpi, bbox_inches='tight')
    plt.close()
    
    print(f"Region map with legend saved to: {output_path} (size: {target_width}x{target_height} pixels, rotated {rotation}°)")
    
    return region_array


# Example usage:
if __name__ == "__main__":
    # Example 1: Basic visualization with 90° rotation
    # region_array = visualize_region_map(
    #     raw_file_path='region_map.raw',
    #     row=512,
    #     column=768,
    #     dtype='int32',
    #     output_path='region_map_rot90.png',
    #     transposed=False,
    #     output_width=1024,
    #     output_height=1536,
    #     rotation=90  # 逆时针旋转90度
    # )
    
    # Example 2: Legend visualization with 180° rotation
    # region_array = visualize_region_map_with_legend(
    #     raw_file_path='region_map.raw',
    #     row=256,
    #     column=512,
    #     dtype='int32',
    #     output_path='region_map_legend_rot180.png',
    #     dpi=150,
    #     transposed=False,
    #     rotation=180  # 逆时针旋转180度
    # )
    
    pass
    
    intputfolder = Path('D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\RoadObjInfo')
    outputfolder = Path('D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\RoadObjInfo')
    inputfileName = '12_4_2_regions.raw'
    outputfileName = '12_4_2_regions_visualized.png'
    inputPath = os.path.join(intputfolder, inputfileName)
    outputPath = os.path.join(outputfolder, outputfileName)
    datatype = 'int32'
    array_row = 600
    array_column = 600
    image_width = 600
    image_height = 600
    isAlpha = True
    transposed_flag = True
    rotate_angle = 0  # 0, 90, 180, 270
    alphaHasTransparent = False
    
    region_array = visualize_region_map(
        raw_file_path=inputPath,
        row=array_row,
        column=array_column,
        dtype=datatype,
        output_path=outputPath,
        transposed=transposed_flag,
        rotation=rotate_angle,
        hasAlpha=isAlpha,
        alphaTransparent=alphaHasTransparent,
        output_width=image_width,
        output_height=image_height
    )
    
    #print(region_array)
    


    pass