import numpy as np
from PIL import Image
import struct
import argparse
import os

class RawToPNGConverter:
    def __init__(self):
        self.supported_dtypes = {
            'uint8': np.uint8,
            'int8': np.int8,
            'uint16': np.uint16,
            'int16': np.int16,
            'uint32': np.uint32,
            'int32': np.int32,
            'float32': np.float32,
            'float64': np.float64
        }
    
    def read_raw_file(self, filepath, width, height, dtype='uint8', endian='little'):
        """
        读取raw文件数据
        
        Args:
            filepath: raw文件路径
            width: 图像宽度
            height: 图像高度
            dtype: 数据类型 ('uint8', 'int8', 'uint16', 'int16', 'uint32', 'int32', 'float32', 'float64')
            endian: 字节序 ('little' 或 'big')
        """
        if dtype not in self.supported_dtypes:
            raise ValueError(f"不支持的数据类型: {dtype}")
        
        np_dtype = self.supported_dtypes[dtype]
        
        # 读取二进制数据
        with open(filepath, 'rb') as f:
            data = f.read()
        
        # 根据数据类型和字节序解析数据
        if endian == 'little':
            endian_char = '<'
        else:
            endian_char = '>'
        
        # 构造struct格式字符串
        type_map = {
            'uint8': 'B', 'int8': 'b',
            'uint16': 'H', 'int16': 'h',
            'uint32': 'I', 'int32': 'i',
            'float32': 'f', 'float64': 'd'
        }
        
        format_char = type_map[dtype]
        expected_size = width * height * np.dtype(np_dtype).itemsize
        
        if len(data) < expected_size:
            raise ValueError(f"文件大小不足，期望 {expected_size} 字节，实际 {len(data)} 字节")
        
        # 解析数据
        format_str = f"{endian_char}{width * height}{format_char}"
        values = struct.unpack(format_str, data[:expected_size])
        
        # 转换为numpy数组并重塑
        array = np.array(values, dtype=np_dtype).reshape((height, width))
        
        return array
    
    def normalize_array(self, array):
        """
        将数组归一化到0-255范围
        """
        # 处理浮点数据
        if array.dtype in [np.float32, np.float64]:
            array = array.astype(np.float64)
        
        # 获取最小值和最大值
        min_val = np.min(array)
        max_val = np.max(array)
        
        # 避免除零错误
        if max_val == min_val:
            return np.zeros_like(array, dtype=np.uint8)
        
        # 归一化到0-255
        normalized = ((array - min_val) / (max_val - min_val) * 255).astype(np.uint8)
        
        return normalized
    
    def create_grayscale_image(self, array):
        """
        创建灰度图像
        """
        normalized = self.normalize_array(array)
        return Image.fromarray(normalized, mode='L')
    
    def create_rgb_image(self, array):
        """
        创建RGB图像（使用颜色映射）
        """
        normalized = self.normalize_array(array)
        
        # 创建RGB数组
        rgb_array = np.zeros((array.shape[0], array.shape[1], 3), dtype=np.uint8)
        
        # 简单的热图颜色映射
        # 蓝色 -> 青色 -> 绿色 -> 黄色 -> 红色
        for i in range(array.shape[0]):
            for j in range(array.shape[1]):
                val = normalized[i, j]
                if val < 64:  # 蓝色到青色
                    rgb_array[i, j] = [0, val * 4, 255]
                elif val < 128:  # 青色到绿色
                    rgb_array[i, j] = [0, 255, 255 - (val - 64) * 4]
                elif val < 192:  # 绿色到黄色
                    rgb_array[i, j] = [(val - 128) * 4, 255, 0]
                else:  # 黄色到红色
                    rgb_array[i, j] = [255, 255 - (val - 192) * 4, 0]
        
        return Image.fromarray(rgb_array, mode='RGB')
    
    def create_single_channel_image(self, array, channel='r'):
        """
        创建单色通道图像
        
        Args:
            array: 输入数组
            channel: 颜色通道 ('r', 'g', 'b')
        """
        normalized = self.normalize_array(array)
        
        # 创建RGB数组
        rgb_array = np.zeros((array.shape[0], array.shape[1], 3), dtype=np.uint8)
        
        # 根据选择的通道填充
        if channel.lower() == 'r':
            rgb_array[:, :, 0] = normalized
        elif channel.lower() == 'g':
            rgb_array[:, :, 1] = normalized
        elif channel.lower() == 'b':
            rgb_array[:, :, 2] = normalized
        else:
            raise ValueError("通道必须是 'r', 'g', 或 'b'")
        
        return Image.fromarray(rgb_array, mode='RGB')
    
    def convert_to_png(self, input_file, output_file, width, height, 
                      dtype='uint8', endian='little', color_mode='grayscale', 
                      channel='r', transpose=False):
        """
        将raw文件转换为PNG图片
        
        Args:
            input_file: 输入raw文件路径
            output_file: 输出PNG文件路径
            width: 图像宽度
            height: 图像高度
            dtype: 数据类型
            endian: 字节序
            color_mode: 颜色模式 ('grayscale', 'rgb', 'single_channel')
            channel: 单通道模式下的颜色通道
            transpose: 是否交换横纵坐标轴 (转置矩阵)
        """
        # 读取raw数据
        print(f"正在读取文件: {input_file}")
        array = self.read_raw_file(input_file, width, height, dtype, endian)
        
        # 如果需要转置矩阵（交换横纵坐标）
        if transpose:
            array = np.transpose(array)
            print(f"已转置矩阵 - 交换横纵坐标轴")
        
        print(f"数据形状: {array.shape}")
        print(f"数据类型: {array.dtype}")
        print(f"数据范围: {np.min(array)} ~ {np.max(array)}")
        
        # 根据颜色模式生成图像
        if color_mode == 'grayscale':
            image = self.create_grayscale_image(array)
        elif color_mode == 'rgb':
            image = self.create_rgb_image(array)
        elif color_mode == 'single_channel':
            image = self.create_single_channel_image(array, channel)
        else:
            raise ValueError("颜色模式必须是 'grayscale', 'rgb', 或 'single_channel'")
        
        # 保存图像
        image.save(output_file)
        print(f"图像已保存到: {output_file}")
        
        return image

