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

import pandas as pd

from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
from voxelfarm import process_lambda

def xc_process_files_entity(api : voxelfarmclient.rest, project_id, folder_id, raw_entity_type, entity_type, folder_path, name : str, version : int, color : bool):

    result = api.creation_result()
    if not os.path.exists(folder_path):
        print(f'File {folder_path} does not exist')
        result.success = False
        result.error_info = f'File {folder_path} does not exist'
        return result
    
    # Use the os.listdir() function to get a list of filenames in the folder
    file_names = os.listdir(folder_path)

    # Create a list of file paths by joining the folder path with each file name
    file_paths = [os.path.join(folder_path, file_name) for file_name in file_names]   
    print(file_paths) 
    delimiter = ' '  # You can specify any delimiter you want, e.g., ',' or '-'
    # Using join() method
    file_paths_string = delimiter.join(file_paths)
    print(file_paths_string)

    result = api.get_project_crs(project_id)
    crs = result.crs
    if not result.success:
        return result
    
    result = api.create_entity_raw(project=project_id, 
        type = raw_entity_type,
        name=f'{name}', 
        fields={
            'file_folder': folder_id,
        }, crs = crs)
    entity_id = result.id
    if not result.success:
        return result

    print(f'Attaching file {file_paths} to entity {entity_id}')
    #api.attach_files(project=project_id, id=entity_id, files={'file': file})
    for file_path in file_paths:
        with open(file_path, "rb") as file:
            api.attach_files(project=project_id, id=entity_id, files={'file': file})
            print(f'attach file {file_path} for entity {entity_id}')

    if raw_entity_type == entity_type:
        return result

    result = api.create_entity_processed(project=project_id, 
        type = entity_type,
        name=f'{name}', 
        fields={
            'source': entity_id,
            'source_type': raw_entity_type,
            'file_folder': folder_id,
            #'source_ortho' if color else '_source_ortho': entity_id
        }, crs = crs)
    if not result.success:
        print(f'--------Fail to create entity {result.id} for {name} {version}--------')
        return result
    print(f'--------Created entity {result.id} for {name} {version}--------')
    return result

#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def create_basemeshes_result_entity(api : voxelfarmclient.rest, basemeshes_output_folder_path):
    basemeshes_result_project_id = Project_id
    basemeshes_result_folder_id = '3A18892690F940759590B782AA80FC13'
    level0_output_folder = os.path.join(basemeshes_output_folder_path, f'{tile_size}_{tile_x}_{tile_y}_0')
    level1_output_folder = os.path.join(basemeshes_output_folder_path, f'{tile_size}_{tile_x}_{tile_y}_1')
    project_entity = api.get_entity(basemeshes_result_project_id)
    version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
    level0_entity_name = f'Workflow_Basemeshes_{tile_size}_{tile_x}_{tile_y}_0-ver-{version}'
    level1_entity_name = f'Workflow_Basemeshes_{tile_size}_{tile_x}_{tile_y}_1-ver-{version}'

    print(f'basemeshes_result_project_id :  {basemeshes_result_project_id}')
    print(f'basemeshes_result_folder_id :  {basemeshes_result_folder_id}')
    print(f'level0_output_folder :  {level0_output_folder}')
    print(f'level1_output_folder :  {level1_output_folder}')
    print(f'version :  {version}')
    print(f'level0_entity_name :  {level0_entity_name}')
    print(f'level0_entity_name :  {level0_entity_name}')

    basemeshes_result_project_entity = api.get_entity(basemeshes_result_project_id)
    version = int(basemeshes_result_project_entity['version']) + 1 if 'version' in basemeshes_result_project_entity else 1
    api.update_entity(project=basemeshes_result_project_id, id=basemeshes_result_project_id, fields={'version': version})
    result = api.create_folder(project=basemeshes_result_project_id, name=f'Version {version}', folder=basemeshes_result_folder_id)
    if not result.success:
        print(f'Failed to create basemeshes workflow result folder for version!')
        exit(4)
    basemeshes_result_version_folder_id = result.id
    print(f'-----------------Successful to create basemeshes workflow folder {basemeshes_result_version_folder_id} for version {version}!-----------------')

    print('Start with create basemeshes workflow level 0 entity')
    result = xc_process_files_entity(api, basemeshes_result_project_id, basemeshes_result_version_folder_id, api.entity_type.RawMesh, api.entity_type.RawMesh, level0_output_folder, level0_entity_name, version=version, color=True)
    if not result.success:
        print(f'Failed to create basemeshes workflow result {level0_entity_name} with {api} basemeshes_result_project_id: {basemeshes_result_project_id} level0_output_folder: {level0_output_folder} raw: api.entity_type.RawMesh index: api.entity_type.IndexedMesh version: {version} !')
        exit(4)
    print('End with create basemeshes workflow level 0 entity')

    print('Start with create basemeshes workflow level 1 entity')
    result = xc_process_files_entity(api, basemeshes_result_project_id, basemeshes_result_version_folder_id, api.entity_type.RawMesh, api.entity_type.RawMesh, level1_output_folder, level1_entity_name, version=version, color=True)
    if not result.success:
        print(f'Failed to create basemeshes workflow result {level1_entity_name} with {api} basemeshes_result_project_id: {basemeshes_result_project_id} level1_output_folder: {level1_output_folder} raw: api.entity_type.RawMesh index: api.entity_type.IndexedMesh version: {version} !')
        exit(4)
    print('End with create basemeshes workflow level 1 entity')
    return

#---------------------------------------------------------------------------------------------------------------------------------------------------------------
    
    
tile_size = 10
tile_x = 8
tile_y = 5

Project_id = '1D4CBBD1D957477E8CC3FF376FB87470'
cloud_url = 'http://52.226.195.5/'
api = voxelfarmclient.rest(cloud_url)
basemeshes_output_folder = f'D:\Downloads\XCTreeCreation\cache'
create_basemeshes_result_entity(api, basemeshes_output_folder)
print(f'create_basemeshes_result_entity for {basemeshes_output_folder}')