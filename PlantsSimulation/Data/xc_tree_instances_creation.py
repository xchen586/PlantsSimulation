import os
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

def create_or_update_ini_file(file_path, section, key, value):
    # Check if the INI file exists
    if not os.path.exists(file_path):
        # If not, create the INI file
        with open(file_path, 'w') as configfile:
            configfile.write('')
    
    # Read the existing INI file
    config = configparser.ConfigParser()

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
qtree_assert_folder = 'D:\\Downloads\\ProcgrenAssets'
basemeshes_heightmap_folder = qtree_assert_folder
smoothlayer_output_parent_folder = f'{qtree_assert_folder}\\output'
tree_ini_path = 'D:\\Downloads\\PlantsSimulation\\TreesInstancesAbsolutePathWin.ini'
tree_output_parent_folder = 'D:\\Downloads\\PlantsSimulation\\output'

basemeshes_exe_name = "BaseMeshVoxelizer.exe"
basemeshes_exe_path = f'{basemeshes_exe_folder}\\{basemeshes_exe_name}' 

tree_exe_name = 'PlantsSimulation.exe'
tree_exe_path = f'{tree_exe_folder}\\{tree_exe_name}'

most_travelled_points_name = 'Most Travelled Points.csv'
most_distant_points_name = 'Most Distant Points.csv'
most_travelled_points_path = f'{road_output_folder}\\{most_travelled_points_name}'
most_distant_points_path = f'{road_output_folder}\\{most_distant_points_name}'

basemeshes_asset_download_parent_folder = f'{qtree_assert_folder}\\BaseMeshes_Versions'
basemeshes_asset_download_folder = f'{basemeshes_asset_download_parent_folder}\\{basemeshes_entity_id}'

basemeshes_0_heightmap_name = f'{basemeshes_heightmap_folder}\\{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level0}_heightarray.bin'
basemeshes_1_heightmap_name = f'{basemeshes_heightmap_folder}\\{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level1}_heightarray.bin'
basemeshes_0_heightmap_mask_name = f'{basemeshes_heightmap_folder}\\{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level0}_heightmasks.bin'
basemeshes_1_heightmap_mask_name = f'{basemeshes_heightmap_folder}\\{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level1}_heightmasks.bin'
basemeshes_0_heightmap_path = f'{basemeshes_heightmap_folder}\\{basemeshes_0_heightmap_name}'
basemeshes_1_heightmap_path = f'{basemeshes_heightmap_folder}\\{basemeshes_1_heightmap_name}'
basemeshes_0_heightmap_mask_path = f'{basemeshes_heightmap_folder}\\{basemeshes_0_heightmap_mask_name}'
basemeshes_1_heightmap_mask_path = f'{basemeshes_heightmap_folder}\\{basemeshes_0_heightmap_mask_name}'

smoothlayer_output_folder = f'{smoothlayer_output_parent_folder}\\{tiles_count}_{tiles_x}_{tiles_y}'
toplayer_image_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.jpg'
toplayer_image_meta_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.jpw'
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

file_list = api.get_file_list(project_id, basemeshes_entity_id)
file_test = api.get_file(project_id, basemeshes_entity_id, file_list[0])
'''
lambda_host = process_lambda.process_lambda_host()
data_path = lambda_host.download_entity_files(basemeshes_entity_id)
'''

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



#update_attach_files_for_entity(api, project_id, tree_entity_id, tree_instance_output_folder, f'instances_lod8_{tiles_count}_{tiles_x}_{tiles_y}-{version}', version=version, color=True)


