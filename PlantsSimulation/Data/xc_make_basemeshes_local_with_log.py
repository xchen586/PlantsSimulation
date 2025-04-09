import os
from os import path
import sys
import time
import glob

import shutil
import subprocess
import argparse
import datetime

from datetime import timedelta
from timeit import default_timer as timer
#from distutils.dir_util import copy_tree
from shutil import copytree

import glob
import configparser
from io import StringIO
from zipfile import ZipFile

import pandas as pd

from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
from voxelfarm import process_lambda

class DualOutput:
    def __init__(self, file_path):
        self.console = sys.stdout  # Save the current console output stream
        self.log_file = open(file_path, 'a')  # Open a file for logging

    def write(self, message):
        self.console.write(message)  # Write to the console
        self.log_file.write(message)  # Write to the log file

    def flush(self):
        self.console.flush()
        self.log_file.flush()

def exit_code(code):
    exit(code)
    
def is_valid_float_string(s):
    try:
        float(s)
        return True
    except ValueError:
        return False
    
def RemoveBaseMeshesdata(level_index_db_file_path, level_data_db_file_path):
    if os.path.exists(level_index_db_file_path):
        os.remove(level_index_db_file_path)  # Remove the file
        print(f"File '{level_index_db_file_path}' has been removed.")
    if os.path.exists(level_data_db_file_path):
        os.remove(level_data_db_file_path)  # Remove the file
        print(f"File '{level_data_db_file_path}' has been removed.")
        
def xc_run_tool(tool_path, progress_start, progress_end):    
    
    print(f'run tree tool_path:\n{tool_path}')
    start = progress_start
    end = progress_end
    scale = (end - start) / 100

    try:
        tool_process = subprocess.Popen(tool_path, stdout=subprocess.PIPE, stderr=None, text=True)
        while True:
            realtime_output = tool_process.stdout.readline()
            if realtime_output:
                tokens = realtime_output.split()
                progress = -1
                if len(tokens) > 2:
                    if tokens[0] == 'progress':
                        progress_string = tokens[1]
                        if is_valid_float_string(progress_string):
                            tool_progress = float(progress_string)
                            # Continue with further processing using float_value
                        else:
                            tool_progress = 0
                            print(f'Cannot convert input progress tokens[1] {progress_string} to float:')
                        progress = start + tool_progress * scale
                        message = ""
                        for token in tokens[2:]:                   
                            message += token + " "
                        print(message)
                if progress == -1:
                    print(realtime_output.replace('\n', ''))
            else: 
                poll = tool_process.poll() 
                if poll is not None:
                    break
    except Exception as e:
        exception_message = str(e)
        exception_repr_message = repr(e)
        print(f'exception_message of xc_run_tool for {tool_path} with exception of {exception_message}')
        print(f'exception_repr_message of xc_run_tool {tool_path} with repr message of {exception_repr_message}')
    
    return tool_process.returncode 

start_time = time.time()
now = datetime.datetime.now()
# Format the current time into a string (e.g., 2024-10-17_14-35-50)
filename = now.strftime("%Y-%m-%d_%H-%M-%S")

scrap_folder= f'D:\\Downloads\\XCTreeCreation'
if not os.path.exists(scrap_folder):
    os.makedirs(scrap_folder)
# Add file extension if needed (e.g., .txt)
log_file_name = f"make_basemeshes_db_{filename}.log"
log_file_path = os.path.join(scrap_folder, log_file_name)
sys.stdout = DualOutput(log_file_path)  # Redirect stdout

Tools_folder = f'D:\\Downloads\\XCTreeWorkFlow\\Tools'
basemeshes_exe_name = f'BaseMeshVoxelizerCmd.exe'
basemeshes_exe_path = os.path.join(Tools_folder, basemeshes_exe_name)

tiles_count = 12
tiles_x = 4
tiles_y = 2
basemeshes_level = 0

basemeshes_assets_folder = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation'
basemeshes_db_base_folder = os.path.join(basemeshes_assets_folder, f'db')
basemeshes_caves_db_base_folder = os.path.join(basemeshes_assets_folder, f'cavesdb')
basemeshes_cache_base_folder = os.path.join(basemeshes_assets_folder, f'cache')
basemeshes_heightmap_folder = os.path.join(basemeshes_assets_folder, f'heightmap') 
basemeshes_debug_level = 0

level_db_output_folder = os.path.join(basemeshes_db_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level}')
index_db_file_name = f'index.vf'
data_db_file_name = f'data.vf'
level_index_db_file_path = os.path.join(level_db_output_folder, index_db_file_name)
level_data_db_file_path = os.path.join(level_db_output_folder, data_db_file_name)
print(f'level_index_db_file_path: {level_index_db_file_path}')
print(f'level_data_db_file_path: {level_data_db_file_path}')

print(f'start to remove basemeshes db files: {level_index_db_file_path} and {level_data_db_file_path}')
RemoveBaseMeshesdata(level_index_db_file_path, level_data_db_file_path)
    
basemeshvoxelizer_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level} {basemeshes_assets_folder} {basemeshes_db_base_folder} {basemeshes_cache_base_folder} {basemeshes_debug_level} {basemeshes_heightmap_folder}'
print(f'step for to run_make_basemeshes : {basemeshvoxelizer_command}')

return_code_basemash = xc_run_tool(basemeshvoxelizer_command, 0, 100)
if return_code_basemash == 0:
    print(f'Process ({basemeshvoxelizer_command}) executed successfully.')
else:
    print(f'Error: The process ({basemeshvoxelizer_command}) returned a non-zero exit code ({return_code_basemash}).')