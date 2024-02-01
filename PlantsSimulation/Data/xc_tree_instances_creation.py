import os
from os import path
import sys
import time

import shutil
import subprocess
import argparse
from datetime import timedelta
from timeit import default_timer as timer
from distutils.dir_util import copy_tree

import glob
import configparser
from zipfile import ZipFile

from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
from voxelfarm import process_lambda

def launch_process(command):
    result = subprocess.run(command, shell=True, check=True)
    return result.returncode

def create_or_overwrite_empty_file(file_path):
    # Check if the file exists
    if os.path.exists(file_path):
        # If the file exists, open it with 'w' mode to overwrite it with an empty file
        with open(file_path, 'w'):
            pass  # Using 'pass' as no content needs to be written
        print(f"File '{file_path}' overwritten as an empty file.")
    else:
        # If the file does not exist, create it with 'w' mode
        with open(file_path, 'w'):
            pass  # Using 'pass' as no content needs to be written
        print(f"File '{file_path}' created as an empty file.")

def copy_files(src_folder, dest_folder):
    # Ensure that the destination folder exists
    if not os.path.exists(dest_folder):
        os.makedirs(dest_folder)
    # Iterate over files in the source folder
    for filename in os.listdir(src_folder):
        src_filepath = os.path.join(src_folder, filename)
        dest_filepath = os.path.join(dest_folder, filename)
        # Copy the file to the destination folder, replacing if it already exists
        shutil.copy2(src_filepath, dest_filepath)
        print(f"File '{filename}' copied to '{dest_folder}'")

def is_exe_file(file_path):
    _, file_extension = os.path.splitext(file_path.lower())
    return file_extension == '.exe'

def save_data_to_file(data, file_path):
    if is_exe_file(file_path):
        print(f"The file at {file_path} is an executable (.exe) file. can not download it")
        return
    # Extract the directory path from the file_path
    directory = os.path.dirname(file_path)
    # Check if the directory exists, create it if not
    if not os.path.exists(directory):
        os.makedirs(directory)
    # Determine the mode based on the type of data
    if isinstance(data, str):
        mode = 'w'
    elif isinstance(data, bytes):
        mode = 'wb'
    else:
        raise ValueError("Unsupported data type. Only str and bytes are supported.")
    # Save the data to the file
    with open(file_path, mode) as file:
        if isinstance(data, str):
            file.write(data)
        elif isinstance(data, bytes):
            file.write(data)
    print(f"Data saved successfully to: {file_path}")

def create_or_update_ini_file(file_path, section, key, value):
    # Check if the INI file exists
    if not os.path.exists(file_path):
        # If not, create the INI file
        with open(file_path, 'w') as configfile:
            configfile.write('')
    # Read the existing INI file
    config = configparser.ConfigParser()
    # Make it not only uppercase
    config.optionxform = str
    # Use the 'with' statement to ensure proper resource management
    with open(file_path, 'r') as configfile:
        config.read_file(configfile)
    # Check if the section exists, create it if not
    if not config.has_section(section):
        config.add_section(section)
    # Check if the key exists, update it if so, add it if not
    config.set(section, key, str(value))  # Convert value to string before setting
    # Write the changes back to the INI file
    with open(file_path, 'w') as configfile:
        config.write(configfile)

def read_ini_value(file_path, section, key, default=None, value_type=str):
    config = configparser.ConfigParser()
    # Use the 'with' statement to ensure proper resource management
    with open(file_path, 'r') as configfile:
        config.read_file(configfile)
    # Check if the section and key exist
    if config.has_section(section) and config.has_option(section, key):
        # Read the value
        value = config.get(section, key)
        # Convert the value to the specified type
        if value_type == bool:
            # Convert common string representations of boolean values to bool
            if value.lower() in ('true', 'yes', 'on', '1'):
                return True
            elif value.lower() in ('false', 'no', 'off', '0'):
                return False
            else:
                # Return default if value is not recognized
                return default
        else:
            return value_type(value)
    else:
        return default

