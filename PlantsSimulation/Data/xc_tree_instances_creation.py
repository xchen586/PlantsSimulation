import os
import shutil
import subprocess
from zipfile import ZipFile
import glob
from timeit import default_timer as timer
from datetime import timedelta
from voxelfarm import voxelfarmclient
from os import path
import configparser
from voxelfarm import workflow_lambda
from voxelfarm import process_lambda
from distutils.dir_util import copy_tree

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
        # Read the value and convert it to the specified type
        return value_type(config.get(section, key))
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

api = voxelfarmclient.rest('http://52.226.195.5/')
workflow_api = workflow_lambda.workflow_lambda_host()

project_id = '1D4CBBD1D957477E8CC3FF376FB87470'
folder_id = '90F6348AD5D94FCEA85C7C1CD081CE97'
#tree_entity_id = 'E0070AD37D4543FCB9E70D60AE47541D' # cosmin new
#tree_entity_id = "536674D5E8D440D9A7EFCD1D879AD57A" # cosmin old
tree_entity_id = "3A3CFEBA226B4692A8719C78335470DD"  #xc tesst
basemeshes_entity_id = '543FB5F2F22341708A4B876907884ECB'
version = 80

tiles_count = 10
tiles_x = 8
tiles_y = 5
basemeshes_level0 = 0
basemeshes_level1 = 1
basemeshes_debug_level = 6
tree_lod = 8

basemeshes_exe_folder = 'D:\\xWork\\VoxelFarm\\Voxel-Farm-WorldGen\\WorldGen.vstudio\\x64\\Release'
tree_exe_folder = 'D:\\xWork\\VoxelFarm\\PlantsSimulation\\PlantsSimulation\\x64\\Release'
road_output_folder = 'D:\\xWork\\VoxelFarm\\ProcgenNPC\\NPCTest2\\bin\\Release\\net6.0'
qtree_assets_folder = 'D:\\Downloads\\ProcgrenAssets'
basemeshes_heightmap_folder = qtree_assets_folder
smoothlayer_output_parent_folder = f'{qtree_assets_folder}\\output'
tree_ini_path = 'D:\\Downloads\\PlantsSimulation\\TreesInstancesAbsolutePathWin.ini'
tree_output_parent_folder = 'D:\\Downloads\\PlantsSimulation\\output'
basemeshes_assets_folder = qtree_assets_folder
basemeshes_db_folder = 'D:\\Downloads\\ProcgrenAssets\\db'
basemeshes_cache_folder = 'D:\\Downloads\\ProcgrenAssets\\db'

basemeshes_exe_name = "BaseMeshVoxelizer.exe"
basemeshes_exe_path = f'{basemeshes_exe_folder}\\{basemeshes_exe_name}' 

tree_exe_name = 'PlantsSimulation.exe'
tree_exe_path = f'{tree_exe_folder}\\{tree_exe_name}'

most_travelled_points_name = 'Most Travelled Points.csv'
most_distant_points_name = 'Most Distant Points.csv'
most_travelled_points_path = f'{road_output_folder}\\{most_travelled_points_name}'
most_distant_points_path = f'{road_output_folder}\\{most_distant_points_name}'

basemeshes_asset_download_parent_folder = f'{qtree_assets_folder}\\BaseMeshes_Versions'
basemeshes_asset_download_folder = f'{basemeshes_asset_download_parent_folder}\\{basemeshes_entity_id}'

basemeshes_0_heightmap_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level0}_heightarray.bin'
basemeshes_1_heightmap_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level1}_heightarray.bin'
basemeshes_0_heightmap_mask_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level0}_heightmasks.bin'
basemeshes_1_heightmap_mask_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level1}_heightmasks.bin'
basemeshes_0_heightmap_path = f'{basemeshes_heightmap_folder}\\{basemeshes_0_heightmap_name}'
basemeshes_1_heightmap_path = f'{basemeshes_heightmap_folder}\\{basemeshes_1_heightmap_name}'
basemeshes_0_heightmap_mask_path = f'{basemeshes_heightmap_folder}\\{basemeshes_0_heightmap_mask_name}'
basemeshes_1_heightmap_mask_path = f'{basemeshes_heightmap_folder}\\{basemeshes_0_heightmap_mask_name}'

smoothlayer_output_folder = f'{smoothlayer_output_parent_folder}\\{tiles_count}_{tiles_x}_{tiles_y}'
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

tree_instance_output_folder = f'{tree_output_parent_folder}\\{tiles_count}_{tiles_x}_{tiles_y}\\instanceoutput'

section_tiles = 'Tiles'
section_input = 'Input'
section_output = 'Output'
section_others = 'Others'

##### Download BaseMeshes(version) assets from Cloud!
file_list = api.get_file_list(project_id, basemeshes_entity_id)
for index, file_name in enumerate(file_list):
    print(f"Index: {index}, File Path: {file_name}")
    file_data = api.get_file(project_id, basemeshes_entity_id, file_name)
    file_path = f'{basemeshes_asset_download_folder}\\{file_name}'
    save_data_to_file(file_data, file_path)

##### Copy BaseMeshes(version) assets to BaseMeshes asset folder!
copy_files(basemeshes_asset_download_folder, qtree_assets_folder)

##### Generate the height map from level 1 of BaseMeshes.  
basemeshvoxelizer1_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level1} {basemeshes_assets_folder} {basemeshes_db_folder} {basemeshes_cache_folder} {basemeshes_debug_level}'
return_code_basemash1 = launch_process(basemeshvoxelizer1_command)
if return_code_basemash1 == 0:
    print("Process ({basemeshvoxelizer1_command}) executed successfully.")
else:
    print(f"Error: The process ({basemeshvoxelizer1_command}) returned a non-zero exit code ({return_code_basemash1}).")

##### Generate the height map from level 0 of BaseMeshes.  
basemeshvoxelizer0_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level0} {basemeshes_assets_folder} {basemeshes_db_folder} {basemeshes_cache_folder} {basemeshes_debug_level}'
return_code_basemash0 = launch_process(basemeshvoxelizer0_command)
if return_code_basemash0 == 0:
    print("Process ({basemeshvoxelizer0_command}) executed successfully.")
else:
    print(f"Error: The process ({basemeshvoxelizer0_command}) returned a non-zero exit code ({return_code_basemash0}).")

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

create_or_update_ini_file(tree_ini_path, section_output, 'Output_Dir', tree_output_parent_folder)
create_or_update_ini_file(tree_ini_path, section_others, 'Lod', tree_lod)

##### Run tree exe to generate to tree instances.
tree_exe_command = f'{tree_exe_path} {tree_ini_path}'
return_code_tree = launch_process(tree_exe_command)
if return_code_tree == 0:
    print("Process ({tree_exe_command}) executed successfully.")
else:
    print(f"Error: The process ({tree_exe_command}) returned a non-zero exit code ({return_code_tree}).")

##### Update the tree instance files of tree entity.
update_attach_files_for_entity(api, project_id, tree_entity_id, tree_instance_output_folder, f'instances_lod8_{tiles_count}_{tiles_x}_{tiles_y}-{version}', version=version, color=True)