def main():
    parser = argparse.ArgumentParser(description='将raw文件转换为PNG图片')
    parser.add_argument('input_file', help='输入raw文件路径')
    parser.add_argument('output_file', help='输出PNG文件路径')
    parser.add_argument('width', type=int, help='图像宽度')
    parser.add_argument('height', type=int, help='图像高度')
    parser.add_argument('--dtype', default='uint8', 
                       choices=['uint8', 'int8', 'uint16', 'int16', 'uint32', 'int32', 'float32', 'float64'],
                       help='数据类型 (默认: uint8)')
    parser.add_argument('--endian', default='little', choices=['little', 'big'],
                       help='字节序 (默认: little)')
    parser.add_argument('--color-mode', default='grayscale', 
                       choices=['grayscale', 'rgb', 'single_channel'],
                       help='颜色模式 (默认: grayscale)')
    parser.add_argument('--channel', default='r', choices=['r', 'g', 'b'],
                       help='单通道模式下的颜色通道 (默认: r)')
    parser.add_argument('--transpose', action='store_true',
                       help='交换横纵坐标轴 (转置矩阵)')
    
    args = parser.parse_args()
    
    # 检查输入文件是否存在
    if not os.path.exists(args.input_file):
        print(f"错误: 文件不存在 - {args.input_file}")
        return
    
    # 创建转换器并执行转换
    converter = RawToPNGConverter()
    try:
        converter.convert_to_png(
            args.input_file, 
            args.output_file, 
            args.width, 
            args.height,
            args.dtype, 
            args.endian, 
            args.color_mode, 
            args.channel,
            args.transpose
        )
    except Exception as e:
        print(f"转换失败: {e}")

# 使用示例
if __name__ == "__main__":
    # 方法1：直接在代码中指定参数（推荐用于你的情况）
    converter = RawToPNGConverter()
    
    # 你的具体参数
    input_file = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\12_4_2_exposure_map.raw'
    output_file = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\12_4_2\\12_4_2_exposure_map_rgb.png'
    width = 4096
    height = 4096
    dtype = 'float64'  # 对应你的 'double' 类型
    color_mode = 'rgb'  # 或 'grayscale', 'rgb'
    transpose = True  # 设置为 True 可以交换横纵坐标轴
    
    try:
        converter.convert_to_png(
            input_file=input_file,
            output_file=output_file,
            width=width,
            height=height,
            dtype=dtype,
            color_mode=color_mode,
            transpose=transpose
        )
        print("转换完成！")
        
        # 如果你想同时生成转置版本的图片
        if not transpose:
            output_file_transposed = output_file.replace('.png', '_transposed.png')
            converter.convert_to_png(
                input_file=input_file,
                output_file=output_file_transposed,
                width=width,
                height=height,
                dtype=dtype,
                color_mode=color_mode,
                transpose=True
            )
            print(f"转置版本已保存到: {output_file_transposed}")
            
    except Exception as e:
        print(f"转换失败: {e}")
    
    # 方法2：如果你想使用命令行参数，请取消下面的注释
    # main()
    
    
    
    