def clear_all_sections(file_path):
    # Check if the INI file exists
    if not os.path.exists(file_path):
        # If not, create the INI file
        with open(file_path, 'w') as configfile:
            configfile.write('')
    config = configparser.ConfigParser()
    # Read the existing INI file
    config.read(file_path)
    # Remove all sections from the configuration
    for section in config.sections():
        config.remove_section(section)
    # Write the changes back to the INI file
    with open(file_path, 'w') as configfile:
        config.write(configfile)

def update_attach_files_for_entity(api : voxelfarmclient.rest, project_id, entity_id, folder_path, name : str, version : int, color : bool):

    if not os.path.exists(folder_path):
        print(f'File {folder_path} does not exist')
        return
    
    # Use the os.listdir() function to get a list of filenames in the folder
    file_names = os.listdir(folder_path)

    # Create a list of file paths by joining the folder path with each file name
    file_paths = [os.path.join(folder_path, file_name) for file_name in file_names]    
    print(file_paths)

    print(f'Attaching file {file_paths} to entity {entity_id}')
    for file_path in file_paths:
        with open(file_path, "rb") as file:
            api.attach_files(project=project_id, id=entity_id, files={'file': file})

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
def tree_instances_generation(config_path):
    
    if not os.path.exists(config_path):
        print(f'Config File {config_path} does not exist')
        return
    
    cloud_url = read_ini_value(config_path, section_main, 'cloud_url')
    project_id = read_ini_value(config_path, section_main, 'project_id')
    folder_id = read_ini_value(config_path, section_main, 'folder_id')
    #tree_entity_id = 'E0070AD37D4543FCB9E70D60AE47541D' # cosmin new
    #tree_entity_id = "536674D5E8D440D9A7EFCD1D879AD57A" # cosmin old
    #tree_entity_id = "3A3CFEBA226B4692A8719C78335470DD"  #xc tesst
    tree_entity_id = read_ini_value(config_path, section_main, 'tree_entity_id')
    basemeshes_entity_id = read_ini_value(config_path, section_main, 'basemeshes_entity_id')
    
    tiles_count = read_ini_value(config_path, section_tiles, 'tiles_count', value_type=int)
    tiles_x = read_ini_value(config_path, section_tiles, 'tiles_x', value_type=int)
    tiles_y = read_ini_value(config_path, section_tiles, 'tiles_y', value_type=int)

    road_input_folder = read_ini_value(config_path, section_input, 'road_input_folder')
    road_exe_path = read_ini_value(config_path, section_input, 'road_exe_path')
    worldgen_exe_path = read_ini_value(config_path, section_input, 'worldgen_exe_path')
    basemeshes_exe_path = read_ini_value(config_path, section_input, 'basemeshes_exe_path')
    tree_exe_path = read_ini_value(config_path, section_input, 'tree_exe_path')
    qtree_assets_folder = read_ini_value(config_path, section_input, 'qtree_assets_folder')

    road_output_folder = read_ini_value(config_path, section_output, 'road_output_folder')
    smoothlayer_output_base_folder = read_ini_value(config_path, section_output, 'smoothlayer_output_base_folder')
    basemeshes_db_base_folder = read_ini_value(config_path, section_output, 'basemeshes_db_base_folder')
    basemeshes_cache_base_folder = read_ini_value(config_path, section_output, 'basemeshes_cache_base_folder')
    basemeshes_heightmap_folder = read_ini_value(config_path, section_output, 'basemeshes_heightmap_folder')
    tree_output_base_folder = read_ini_value(config_path, section_output, 'tree_output_base_folder')

    run_update_basemeshes_assets = read_ini_value(config_path, section_run, 'run_update_basemeshes_assets', value_type=bool)
    run_road_exe = read_ini_value(config_path, section_run, 'run_road_exe', value_type=bool)
    run_worldgen_road = read_ini_value(config_path, section_run, 'run_worldgen_road', value_type=bool)
    run_make_basemeshes = read_ini_value(config_path, section_run, 'run_make_basemeshes', value_type=bool)
    run_make_tree_instances = read_ini_value(config_path, section_run, 'run_make_tree_instances', value_type=bool)
    run_upload_tree_instances = read_ini_value(config_path, section_run, 'run_upload_tree_instances', value_type=bool)

    road_Heightmap_width = read_ini_value(config_path, section_road, 'road_Heightmap_width', value_type=int)
    road_heightmap_height = read_ini_value(config_path, section_road, 'road_heightmap_height', value_type=int)

    basemeshes_debug_level = read_ini_value(config_path, section_others, 'basemeshes_debug_level', value_type=int)
    tree_lod = read_ini_value(config_path, section_others, 'tree_lod', value_type=int)

    basemeshes_level0 = 0
    basemeshes_level1 = 1
    version = 80

    most_travelled_points_path = f'{road_output_folder}\\{tiles_count}_{tiles_x}_{tiles_y}_Most_Travelled_Points.csv'
    most_distant_points_path = f'{road_output_folder}\\{tiles_count}_{tiles_x}_{tiles_y}_Most_Distant_Points.csv'
    
    tree_ini_folder = f'{tree_output_base_folder}\\{tiles_count}_{tiles_x}_{tiles_y}'
    tree_ini_name = 'TreesInstancesAbsolutePathWin.ini'
    tree_ini_path = f'{tree_ini_folder}\\{tree_ini_name}'
    if not os.path.exists(tree_ini_folder):
        os.makedirs(tree_ini_folder)

    basemeshes_assets_folder = qtree_assets_folder
    basemeshes_0_heightmap_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level0}_heightarray.bin'
    basemeshes_1_heightmap_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level1}_heightarray.bin'
    basemeshes_0_heightmap_mask_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level0}_heightmasks.bin'
    basemeshes_1_heightmap_mask_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level1}_heightmasks.bin'
    basemeshes_0_heightmap_path = f'{basemeshes_heightmap_folder}\\{basemeshes_0_heightmap_name}'
    basemeshes_1_heightmap_path = f'{basemeshes_heightmap_folder}\\{basemeshes_1_heightmap_name}'
    basemeshes_0_heightmap_mask_path = f'{basemeshes_heightmap_folder}\\{basemeshes_0_heightmap_mask_name}'
    basemeshes_1_heightmap_mask_path = f'{basemeshes_heightmap_folder}\\{basemeshes_0_heightmap_mask_name}'

    smoothlayer_output_folder = f'{smoothlayer_output_base_folder}\\{tiles_count}_{tiles_x}_{tiles_y}'
    toplayer_image_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.jpg'
    toplayer_image_meta_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.jgw'
    toplayer_heightmap_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.height.raw'
    toplayer_heightmap_mask_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.height.masks.raw'
    level1_heightmap_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_level1.xyz.height.raw'
    level1_heightmap_mask_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_level1.xyz.height.masks.raw'
    toplayer_image_path = f'{smoothlayer_output_folder}\\{toplayer_image_name}'
    toplayer_image_meta_path = f'{smoothlayer_output_folder}\\{toplayer_image_meta_name}'
    toplayer_heightmap_path = f'{smoothlayer_output_folder}\\{toplayer_heightmap_name}'
    toplayer_heightmap_mask_path = f'{smoothlayer_output_folder}\\{toplayer_heightmap_mask_name}'
    level1_heightmap_path = f'{smoothlayer_output_folder}\\{level1_heightmap_name}'
    level1_heightmap_mask_path = f'{smoothlayer_output_folder}\\{level1_heightmap_mask_name}'

    api = voxelfarmclient.rest(cloud_url)
    
    basemeshes_asset_download_parent_folder = f'{qtree_assets_folder}\\BaseMeshes_Versions'
    basemeshes_asset_download_folder = f'{basemeshes_asset_download_parent_folder}\\{basemeshes_entity_id}'
    if run_update_basemeshes_assets:
        ##### Download BaseMeshes(version) assets from Cloud!
        file_list = api.get_file_list(project_id, basemeshes_entity_id)
        for index, file_name in enumerate(file_list):
            print(f"Index: {index}, File Path: {file_name}")
            file_data = api.get_file(project_id, basemeshes_entity_id, file_name)
            file_path = f'{basemeshes_asset_download_folder}\\{file_name}'
            save_data_to_file(file_data, file_path)
        ##### Copy BaseMeshes(version) assets to BaseMeshes asset folder!
        copy_files(basemeshes_asset_download_folder, qtree_assets_folder)
   
    dont_run_road_game = 1
    road_exe_command = f'{road_exe_path} {tiles_count} {tiles_x} {tiles_y} {road_Heightmap_width} {road_heightmap_height} {road_input_folder} {road_output_folder} {dont_run_road_game}'
    if run_road_exe:
        ##### Generate the road obj and image for smooth layer. 
        return_code_road = launch_process(road_exe_command)
        if return_code_road == 0:
            print(f'Process ({road_exe_command}) executed successfully.')
        else:
            print(f'Error: The process ({road_exe_command}) returned a non-zero exit code ({run_road_exe}).')
    
    worldgen_level = 5
    worldgen_command =  f'{worldgen_exe_path} {tiles_count} {tiles_x} {tiles_y} {worldgen_level} {qtree_assets_folder} {smoothlayer_output_base_folder} {road_output_folder}'
    if run_worldgen_road:
        ##### Generate the height map and image for smooth layer. 
        return_code_worldgen_road = launch_process(worldgen_command)
        if return_code_worldgen_road == 0:
            print(f'Process ({worldgen_command}) executed successfully.')
        else:
            print(f'Error: The process ({worldgen_command}) returned a non-zero exit code ({return_code_worldgen_road}).')
    
    basemeshvoxelizer1_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level1} {basemeshes_assets_folder} {basemeshes_db_base_folder} {basemeshes_cache_base_folder} {basemeshes_debug_level} {basemeshes_heightmap_folder}'
    basemeshvoxelizer0_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level0} {basemeshes_assets_folder} {basemeshes_db_base_folder} {basemeshes_cache_base_folder} {basemeshes_debug_level} {basemeshes_heightmap_folder}'
    if run_make_basemeshes:
        ##### Generate the height map from level 1 of BaseMeshes. 
        return_code_basemash1 = launch_process(basemeshvoxelizer1_command)
        if return_code_basemash1 == 0:
            print(f'Process ({basemeshvoxelizer1_command}) executed successfully.')
        else:
            print(f'Error: The process ({basemeshvoxelizer1_command}) returned a non-zero exit code ({return_code_basemash1}).')
        ##### Generate the height map from level 0 of BaseMeshes.  
        return_code_basemash0 = launch_process(basemeshvoxelizer0_command)
        if return_code_basemash0 == 0:
            print(f'Process ({basemeshvoxelizer0_command}) executed successfully.')
        else:
            print(f'Error: The process ({basemeshvoxelizer0_command}) returned a non-zero exit code ({return_code_basemash0}).')

    ##### Make ini config file for tree exe.
    #clear_all_sections(tree_ini_path)
    create_or_overwrite_empty_file(tree_ini_path)

    create_or_update_ini_file(tree_ini_path, section_tiles, 'Tiles_Count', tiles_count)
    create_or_update_ini_file(tree_ini_path, section_tiles, 'Tiles_X_Index', tiles_x)
    create_or_update_ini_file(tree_ini_path, section_tiles, 'Tiles_Y_Index', tiles_y)

    create_or_update_ini_file(tree_ini_path, section_input, 'Toplayer_Image', toplayer_image_path)
    create_or_update_ini_file(tree_ini_path, section_input, 'Toplayer_Image_Meta', toplayer_image_meta_path)
    create_or_update_ini_file(tree_ini_path, section_input, 'BaseMeshes_Level_0_HeightMap', basemeshes_0_heightmap_path)
    create_or_update_ini_file(tree_ini_path, section_input, 'BaseMeshes_Level_1_HeightMap', basemeshes_1_heightmap_path)
    create_or_update_ini_file(tree_ini_path, section_input, 'TopLayer_HeightMap', toplayer_heightmap_path)
    create_or_update_ini_file(tree_ini_path, section_input, 'Level1Layer_heightMap', level1_heightmap_path)
    create_or_update_ini_file(tree_ini_path, section_input, 'BaseMeshes_Level_0_HeightMap_Mask', basemeshes_0_heightmap_mask_path)
    create_or_update_ini_file(tree_ini_path, section_input, 'BaseMeshes_Level_1_HeightMap_Mask', basemeshes_1_heightmap_mask_path)
    create_or_update_ini_file(tree_ini_path, section_input, 'TopLayer_HeightMap_Mask', toplayer_heightmap_mask_path)
    create_or_update_ini_file(tree_ini_path, section_input, 'Level1Layer_heightMap_Mask', level1_heightmap_mask_path)
    create_or_update_ini_file(tree_ini_path, section_input, 'Most_Travelled_Points', most_travelled_points_path)
    create_or_update_ini_file(tree_ini_path, section_input, 'Most_Distant_Points', most_distant_points_path)

    create_or_update_ini_file(tree_ini_path, section_output, 'Output_Dir', tree_output_base_folder)
    create_or_update_ini_file(tree_ini_path, section_others, 'Lod', tree_lod)

    tree_exe_command = f'{tree_exe_path} {tree_ini_path}'
    if run_make_tree_instances:
        ##### Run tree exe to generate to tree instances.
        return_code_tree = launch_process(tree_exe_command)
        if return_code_tree == 0:
            print(f'Process ({tree_exe_command}) executed successfully.')
        else:
            print(f'Error: The process ({tree_exe_command}) returned a non-zero exit code ({return_code_tree}).')

    ##### Update the tree instance files of tree entity.
    workflow_api = workflow_lambda.workflow_lambda_host()
    tree_instance_output_folder = f'{tree_output_base_folder}\\{tiles_count}_{tiles_x}_{tiles_y}\\instanceoutput'
    if run_upload_tree_instances:
        update_attach_files_for_entity(api, project_id, tree_entity_id, tree_instance_output_folder, f'instances_lod8_{tiles_count}_{tiles_x}_{tiles_y}-{version}', version=version, color=True)
    return

def tree_config_creation(ini_path):
    road_input_folder = f'{Data_folder}\\RoadRawInit'
    road_exe_path = f'{Tools_folder}\\NPCTest2.exe'
    basemeshes_exe_path = f'{Tools_folder}\\BaseMeshVoxelizer.exe'
    worldgen_exe_path = f'{Tools_folder}\\WorldGen.exe'
    tree_exe_path = f'{Tools_folder}\PlantsSimulation.exe'
    qtree_assets_folder = Data_folder

    road_output_folder =f'{Data_folder}\\\RoadObjInfo'
    smoothlayer_output_base_folder = f'{Data_folder}\\sommothlayer_output'
    basemeshes_db_base_folder =f'{Data_folder}\\db'
    basemeshes_cache_base_folder = f'{Data_folder}\\cache'
    basemeshes_heightmap_folder = f'{Data_folder}\\heightmap'
    tree_output_base_folder = f'{Data_folder}\\tree_output'

    create_or_overwrite_empty_file(ini_path)

    create_or_update_ini_file(ini_path, section_main, 'cloud_url', Cloud_url)
    create_or_update_ini_file(ini_path, section_main, 'project_id', Project_id)
    create_or_update_ini_file(ini_path, section_main, 'folder_id', Folder_id)
    create_or_update_ini_file(ini_path, section_main, 'tree_entity_id', Tree_entity_id)
    create_or_update_ini_file(ini_path, section_main, 'basemeshes_entity_id', Basemeshes_entity_id)

    create_or_update_ini_file(ini_path, section_tiles, 'tiles_count', Tiles_size)
    create_or_update_ini_file(ini_path, section_tiles, 'tiles_x', Tiles_x)
    create_or_update_ini_file(ini_path, section_tiles, 'tiles_y', Tiles_y)

    create_or_update_ini_file(ini_path, section_input, 'road_input_folder', road_input_folder)
    create_or_update_ini_file(ini_path, section_input, 'road_exe_path', road_exe_path)
    create_or_update_ini_file(ini_path, section_input, 'basemeshes_exe_path', basemeshes_exe_path)
    create_or_update_ini_file(ini_path, section_input, 'worldgen_exe_path', worldgen_exe_path)
    create_or_update_ini_file(ini_path, section_input, 'tree_exe_path', tree_exe_path)
    create_or_update_ini_file(ini_path, section_input, 'qtree_assets_folder', qtree_assets_folder)

    create_or_update_ini_file(ini_path, section_output, 'road_output_folder', road_output_folder)
    create_or_update_ini_file(ini_path, section_output, 'smoothlayer_output_base_folder', smoothlayer_output_base_folder)
    create_or_update_ini_file(ini_path, section_output, 'basemeshes_db_base_folder', basemeshes_db_base_folder)
    create_or_update_ini_file(ini_path, section_output, 'basemeshes_cache_base_folder', basemeshes_cache_base_folder)
    create_or_update_ini_file(ini_path, section_output, 'basemeshes_heightmap_folder', basemeshes_heightmap_folder)
    create_or_update_ini_file(ini_path, section_output, 'tree_output_base_folder', tree_output_base_folder)

    create_or_update_ini_file(ini_path, section_run, 'run_update_basemeshes_assets', True)
    create_or_update_ini_file(ini_path, section_run, 'run_update_basemeshes_assets', True)
    create_or_update_ini_file(ini_path, section_run, 'run_update_basemeshes_assets', True)
    create_or_update_ini_file(ini_path, section_run, 'run_update_basemeshes_assets', True)
    create_or_update_ini_file(ini_path, section_run, 'run_update_basemeshes_assets', True)
    create_or_update_ini_file(ini_path, section_run, 'run_update_basemeshes_assets', False)

    create_or_update_ini_file(ini_path, section_road, 'road_Heightmap_width', 300)
    create_or_update_ini_file(ini_path, section_road, 'road_heightmap_height', 300)

    create_or_update_ini_file(ini_path, section_others, 'basemeshes_debug_level', Basemeshes_debug_level)
    create_or_update_ini_file(ini_path, section_others, 'tree_lod', 8)
    return

start_time = time.time()

params = sys.argv[1:]

section_main = 'Main'
section_tiles = 'Tiles'
section_input = 'Input'
section_output = 'Output'
section_run = 'Run'
section_others = 'Others'
section_road = 'Road'

Data_folder = 'D:\Downloads\TreeCreation'
Tools_folder = 'D:\Downloads\TreeCreation'
Cloud_url = 'http://52.226.195.5/'
Project_id = '1D4CBBD1D957477E8CC3FF376FB87470'
Folder_id = '90F6348AD5D94FCEA85C7C1CD081CE97' 
#tree_entity_id = E0070AD37D4543FCB9E70D60AE47541D
Tree_entity_id = '3A3CFEBA226B4692A8719C78335470DD'  
Basemeshes_entity_id = '4A59F80631E745E39557D23CED145732'
Tiles_size = 10
Tiles_x = 8
Tiles_y = 5
Basemeshes_debug_level = 6
configfile_path = f'{Data_folder}\\TreeInstancesCreationConfig.ini'
#configfile_path = params[0]
print(f'Tree instance generation config file : {configfile_path}')

tree_config_creation(configfile_path)
tree_instances_generation(configfile_path)

end_time = time.time()

execution_time_seconds = end_time - start_time

# Convert seconds to days, hours, minutes, seconds, and milliseconds
milliseconds = int(execution_time_seconds * 1000)
seconds, milliseconds = divmod(milliseconds, 1000)
minutes, seconds = divmod(seconds, 60)
hours, minutes = divmod(minutes, 60)
days, hours = divmod(hours, 24)

# Format the execution time
formatted_time = "{:02}:{:02}:{:02}:{:02}:{:03}".format(days, hours, minutes, seconds, milliseconds)

print("Whole Execution time :", formatted_time)