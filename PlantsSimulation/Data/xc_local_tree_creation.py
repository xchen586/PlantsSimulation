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
        
def find_files_of_type_in_folder(folder_path, file_extension):
    files = []
    # Recursively traverse through all subdirectories
    for root, _, files_list in os.walk(folder_path):
        # Use glob.glob to find files matching the pattern in each directory
        files.extend(glob.glob(os.path.join(root, f"*.{file_extension}")))
    
    return files

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

def zip_folder(folder_path, zip_path):
    try:
        if os.path.exists(zip_path):
            os.remove(zip_path)  # Remove existing zip file
            
        with ZipFile(zip_path, 'w') as zipf:
            for root, dirs, files in os.walk(folder_path):
                for file in files:
                    file_path = os.path.join(root, file)
                    zipf.write(file_path, os.path.relpath(file_path, folder_path))
        return True
    except Exception as e:
        print("An error occurred:", e)
        return False

def print_file_paths(folder_path):
    try:
        # Loop through all files and directories in the specified folder
        for file_name in os.listdir(folder_path):
            file_path = os.path.join(folder_path, file_name)
            if os.path.isfile(file_path):  # Check if it is a file
                print(file_path)  # Print the full file path
    except FileNotFoundError:
        print(f"Folder '{folder_path}' not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

def copy_files(src_folder, dest_folder):
    print(f"copy_files Start to print file in {src_folder} !")
    print_file_paths(src_folder)
    print(f"copy_files End to print file in {src_folder} !") 
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
        
def copy_files_in_folder(source_folder, destination_folder):
    """
    Copies files that are directly in the source folder (not in subfolders) to the destination folder.

    Parameters:
    source_folder (str): Path to the source folder containing the files.
    destination_folder (str): Path to the destination folder where files will be copied.
    """
    print(f"copy_files_in_folder Start to print file in {source_folder} !")
    print_file_paths(source_folder)
    print(f"copy_files_in_folder End to print file in {source_folder} !") 
    # Make sure the destination folder exists
    if not os.path.exists(destination_folder):
        os.makedirs(destination_folder)

    # Iterate through items in the source folder
    for item in os.listdir(source_folder):
        item_path = os.path.join(source_folder, item)

        # Check if the item is a file (not a directory)
        if os.path.isfile(item_path):
            # Copy the file to the destination folder
            shutil.copy(item_path, destination_folder)
            print(f"Copied: {item_path} to {destination_folder}")


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
    
# Create a custom ConfigParser that preserves case sensitivity
class CaseSensitiveConfigParser(configparser.ConfigParser):
    def optionxform(self, optionstr):
        return optionstr  # Preserve the original case of the key
    
def ini_file_to_string(file_path):
    # Create a ConfigParser object
    
    #ConfigParser that lowercase
    #config = configparser.ConfigParser()

    #ConfigParser that preserves case
    config = CaseSensitiveConfigParser()
    
    # Read the .ini file
    config.read(file_path)

    # Use a StringIO object to get the content as a string
    with StringIO() as output:
        config.write(output)
        ini_string = output.getvalue()

    return ini_string
    
def create_or_update_ini_file(file_path, section, key, value):
    # Check if the INI file exists
    if not os.path.exists(file_path):
        # If not, create the INI file
        with open(file_path, 'w') as configfile:
            configfile.write('')
            
    # Read the existing INI file
    
    #ConfigParser that lowercase
    #config = configparser.ConfigParser()
    # Make it not only uppercase
    #config.optionxform = str
    
    #ConfigParser that preserves case
    config = CaseSensitiveConfigParser()
    
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
    
    #ConfigParser that lowercase
    #config = configparser.ConfigParser()

    #ConfigParser that preserves case
    config = CaseSensitiveConfigParser()
    
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

def update_attach_files_for_entity(api : voxelfarmclient.rest, project_id, entity_id, folder_path):

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

def is_valid_float_string(s):
    try:
        float(s)
        return True
    except ValueError:
        return False
    
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

#---------------------------------------------------------------------------------------------------------------------------------------------------------------

TREE_INSTANCE = 0
SPAWN_INSTANCE = 1
NPC_INSTANCE = 2
RESOURCE_INSTANCE = 3
TREE_LEVEL1_INSTANCE = 4
InstanceType_Attribute = 'InstanceType'
Variant_Attribute = 'Variant'
Index_Attribute = 'Index'

def calculate_id_for_instance(instance_type, tree_index, spawn_index, npc_index, resource_index, tree_level1_index):
    # Calculate the extra column value based on the instance type and indices
    instance_string = 'Others'
    index = 0

    if instance_type == TREE_INSTANCE:
        instance_string = 'Tree'
        index = tree_index
    elif instance_type == SPAWN_INSTANCE:
        instance_string = 'Poi_Spawn'
        index = spawn_index
    elif instance_type == NPC_INSTANCE:
        instance_string = 'Poi_NPC'
        index = npc_index
    elif instance_type == RESOURCE_INSTANCE:
        instance_string = 'Poi_Resource'
        index = resource_index
    elif instance_type == TREE_LEVEL1_INSTANCE:
        instance_string = 'Tree_Level1'
        index = tree_level1_index
    
    extra_value = f'{instance_string} {index}'
    return extra_value

'''
    # Assign unique IDs to corresponding rows and update index variables
    def update_id(row):
        nonlocal current_tree_id, current_poi_id
        instance_type = row[InstanceType_Attribute]
        if instance_type == TREE_INSTANCE:
            extra_id = current_tree_id
            current_tree_id += 1
        elif instance_type == POI_INSTANCE:
            extra_id = current_poi_id
            current_poi_id += 1
        else:
            extra_id = 0
        return calculate_id_for_instance(instance_type, extra_id, extra_id)
'''

def add_extra_column_to_csv(input_file, output_file, extra_column_name):
    # Read the CSV file
    merged_df = pd.read_csv(input_file)

    # Initialize index variables
    current_tree_id = 1
    current_poi_id = 1

    # Assign unique IDs to corresponding rows and update index variables
    def update_id(row):
        nonlocal current_tree_id, current_poi_id
        instance_type = row[InstanceType_Attribute]
        instance_index = row[Index_Attribute]
        extra_id = instance_index
        return calculate_id_for_instance(instance_type, extra_id, extra_id, extra_id, extra_id, extra_id)
    
    merged_df[extra_column_name] = merged_df.apply(update_id, axis=1)

    # Write the updated DataFrame to a new CSV file
    merged_df.to_csv(output_file, index=False)

def xc_process_files_entity(api : voxelfarmclient.rest, project_id, folder_id, raw_entity_type, entity_type, folder_path, name : str, color : bool, zipped:bool = False):

    print(f'start to result = api.creation_result()')
    result = api.creation_result()
    print(f'end to result = api.creation_result()')

    print(f'start to os.path.exists({folder_path})')
    if not os.path.exists(folder_path):
        print(f'File {folder_path} does not exist')
        result.success = False
        result.error_info = f'File {folder_path} does not exist'
        return result
    print(f'end to os.path.exists({folder_path})')

    # Use the os.listdir() function to get a list of filenames in the folder
    print(f'start to os.listdir({folder_path})')
    file_names = os.listdir(folder_path)
    print(f'end to os.listdir({folder_path})')

    # Create a list of file paths by joining the folder path with each file name
    file_paths = [os.path.join(folder_path, file_name) for file_name in file_names]   
    delimiter = ' '  # You can specify any delimiter you want, e.g., ',' or '-'
    # Using join() method
    file_paths_string = delimiter.join(file_paths)
    print(file_paths_string)

    print(f'start to api.get_project_crs({project_id})')
    result = api.get_project_crs(project_id)
    crs = result.crs
    if not result.success:
        return result
    print(f'end to api.get_project_crs({project_id})')
    
    print(f'start to api.create_entity_raw')
    result = api.create_entity_raw(project=project_id, 
        type = raw_entity_type,
        name=f'{name}_src', 
        fields={
            'file_folder': folder_id,
        }, crs = crs)
    entity_id = result.id
    if not result.success:
        return result
    print(f'end to api.create_entity_raw')

    print(f'Attaching file {file_paths} to entity {entity_id}')
    if zipped:
        zip_path = f'{folder_path}_zip.zip'
        print(f'start to zip folder {folder_path} to entity {zip_path}')
        canzip = zip_folder(folder_path, zip_path)
        print(f'end to zip folder {folder_path} to entity {zip_path}')
        if canzip:
            with open(zip_path, 'rb') as zfile:
                try:
                    #result = api.attach_files(project=project_id, id=entity_id, files={'file': (zip_path, zfile, 'application/zip')})
                    result = api.attach_files(project=project_id, id=entity_id, files={'file': zfile})
                except Exception as e:
                    print("An error occurred:", e)
                if not result.success:
                    print(result.error_info)
                    return result
            print(f'attach zip file {zip_path} for entity {entity_id}')
        else:
            result.success = False
            result.error_info = f'Fail to zip {folder_path} as file'
            return result
    else:
        for file_path in file_paths:
            with open(file_path, "rb") as file:
                result = api.attach_files(project=project_id, id=entity_id, files={'file': file})
                if not result.success:
                    print(result.error_info)
                    return result
                print(f'attach file {file_path} for entity {entity_id}')

    
    #api.attach_files(project=project_id, id=entity_id, files={'file': file})
    for file_path in file_paths:
        with open(file_path, "rb") as file:
            api.attach_files(project=project_id, id=entity_id, files={'file': file})
            print(f'attach file {file_path} for entity {entity_id}')

    print(f'start to only create the raw entity {entity_id}')
    if raw_entity_type == entity_type:
        print(f'only create the raw entity {entity_id}')
        return result
    print(f'start to only create the raw entity {entity_id}')

    print(f'start to api.create_entity_processed')
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
        print(f'--------Fail to create entity {result.id} for {name} --------')
        return result
    print(f'end to api.create_entity_processed')

    print(f'--------Created entity {result.id} for {name} --------')
    return result

def create_geochem_tree_entity(api, project_id, folder_id, geo_chemical_folder, tiles_size, tiles_x, tiles_y, level, version : int):
    extra_column_name = 'Id'
    geochems_project_id = project_id
    geochems_folder_id = folder_id
    
    entity_basename = f'GeoChemical_instances_{tiles_size}_{tiles_x}_{tiles_y}_{level}'

    merged_csv_name = f'{tiles_size}_{tiles_x}_{tiles_y}_{level}_geo_merged.csv'
    merged_csv_path = os.path.join(geo_chemical_folder, merged_csv_name)
    geo_meta_name = 'process.meta'
    geo_meta_path = os.path.join(geo_chemical_folder, geo_meta_name)

    print('Start to Add Id field to  the csv file {merged_csv_path}')
    add_extra_column_to_csv(merged_csv_path, merged_csv_path, extra_column_name)
    print('End with raw data file {merged_csv_path}')

    print('Start with geochem meta file {geo_meta_path}')

    create_or_overwrite_empty_file(geo_meta_path)
    create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile', merged_csv_name)
   #create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile_ID', 5)
    create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile_ID', 6)
    create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile_X', 0)
    create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile_Y', 1)
    create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile_Z', 2)
    create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile_Attribute_Columns', 2)
    create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile_Attribute_Column0_Index', 3)
    create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile_Attribute_Column0_Name', InstanceType_Attribute)
    create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile_Attribute_Column0_Type', 0)
    create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile_Attribute_Column1_Index', 4)
    create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile_Attribute_Column1_Name', Variant_Attribute)
    create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile_Attribute_Column1_Type', 0)
    geo_meta_string = ini_file_to_string(geo_meta_path)
    print(f'Geo meta file content is :')
    print(f'{geo_meta_string}')

    '''
    geochems_project_entity = api.get_entity(geochems_project_id)
    version = int(geochems_project_entity['version']) + 1 if 'version' in geochems_project_entity else 1
    api.update_entity(project=geochems_project_id, id=geochems_project_id, fields={'version': version})
    result = api.create_folder(project=geochems_project_id, name=f'GeoChem Version {version}', folder=geochems_folder_id)
    if not result.success:
        print(f'Failed to create geochem folder for version!')
        exit(4)
    geochems_folder_id = result.id
    print(f'-----------------Successful to create geochem folder {geochems_folder_id} for version!-----------------')
    '''

    print('Start with create geo chem entity')

    geochem_entity_name = f'{entity_basename}-{version}'
    result = xc_process_files_entity(api, geochems_project_id, geochems_folder_id, api.entity_type.RawGeoChem, api.entity_type.GeoChem, geo_chemical_folder, geochem_entity_name, color=True)
    if not result.success:
        print(f'Failed to create geochem entity {geochem_entity_name} with {api} basemeshes_result_project_id: {geochems_project_id} geo_chemical_folder: {geo_chemical_folder} raw: api.entity_type.RawGeoChem index: api.entity_type.GeoChem version: {version} !')
        exit(4)
    print('End with create geo chem entity')

#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def merge_instances_csv_files(folder_a, folder_b, destination_folder):
    # Ensure destination folder exists and clear it
    print(f'start to merge_instances_csv_files from folder_a : {folder_a} and folder_b : {folder_b} to destination_folder : {destination_folder}')
    if os.path.exists(destination_folder):
        shutil.rmtree(destination_folder)
    os.makedirs(destination_folder, exist_ok=True)
    
    files_a = set(os.listdir(folder_a))
    files_b = set(os.listdir(folder_b))
    
    all_files = files_a | files_b
    
    for file_name in all_files:
        path_a = os.path.join(folder_a, file_name)
        path_b = os.path.join(folder_b, file_name)
        dest_path = os.path.join(destination_folder, file_name)
        
        if file_name in files_a and file_name in files_b:
            # Merge files
            with open(path_a, 'r', newline='', encoding='utf-8') as file_a:
                reader_a = file_a.readlines()
            
            with open(path_b, 'r', newline='', encoding='utf-8') as file_b:
                reader_b = file_b.readlines()
            
            # Write merged content
            with open(dest_path, 'w', newline='', encoding='utf-8') as dest_file:
                dest_file.writelines(reader_a)  # Write all of file A
                dest_file.writelines(reader_b[1:])  # Append file B (skip header)
        
        elif file_name in files_a:
            shutil.copy(path_a, dest_path)
        
        elif file_name in files_b:
            shutil.copy(path_b, dest_path)
    
    print("merge_instances_csv_files completed successfully.")

#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def process_file_image(api : voxelfarmclient.rest, project_id, folder_id, file_path, jgw_path : str, name : str, version : int):

    print(f'process_file_image project id = {project_id}')
    print(f'process_file_image parent folder id = {folder_id}')
    print(f'process_file_image image file path = {file_path}')
    print(f'process_file_image image meta path = {jgw_path}')   
     
    if not os.path.exists(file_path):
        print(f'Image File {file_path} does not exist')
        return
    if not os.path.exists(jgw_path):
        print(f'Image Meta File {jgw_path} does not exist')
        return
    
    result = api.get_project_crs(project_id)
    crs = result.crs

    '''
    project_entity = api.get_entity(project_id)
    version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
    api.update_entity(project=project_id, id=project_id, fields={'version': version})

    result = api.create_folder(project=project_id, name=f'Road Image Version {version}', folder=folder_id)
    if not result.success:
        print(f'Failed to create image file folder for version!')
        return 
    entity_folder_id = result.id
    print(f'Successful to create image file folder {entity_folder_id} for version!')
    '''

    entity_folder_id = folder_id
    result = api.create_entity_raw(project=project_id, 
            type=api.entity_type.IndexedOrthoImagery, 
            name=f'{name}-{version}_src', 
            fields={
                'file_folder': entity_folder_id,
            }, crs = crs)
    entity_id = result.id
    
    with open(jgw_path, 'rb') as j:
        api.attach_files(project=project_id, id=entity_id, files={'file': j})
    print(f'Attaching file {jgw_path} to entity {entity_id}')
    with open(file_path, 'rb') as f:
        api.attach_files(project=project_id, id=entity_id, files={'file': f})
    print(f'Attaching file {file_path} to entity {entity_id}')

    result = api.create_entity_processed(project=project_id, 
        type=api.entity_type.IndexedOrthoImagery, 
        name=f'{name}-{version}', 
        fields={
            'source': entity_id,
            'source_type': 'ORTHO',
            'file_folder': entity_folder_id,
            'source_ortho': entity_id
        }, crs = crs)
    print(f'Created entity {result.id} for {name}-{version}')
    
#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def process_point_cloud(api : voxelfarmclient.rest, file_path_txt2las, project_id, folder_id, file_path,  entityType, entity_basename : str, version : int, color : bool):

    print(f'process_point_cloud project id = {project_id}')
    print(f'process_point_cloud parent folder id = {folder_id}')
    print(f'process_point_cloud point cloud file path = {file_path}') 
    print(f'process_point_cloud txt2las file path = {file_path_txt2las}') 
    
    if not os.path.exists(file_path):
        print(f'Point cloud file {file_path} does not exist')
        return
    if not os.path.exists(file_path_txt2las):
        print(f'Tool txt2las File {file_path_txt2las} does not exist')
        return

    # Run txt2las to convert the txt file to a laz file
    file_path_laz = file_path + '.laz'
    subprocess.run([
        #'C:\\Work\\SDK\\Voxel-Farm-Examples\\Python\\txt2las.exe', '-i', file_path, '-o', file_path_laz,  
        file_path_txt2las, '-i', file_path, '-o', file_path_laz,  
        '-set_version', '1.4', 
        '-set_system_identifier', 'Pangea Next Procgren', 
        '-set_generating_software', 'Pangea Next Procgren',
        '-parse', 'xyzRGB' if color else 'xyz',], 
        stdout=subprocess.PIPE)

    result = api.get_project_crs(project_id)
    crs = result.crs
    
    '''
    project_entity = api.get_entity(project_id)
    version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
    api.update_entity(project=project_id, id=project_id, fields={'version': version})
    
    result = api.create_folder(project=project_id, name=f'Smooth Layer Version {version}', folder=folder_id)
    if not result.success:
        print(f'Failed to Smooth Layer file folder for version {version}!')
        return 
    entity_folder_id = result.id
    print(f'Successful to Smooth Layer file folder {entity_folder_id} for version {version}!')
    '''
    entity_folder_id = folder_id
    result = api.create_entity_raw(project=project_id, 
        type=api.entity_type.RawPointCloud, 
        name=f'{entity_basename}-{entityType}-{version}_src', 
        fields={
            'file_folder': entity_folder_id,
        }, crs = crs)
    entity_id = result.id

    with open(file_path_laz, 'rb') as f:
        api.attach_files(project=project_id, id=entity_id, files={'file': f})
    print(f'Attaching file {file_path_laz} to entity {entity_id}')
    
    result = api.create_entity_processed(project=project_id, 
        #type=api.entity_type.VoxelTerrain, 
        #type=api.entity_type.IndexedPointCloud,
        type = entityType,
        name=f'{entity_basename}-{entityType}-{version}', 
        fields={
            'source': entity_id,
            'source_type': 'RAWPC',
            'file_folder': entity_folder_id,
            #'source_ortho' if color else '_source_ortho': entity_id
        }, crs = crs)
    print(f'Created entity {result.id} for {entity_basename}-{entityType}-{version}')

#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def on_upload_db_succeessfull(vf, project_id, entity_id, output_dir):
    # save entity properties and complete state
    properties_file = f'{output_dir}/properties.vf'

    if path.exists(properties_file):
        entity_data= {}
        config = configparser.ConfigParser()
        config.read(properties_file)
        print('XC Entity Properties')

        section = config.sections()[0]

        for key in config[section]:  
            entity_data[key]=config[section][key]
            print(f'  {key}={entity_data[key]}')

        # update entity
        result = vf.update_entity(
            id=entity_id,
            project=project_id, 
            fields=entity_data)

    unified_file = f'{output_dir}/unified.vf'
    if path.exists(unified_file):
        print("XC extended.meta File is found")
        #lambda_host.upload(unified_file, "extended.meta", entity_id)
    else:
        print("XC extended.meta File not found")
        
#---------------------------------------------------------------------------------------------------------------------------------------------------------------
'''
def do_simple_upload_basemeshes(api : voxelfarmclient.rest, project_id, basemeshes_db_folderId, file_path : str, version : int, entity_name : str, code_path : str):
    print(f'Start do_simple_upload_basemeshes Created entity {entity_name}')

    result = api.get_project_crs(project_id)
    crs = result.crs
    entity_id = None

    print(f'start create_entity_raw file for entity {entity_name}')
    result = api.create_entity_raw(project=project_id, 
        type=api.entity_type.VoxelPC, 
        name=entity_name, 
        fields={
            'state': 'PARTIAL',
            'file_folder': basemeshes_db_folderId,
            'virtual': '0',
        }, crs = crs)
    entity_id = result.id
    print(f'end create_entity_raw file for entity {entity_name}')
    if not result.success:
        print(f'Fail to create_entity_raw Created entity for {entity_name} : {result.error_info}')
    else:
        print(f'Successfully to create_entity_raw Created entity for {result.id} for {entity_name}')
        
    #dbName = f'vox-mesh-{entity_name}'
    #dbTitle = f'Voxel Mesh Data For {entity_name}'
    dbName = f'vox-pc'
    dbTitle = f'Voxel Data'
    try:
        print(f'Start lambda_host.upload_db({entity_id}, {file_path}, {dbName}, {dbTitle})')
        uploadDbOk = lambda_host.upload_db(entity_id, file_path, dbName, dbTitle)
        print(f'End lambda_host.upload_db({entity_id}, {file_path}, {dbName}, {dbTitle})')
    except Exception as e:
        print(f'Exception of lambda_host.upload_db: files folder: {file_path} to entity {entity_id} with exception of {str(e)}')   
    if uploadDbOk:
        print(f'lambda_host.upload_db is successful in do_simple_upload_basemeshes with {file_path} to entity {entity_id}')
        on_upload_db_succeessfull(api, project_id, entity_id, file_path)
        result = api.update_entity(
        id = entity_id,
        project = project_id, 
        fields = {
            'state' : 'COMPLETE'
        })
        create_or_update_ini_file(g_Lambda_Info_ini_path, section_entity, entity_name, entity_id)
    else:
        print(f'lambda_host.upload_db is failed in do_simple_upload_basemeshes with {file_path} to entity {entity_id}')
        result = api.update_entity(
        id = entity_id,
        project = project_id, 
        fields = {
            'state' : 'ERROR'
        })

    if not result.success:
        print(result.error_info)
        exit(3)

    print(f'End do_simple_upload_basemeshes Created entity {result.id} for {entity_name}')
    
#---------------------------------------------------------------------------------------------------------------------------------------------------------------

def do_upload_base_meshes_swarm(api : voxelfarmclient.rest, project_id, basemeshes_db_folderId, file_path : str, version : int, entity_name : str, code_path : str):
    print(f'Start do_upload_base_meshes_swarm Created entity {entity_name}')

    result = api.get_project_crs(project_id)
    crs = result.crs
    entity_id = None
    index_vf_path = os.path.join(file_path, 'index.vf')
    with open(index_vf_path, 'rb') as f:
        print(f'start create_entity_raw file for entity {entity_name}')
        result = api.create_entity_raw(project=project_id, 
            type=api.entity_type.VoxelPC, 
            name=entity_name, 
            fields={
                'state': 'PARTIAL',
                'file_folder': basemeshes_db_folderId,
            }, crs = crs)
        entity_id = result.id
        if not result.success:
            print(f'Fail to create_entity_raw Created entity for {entity_name} : {result.error_info}')
        else:
            print(f'Successfully to create_entity_raw Created entity for {result.id} for {entity_name}')
        index_vf_size = os.path.getsize(index_vf_path)
        print(f'Attaching file index vf {index_vf_path} with size of {index_vf_size} to entity {entity_id}')
        try:
            result = api.attach_files(project=project_id, id=entity_id, files={'file': f})
        except Exception as e:
            exception_message = str(e)
            print(f'Exception of Attach_files index vf to 1 attach file {index_vf_path} to entity {entity_id} with exception of {exception_message}')
        if not result.success:
            print(f'Failed to 1 attach file {index_vf_path} to entity {entity_id} with error of {result.error_info}')
            #return

    data_vf_path = os.path.join(file_path, 'data.vf')
    with open(data_vf_path, 'rb') as f:
        data_vf_size = os.path.getsize(data_vf_path)
        print(f'Attaching file data vf {data_vf_path} with size of {data_vf_size} to entity {entity_id}')
        try: 
            result = api.attach_files(project=project_id, id=entity_id, files={'file': f})
        except Exception as e:
            exception_message = str(e)
            exception_repr_message = repr(e)
            print(f'Exception of Attach_files data vf to 1 attach file {data_vf_path} to entity {entity_id} with exception of {exception_message}')
            print(f'Exception of Attach_files data vf exception repr message of {exception_repr_message}')
        if not result.success:
            print(f'Failed to 2 attach file {data_vf_path} to entity {entity_id} with error of {result.error_info}')
            #return
    print(f'Start to do_swarm_db_upload entity_id : {entity_id} ---- with folder ; {file_path}')
    
    uploadcode = None
    try:
        # Attempt to upload the database
        #dbName = f'vox-mesh-{entity_name}'
        #dbTitle = f'Voxel Mesh Data For {entity_name}'
        dbName = f'vox-pc'
        dbTitle = f'Voxel Data'
        uploadcode = do_swarm_db_upload(project_id, entity_id, file_path, dbName, dbTitle)
    except Exception as e:
        # Log any exceptions that occur during the upload
        print(f'Exception during do_swarm_db_upload: files folder: {file_path} to entity {entity_id} with exception: {str(e)}')
    
    # Check if uploadcode was assigned
    if uploadcode is not None:
        # Proceed with further logic if needed
        print(f'Upload completed with exit code: {uploadcode}')
    else:
        # Handle the case where uploadcode was not assigned (likely due to an exception)
        print('Upload did not complete successfully. Please check the logs for details.')

    result = api.update_entity(
    id=entity_id,
    project=project_id, 
    fields={
        'state' : 'COMPLETE'
    })
    
    create_or_update_ini_file(g_Lambda_Info_ini_path, section_entity, entity_name, entity_id)
    
    if not result.success:
        print(result.error_info)
        exit_code(111)

    print(f'End do_upload_base_meshes_swarm Created entity {result.id} for {entity_name}')
'''
#-----------------------------------------------------------------------------------------------------------

def do_swarm_db_upload(projectId, itemId, outputDBFolder, dbName, dbTitle):
    # Log progress for creating configuration file
    print('XC Create configuration file for uploading dataset')
    
    # Define organization ID
    OrgId = 2343243456678890
    
    # Create configuration string
    uploaddb_cfg = f'''
#[Configuration]
#Organization={OrgId}
#Instance={itemId}
#Project={projectId}
#OutputFolder={outputDBFolder}
#dbName={dbName}
#dbTitle={dbTitle}
#azure_container_name=vfcloudstorage
#azure_storage_connection_string=DefaultEndpointsProtocol=https;AccountName=vfstpangea;AccountKey=qo+5MnyJBELDbjQUBIOyl7mlyg9FlYnz7XShIyao2wd6Et+vVNMv3Szuvc5uY++zhba8TaWq/uXc+AStuouKIQ==;EndpointSuffix=core.windows.net
#storage_provider=AZURE
'''

    # Log the configuration string
    print(f'XC Tool.UploadDB.exe surveys configuration ini file content is : ')
    print(f'{uploaddb_cfg}')
    
    # Create path for the ini file
    ini_file = os.path.join(Data_folder, f'uploaddb_{dbName}.ini')
    print(f'XC Create surveys configuration file {ini_file}')
    
    # Write configuration string to the ini file
    with open(ini_file, "w") as ini:
        ini.write(uploaddb_cfg)
    
    # Log the configuration string again
    print(f'{uploaddb_cfg}')
    
    # Define swarm index and size
    swarmIndex = 0
    swarmSize = 1
    
    # Construct the path for the upload tool
    uploaddb_path = f'{tools}\\Tool.UploadDB.exe {ini_file} {swarmIndex} {swarmSize}'

    # Attach the configuration file for logging
    #lambda_host.attach_file(ini_file)

    # Execute the upload tool and time the operation
    start = timer()
    return_code = xc_run_tool(uploaddb_path, 0, 100)
    end = timer()
    
    # Log the duration and exit code of the upload operation
    print(f'XC Swarm UploadDB: {timedelta(seconds=end - start)}, exit code: {return_code}')
    
    # Return the exit code
    return return_code
#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def swarmDBUploadThread(api : voxelfarmclient.rest, project_id, itemId, outputDBFolder, dbName, dbTitle):
    try:
        print("swarmDBUploadThread")
        
        OrgId = 2343243456678890
    
        # Create configuration string
        uploaddb_cfg = f'''
    [Configuration]
    Organization={OrgId}
    Instance={itemId}
    Project={project_id}
    OutputFolder={outputDBFolder}
    dbName={dbName}
    dbTitle={dbTitle}
    azure_container_name=vfcloudstorage
    azure_storage_connection_string=DefaultEndpointsProtocol=https;AccountName=vfstpangea;AccountKey=qo+5MnyJBELDbjQUBIOyl7mlyg9FlYnz7XShIyao2wd6Et+vVNMv3Szuvc5uY++zhba8TaWq/uXc+AStuouKIQ==;EndpointSuffix=core.windows.net
    storage_provider=AZURE
    '''

        # Log the configuration string
        print(f'XC Tool.UploadDB.exe surveys configuration ini file content is : ')
        print(f'{uploaddb_cfg}')
        
        # Create path for the ini file
        ini_file = os.path.join(Data_folder, f'uploaddb_{dbName}_{itemId}.ini')
        print(f'XC Create surveys configuration file {ini_file}')
        
        # Write configuration string to the ini file
        with open(ini_file, "w") as ini:
            ini.write(uploaddb_cfg)
        
        # Log the configuration string again
        print(f'{uploaddb_cfg}')
        
        # Define swarm index and size
        swarmIndex = 0
        swarmSize = 1
    
        # Create the command to run the external process
        command = [
            f'{tools}\\Tool.UploadDB.exe', 
            ini_file,
            str(swarmIndex), 
            str(swarmSize)
        ]
        
        print("Command arguments: " + " ".join(command))
        
        # Start the process
        process = subprocess.Popen(
            command,
            cwd=tools,  # Working directory
            stdin=subprocess.PIPE,  # Redirecting input
            stdout=subprocess.PIPE,  # Redirecting output
            stderr=subprocess.PIPE,  # Redirecting errors (optional)
            text=True  # This allows reading/writing strings instead of bytes
        )
        
        entity = api.get_entity(itemId)
        
        is_virtual = False
        virtual_value = entity['virtual']
        if virtual_value == "1":
            is_virtual = True

        if not is_virtual:
            # Read the output line by line (non-blocking)
            for line in process.stdout:
                print(f'{itemId}, {project_id}, {line.strip()}')

        # Wait for the process to exit
        process.wait()

        # Check the exit code
        success = process.returncode == 33
        print("UploadDB.exe exit code: " + str(process.returncode))

        if success:
            print(f'swarmDBUploadThread is successful!')
        else:
            print(f'swarmDBUploadThread is failed!')
            
        print("swarmDBUploadThread done")
        return process.returncode

    except Exception as e:
        print(f"Tool.UploadDB.exe Error: {str(e)}")
        return False
#---------------------------------------------------------------------------------------------------------------------------------------------------------------

def do_simple_upload_basemeshes_swarm(api : voxelfarmclient.rest, project_id, basemeshes_db_folderId, file_path : str, version : int, entity_name : str, code_path : str):
    print(f'Start do_simple_upload_basemeshes_swarm Created entity {entity_name}')

    result = api.get_project_crs(project_id)
    crs = result.crs
    entity_id = None

    print(f'start create_entity_raw file for entity {entity_name}')
    result = api.create_entity_raw(project=project_id, 
        type=api.entity_type.VoxelPC, 
        name=entity_name, 
        fields={
            'state': 'PARTIAL',
            'file_folder': basemeshes_db_folderId,
            'virtual': '0',
        }, crs = crs)
    entity_id = result.id
    print(f'end create_entity_raw file for entity {entity_name}')
    if not result.success:
        print(f'Fail to create_entity_raw Created entity for {entity_name} : {result.error_info}')
    else:
        print(f'Successfully to create_entity_raw Created entity for {result.id} for {entity_name}')
        
    #dbName = f'vox-mesh-{entity_name}'
    #dbTitle = f'Voxel Mesh Data For {entity_name}'
    print(f'Start to do_swarm_db_upload entity_id : {entity_id} ---- with folder ; {file_path}')
    
    uploadcode = None
    try:
        # Attempt to upload the database
        #dbName = f'vox-mesh-{entity_name}'
        #dbTitle = f'Voxel Mesh Data For {entity_name}'
        dbName = f'vox-pc'
        dbTitle = f'Voxel Data'
        #uploadcode = do_swarm_db_upload(project_id, entity_id, file_path, dbName, dbTitle)
        uploadcode = swarmDBUploadThread(api, project_id, entity_id, file_path, dbName, dbTitle)
    except Exception as e:
        # Log any exceptions that occur during the upload
        print(f'Exception during do_swarm_db_upload: files folder: {file_path} to entity {entity_id} with exception: {str(e)}')
    
    # Check if uploadcode was assigned
    if uploadcode is not None:
        # Proceed with further logic if needed
        print(f'Upload completed with exit code: {uploadcode}')
    else:
        # Handle the case where uploadcode was not assigned (likely due to an exception)
        print('Upload did not complete successfully. Please check the logs for details.')

    result = api.update_entity(
    id=entity_id,
    project=project_id, 
    fields={
        'state' : 'COMPLETE'
    })
    
    create_or_update_ini_file(g_Lambda_Info_ini_path, section_entity, entity_name, entity_id)
    
    if not result.success:
        print(result.error_info)
        exit_code(111)

    print(f'End do_simple_upload_basemeshes_swarm Created entity {result.id} for {entity_name}')

    
#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def do_process_base_meshes(api : voxelfarmclient.rest, project_id, basemeshes_db_folderId, file_path : str, version : int, entity_name : str, code_path : str):
    print(f'Start do_process_base_meshes Created entity {entity_name}')

    file_extension = 'py'
    code_files = find_files_of_type_in_folder(code_path, file_extension)
    code_files_string = ', '.join(code_files)
    print(f'Python code filse are : {code_files_string}')
    
    result = api.get_project_crs(project_id)
    crs = result.crs
    entity_id = None
    index_vf_path = os.path.join(file_path, 'index.vf')
    with open(index_vf_path, 'rb') as f:
        print(f'start create_entity_raw file for entity {entity_name}')
        result = api.create_entity_raw(project=project_id, 
            type=api.entity_type.VoxelMesh, 
            name=entity_name, 
            fields={
                'state': 'PARTIAL',
                'file_folder': basemeshes_db_folderId,
            }, crs = crs)
        entity_id = result.id
        if not result.success:
            print(f'Fail to create_entity_raw Created entity for {entity_name} : {result.error_info}')
        else:
            print(f'Successfully to create_entity_raw Created entity for {result.id} for {entity_name}')
        index_vf_size = os.path.getsize(index_vf_path)
        print(f'Attaching file {index_vf_path} with size of {index_vf_size} to entity {entity_id}')
        try:
            result = api.attach_files(project=project_id, id=entity_id, files={'file': f})
        except Exception as e:
            print(f'Exception of Attach_files index vf to 1 attach file {data_vf_path} to entity {entity_id} with exception of {str(e)}')
        if not result.success:
            print(f'Failed to 3 attach index vf file {index_vf_path} to entity {entity_id} with error of {result.error_info}')
            return

    data_vf_path = os.path.join(file_path, 'data.vf')
    with open(data_vf_path, 'rb') as f:
        data_vf_size = os.path.getsize(data_vf_path)
        print(f'Attaching file {data_vf_path} with size of {data_vf_size} to entity {entity_id}')
        try:
            result = api.attach_files(project=project_id, id=entity_id, files={'file': f})
        except Exception as e:
            print(f'Exception of Attach_files data vf to 1 attach file {data_vf_path} to entity {entity_id} with exception of {str(e)}')
        if not result.success:
            print(f'Failed to 4 attach data vf file {data_vf_path} to entity {entity_id} with error of {result.error_info}')
            return
    print(f'Start to upload_db entity_id : {entity_id} ---- with folder ; {file_path}')
    
    try:
        result = api.create_process_entity(
        
            project=project_id,
            #type=api.entity_type.Process,
            name=f'Upload Base Meshes Voxel DB : {entity_name}',
            code= 'xc_lambda-uploaddb.py',
            fields={
                'code': 'xc_lambda-uploaddb.py',
                'file_folder': basemeshes_db_folderId,
                'input_value_project_id': project_id,
                'input_value_entity_db_id': entity_id,
                'input_value_db_path' : file_path,
            },
            inputs={
                'input_value_project_id': project_id,
                'input_value_entity_db_id': entity_id,
                'input_value_db_path' : file_path,
            },
            #files = code_files
            #crs=crs,
            files=['xc_lambda-uploaddb.py']
        )
    except Exception as e:
        print(f'Exception of create_process_entity to entity {entity_name} with exception of {str(e)}')
    
    if not result.success:
        print(f'Fail to do_process_base_meshes Created entity for {entity_name} : {result.error_info}')
    else:
        print(f'Successfully to do_process_base_meshes Created entity for {result.id} for {entity_name}')

    print(f'End do_process_base_meshes Created entity {result.id} for {entity_name}')

#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def xc_process_base_meshes(api : voxelfarmclient.rest, basemeshes_output_folder_path, basemeshes_result_project_id, basemeshes_result_folder_id, version : int):
    
    level0_db_output_folder = os.path.join(basemeshes_output_folder_path, f'{tile_size}_{tile_x}_{tile_y}_0')
    level1_db_output_folder = os.path.join(basemeshes_output_folder_path, f'{tile_size}_{tile_x}_{tile_y}_1')

    #project_entity = api.get_entity(basemeshes_result_project_id)
    #version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
    #api.update_entity(project=basemeshes_result_project_id, id=basemeshes_result_project_id, fields={'version': version})
    #level0_entity_name = f'Workflow_Basemeshes_{tile_size}_{tile_x}_{tile_y}_0-ver-{version}'
    #level1_entity_name = f'Workflow_Basemeshes_{tile_size}_{tile_x}_{tile_y}_1-ver-{version}'

    #basemeshes_project_id = '74F0C96BF0F24DA2BB5AE4ED65D81D8C'
    #basemeshes_project_id = '1D4CBBD1D957477E8CC3FF376FB87470' #Project: "My Projects > Pangea Next"
    #basemeshes_db_parent_folderId = 'CBF17A5E89EF4BA2A9A619CC57FBDA93' #Folder: "My Projects > Pangea Next > Basemeshes_Workflow"

    basemeshes_project_id = Project_id #Project: "My Projects > Pangea Next"

    '''
    basemeshes_db_parent_folderId = basemeshes_result_folder_id
    basemeshes_project_entity = api.get_entity(basemeshes_project_id)
    test_version = basemeshes_project_entity['basemeshes_version']
    print(f'-----------------test version: {test_version}!-----------------')
    basemeshes_version = (int(test_version) + 1) if 'basemeshes_version' in basemeshes_project_entity else 1
    api.update_entity(project=basemeshes_project_id, id=basemeshes_project_id, fields={'basemeshes_version': basemeshes_version})  
    print(f'-----------------Successful to get basemeshes_version {basemeshes_version}!-----------------')
    
    result = api.create_folder(project=basemeshes_project_id, name=f'Base Meshes Version {basemeshes_version}', folder=basemeshes_db_parent_folderId)
    if not result.success:
        print(f'Failed to create base meshes db folder for version!')
        exit(4)
    basemeshes_db_folder_Id = result.id
    print(f'Successful to create base meshes db folder {basemeshes_db_folder_Id} for version!')
    '''
    
    basemeshes_version = version
    level0_entity_name = f'Workflow_Basemeshes_{tile_size}_{tile_x}_{tile_y}_0-ver-{basemeshes_version}'
    level1_entity_name = f'Workflow_Basemeshes_{tile_size}_{tile_x}_{tile_y}_1-ver-{basemeshes_version}'

    print(f'basemeshes_result_project_id :  {basemeshes_result_project_id}')
    print(f'basemeshes_result_folder_id :  {basemeshes_result_folder_id}')
    print(f'level0_db_output_folder :  {level0_db_output_folder}')
    print(f'level1_db_output_folder :  {level1_db_output_folder}')
    print(f'version :  {version}')
    print(f'level0_entity_name :  {level0_entity_name}')
    print(f'level1_entity_name :  {level1_entity_name}')
    
    #do_process_base_meshes(api, basemeshes_project_id, basemeshes_db_folder_Id, level0_db_output_folder, basemeshes_version, level0_entity_name, pythoncode_data_folder)
    #do_process_base_meshes(api, basemeshes_project_id, basemeshes_db_folder_Id, level1_db_output_folder, basemeshes_version, level1_entity_name, pythoncode_data_folder)
    #do_upload_base_meshes_swarm(api, basemeshes_project_id, basemeshes_db_folder_Id, level0_db_output_folder, basemeshes_version, level0_entity_name, pythoncode_data_folder)
    #do_upload_base_meshes_swarm(api, basemeshes_project_id, basemeshes_db_folder_Id, level1_db_output_folder, basemeshes_version, level1_entity_name, pythoncode_data_folder)
    
    basemeshes_db_folder_Id = basemeshes_result_folder_id
    #do_simple_upload_basemeshes(api, basemeshes_project_id, basemeshes_db_folder_Id, level0_db_output_folder, basemeshes_version, level0_entity_name, pythoncode_data_folder)
    #do_simple_upload_basemeshes(api, basemeshes_project_id, basemeshes_db_folder_Id, level1_db_output_folder, basemeshes_version, level1_entity_name, pythoncode_data_folder)
    
    do_simple_upload_basemeshes_swarm(api, basemeshes_project_id, basemeshes_db_folder_Id, level0_db_output_folder, basemeshes_version, level0_entity_name, pythoncode_data_folder)
    do_simple_upload_basemeshes_swarm(api, basemeshes_project_id, basemeshes_db_folder_Id, level1_db_output_folder, basemeshes_version, level1_entity_name, pythoncode_data_folder)
    
#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def xc_process_cave_meshes(api : voxelfarmclient.rest, cave_meshes_output_folder_path, cave_meshes_result_project_id, cave_meshes_result_folder_id, version : int):
    
    level0_cave_output_folder = os.path.join(cave_meshes_output_folder_path, f'{tile_size}_{tile_x}_{tile_y}_0')
    level1_cave_output_folder = os.path.join(cave_meshes_output_folder_path, f'{tile_size}_{tile_x}_{tile_y}_1')

    cave_meshes_project_id = Project_id #Project: "My Projects > Pangea Next"
    
    cave_meshes_version = version
    level0_entity_name = f'TopCaves_{tile_size}_{tile_x}_{tile_y}_0-ver-{cave_meshes_version}'
    level1_entity_name = f'TopCaves_{tile_size}_{tile_x}_{tile_y}_1-ver-{cave_meshes_version}'

    print(f'cave_meshes_result_project_id :  {cave_meshes_result_project_id}')
    print(f'cave_meshes_result_folder_id :  {cave_meshes_result_folder_id}')
    print(f'level0_db_output_folder :  {level0_cave_output_folder}')
    print(f'level1_db_output_folder :  {level1_cave_output_folder}')
    print(f'version :  {cave_meshes_version}')
    print(f'level0_entity_name :  {level0_entity_name}')
    print(f'level1_entity_name :  {level1_entity_name}')
    
    cave_meshes_db_folder_Id = cave_meshes_result_folder_id
    
    do_simple_upload_basemeshes_swarm(api, cave_meshes_project_id, cave_meshes_db_folder_Id, level0_cave_output_folder, cave_meshes_version, level0_entity_name, pythoncode_data_folder)
    #do_simple_upload_basemeshes_swarm(api, cave_meshes_project_id, cave_meshes_db_folder_Id, level1_cave_output_folder, cave_meshes_version, level1_entity_name, pythoncode_data_folder)

#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def create_basemeshes_result_entity(api : voxelfarmclient.rest, basemeshes_output_folder_path, basemeshes_result_project_id, basemeshes_result_folder_id):
    
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
    print(f'level0_entity_name :  {level1_entity_name}')

    basemeshes_result_project_entity = api.get_entity(basemeshes_result_project_id)
    version = int(basemeshes_result_project_entity['version']) + 1 if 'version' in basemeshes_result_project_entity else 1
    api.update_entity(project=basemeshes_result_project_id, id=basemeshes_result_project_id, fields={'version': version})
    result = api.create_folder(project=basemeshes_result_project_id, name=f'Version {version}', folder=basemeshes_result_folder_id)
    if not result.success:
        print(f'Failed to create basemeshes workflow result folder for version!')
        exit(4)
    basemeshes_result_version_folder_id = result.id
    print(f'-----------------Successful to create basemeshes workflow folder {basemeshes_result_version_folder_id} for version {version}!-----------------')

    print('Start with create basemeshes workflow level 0 entity {level0_entity_name}')
    result = xc_process_files_entity(api, basemeshes_result_project_id, basemeshes_result_version_folder_id, api.entity_type.RawMesh, api.entity_type.RawMesh, level0_output_folder, level0_entity_name, color=True, zipped=True)
    if not result.success:
        print(f'Failed to create basemeshes workflow result {level0_entity_name} with {api} basemeshes_result_project_id: {basemeshes_result_project_id} level0_output_folder: {level0_output_folder} raw: api.entity_type.RawMesh index: api.entity_type.IndexedMesh version: {version} !')
        exit(4)
    print('End with create basemeshes workflow level 0 entity {level0_entity_name}')

    print('Start with create basemeshes workflow level 1 entity {level1_entity_name}')
    result = xc_process_files_entity(api, basemeshes_result_project_id, basemeshes_result_version_folder_id, api.entity_type.RawMesh, api.entity_type.RawMesh, level1_output_folder, level1_entity_name, color=True, zipped=True)
    if not result.success:
        print(f'Failed to create basemeshes workflow result {level1_entity_name} with {api} basemeshes_result_project_id: {basemeshes_result_project_id} level1_output_folder: {level1_output_folder} raw: api.entity_type.RawMesh index: api.entity_type.IndexedMesh version: {version} !')
        exit(4)
    print('End with create basemeshes workflow level 1 entity {level1_entity_name}')
    return

'''
    result = api.get_project_crs(basemeshes_result_project_id)
    crs = result.crs

    result = api.create_entity_raw(project=basemeshes_result_project_id, 
        type = api.entity_type.RawMesh,
        name = level0_entity_name, 
        fields={
            'file_folder': basemeshes_result_folder_id,
        }, crs = crs)
    level0_entity_id = result.id
    print(f'Attaching file {level0_output_folder} to level 0 entity {level0_entity_id}')
    for file_path in level0_output_folder:
        with open(file_path, "rb") as file:
            api.attach_files(project=basemeshes_result_project_id, id=level0_entity_id, files={'file': file})

    result = api.create_entity_raw(project=basemeshes_result_project_id, 
        type = api.entity_type.RawMesh,
        name = level1_entity_name, 
        fields={
            'file_folder': basemeshes_result_folder_id,
        }, crs = crs)
    level1_entity_id = result.id
    print(f'Attaching file {level1_output_folder} to entity {level1_entity_id}')
    for file_path in level1_output_folder:
        with open(file_path, "rb") as file:
            api.attach_files(project=basemeshes_result_project_id, id=level1_entity_id, files={'file': file})
'''
#---------------------------------------------------------------------------------------------------------------------------------------------------------------

def exit_code(code):
    exit(code)

def tree_instances_generation(config_path):
    print(f'start for step tree_instances_generation')

    if not os.path.exists(config_path):
        print(f'Config File {config_path} does not exist')
        print(f'Config File {config_path} does not exist')
        return -1
    
    print(f'Start to read value from {config_path}')

    cloud_url = read_ini_value(config_path, section_main, 'cloud_url')
    project_id = read_ini_value(config_path, section_main, 'project_id')
    
    tree_entity_id = read_ini_value(config_path, section_main, 'tree_entity_id')
    
    tiles_count = read_ini_value(config_path, section_tiles, 'tiles_count', value_type=int)
    tiles_x = read_ini_value(config_path, section_tiles, 'tiles_x', value_type=int)
    tiles_y = read_ini_value(config_path, section_tiles, 'tiles_y', value_type=int)

    road_input_folder = read_ini_value(config_path, section_input, 'road_input_folder')
    road_exe_path = read_ini_value(config_path, section_input, 'road_exe_path')
    worldgen_exe_path = read_ini_value(config_path, section_input, 'worldgen_exe_path')
    txt2las_exe_path = read_ini_value(config_path, section_input, 'txt2las_exe_path')
    basemeshes_exe_path = read_ini_value(config_path, section_input, 'basemeshes_exe_path')
    tree_exe_path = read_ini_value(config_path, section_input, 'tree_exe_path')
    qtree_assets_folder = read_ini_value(config_path, section_input, 'qtree_assets_folder')
    tree_list = read_ini_value(config_path, section_input, 'treelist_data_path')
    level1_tree_list = read_ini_value(config_path, section_input, 'level1_treelist_data_path')

    road_output_folder = read_ini_value(config_path, section_output, 'road_output_folder')
    smoothlayer_output_base_folder = read_ini_value(config_path, section_output, 'smoothlayer_output_base_folder')
    basemeshes_db_base_folder = read_ini_value(config_path, section_output, 'basemeshes_db_base_folder')
    basemeshes_caves_db_base_folder = read_ini_value(config_path, section_output, 'basemeshes_caves_db_base_folder')
    basemeshes_cache_base_folder = read_ini_value(config_path, section_output, 'basemeshes_cache_base_folder')
    basemeshes_heightmap_folder = read_ini_value(config_path, section_output, 'basemeshes_heightmap_folder')
    
    basemeshes_whole_cache_folder = os.path.join(Data_folder, f'whole')
    basemeshes_qtree_pointcloud_folder = os.path.join(Data_folder, f'qtreepc')
    
    tree_output_base_folder = read_ini_value(config_path, section_output, 'tree_output_base_folder')

    run_road_exe = read_ini_value(config_path, section_run, 'run_road_exe', value_type=bool)
    run_worldgen_road = read_ini_value(config_path, section_run, 'run_worldgen_road', value_type=bool)
    run_upload_smooth_layer = read_ini_value(config_path, section_run, 'run_upload_smooth_layer', value_type=bool)
    run_make_basemeshes = read_ini_value(config_path, section_run, 'run_make_basemeshes', value_type=bool)
    run_upload_basemeshes = read_ini_value(config_path, section_run, 'run_upload_basemeshes', value_type=bool)
    run_make_caves = read_ini_value(config_path, section_run, 'run_make_caves', value_type=bool)
    run_upload_caves = read_ini_value(config_path, section_run, 'run_upload_caves', value_type=bool)
    run_make_tree_instances = read_ini_value(config_path, section_run, 'run_make_tree_instances', value_type=bool)
    run_upload_tree_instances = read_ini_value(config_path, section_run, 'run_upload_tree_instances', value_type=bool)
    run_create_geochem_entity = read_ini_value(config_path, section_run, 'run_create_geochem_entity', value_type=bool)
    run_generate_road_input = read_ini_value(config_path, section_run, 'run_generate_road_input', value_type=bool)

    road_Heightmap_width = read_ini_value(config_path, section_road, 'road_Heightmap_width', value_type=int)
    road_heightmap_height = read_ini_value(config_path, section_road, 'road_heightmap_height', value_type=int)

    basemeshes_debug_level = read_ini_value(config_path, section_others, 'basemeshes_debug_level', value_type=int)
    tree_lod = read_ini_value(config_path, section_others, 'tree_lod', value_type=int)
    forest_age = read_ini_value(config_path, section_others, 'forest_age', value_type=int)
    tree_iteration = read_ini_value(config_path, section_others, 'tree_iteration', value_type=int)

    print(f'End to read value from {config_path}')

    print(f'Start to prepare input data parameter for TreesInstancesAbsolutePathWin.ini')
    basemeshes_all_level = 0 # use all level for base meshes 
    
    basemeshes_level0 = 0
    basemeshes_level1 = 1
    version = 80
        
    road_heightmap_file_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{road_Heightmap_width}_{road_heightmap_height}_ushort_height_map_raw.raw'
    road_humidity_file_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{road_Heightmap_width}_{road_heightmap_height}_byte_humidity_map_raw.raw'
    road_regions_name_file_name = f'{tiles_count}_{tiles_x}_{tiles_y}_regions_name.csv'
    road_regions_namelist_file_name = f'{tiles_count}_{tiles_x}_{tiles_y}_regions_namelist.csv'
    
    original_road_heightmap_file_path = os.path.join(road_input_folder, road_heightmap_file_name)
    original_road_humidity_file_path = os.path.join(road_input_folder, road_humidity_file_name)
    road_regions_name_file_path = os.path.join(road_input_folder, road_regions_name_file_name)
    road_regions_namelist_file_path = os.path.join(road_input_folder, road_regions_namelist_file_name)

    most_travelled_points_path = os.path.join(road_output_folder, f'{tiles_count}_{tiles_x}_{tiles_y}_Most_Travelled_Points.csv') 
    most_distant_points_path = os.path.join(road_output_folder, f'{tiles_count}_{tiles_x}_{tiles_y}_Most_Distant_Points.csv') 
    region_centroid_points_path = os.path.join(road_output_folder, f'{tiles_count}_{tiles_x}_{tiles_y}_Region_Centroid_Points.csv')
    regions_raw_path = os.path.join(road_output_folder, f'{tiles_count}_{tiles_x}_{tiles_y}_regions.raw') 
    regions_info_path = os.path.join(road_output_folder, f'{tiles_count}_{tiles_x}_{tiles_y}_regions_info.csv') 
    
    tree_ini_folder = os.path.join(tree_output_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}') 
    tree_ini_name = 'TreesInstancesAbsolutePathWin.ini'
    tree_ini_path = os.path.join(tree_ini_folder, tree_ini_name) 
    if not os.path.exists(tree_ini_folder):
        os.makedirs(tree_ini_folder)
        
    basemeshes_ini_name = 'BaseMeshVoxelizerAdvanced.ini'
    basemeshes_ini_path = os.path.join(Data_folder, basemeshes_ini_name)
    basemeshes_csv_name = 'BaseMeshes.csv'
    basemeshes_csv_path = os.path.join(Data_folder, basemeshes_csv_name)

    basemeshes_assets_folder = qtree_assets_folder
    basemeshes_0_heightmap_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level0}_{basemeshes_debug_level}_heightarray.bin'
    basemeshes_1_heightmap_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level1}_{basemeshes_debug_level}_heightarray.bin'
    basemeshes_0_heightmap_mask_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level0}_{basemeshes_debug_level}_heightmasks.bin'
    basemeshes_1_heightmap_mask_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level1}_{basemeshes_debug_level}_heightmasks.bin'
    basemeshes_0_heightmap_path = os.path.join(basemeshes_heightmap_folder, basemeshes_0_heightmap_name)
    basemeshes_1_heightmap_path = os.path.join(basemeshes_heightmap_folder, basemeshes_1_heightmap_name)
    basemeshes_0_heightmap_mask_path = os.path.join(basemeshes_heightmap_folder, basemeshes_0_heightmap_mask_name)
    basemeshes_1_heightmap_mask_path = os.path.join(basemeshes_heightmap_folder, basemeshes_1_heightmap_mask_name)

    smoothlayer_output_folder = os.path.join(smoothlayer_output_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}')
    toplayer_heightmap_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.height.raw'
    toplayer_heightmap_mask_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.height.masks.raw'
    level1_heightmap_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_level1.xyz.height.raw'
    level1_heightmap_mask_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_level1.xyz.height.masks.raw'
    bedrock_heightmap_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_bedrock.xyz.height.raw'
    bedrock_heightmap_mask_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_bedrock.xyz.height.masks.raw'
    lakes_heightmap_make_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_lakes.xyz.height.masks.raw'
    level1_lakes_heightmap_make_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_lakes_level1.xyz.height.masks.raw'
    
    toplayer_heightmap_path = os.path.join(smoothlayer_output_folder, toplayer_heightmap_name)
    toplayer_heightmap_mask_path = os.path.join(smoothlayer_output_folder, toplayer_heightmap_mask_name)
    level1_heightmap_path = os.path.join(smoothlayer_output_folder, level1_heightmap_name)
    level1_heightmap_mask_path = os.path.join(smoothlayer_output_folder, level1_heightmap_mask_name)
    bedrock_heightmap_path = os.path.join(smoothlayer_output_folder, bedrock_heightmap_name)
    bedrock_heightmap_mask_path = os.path.join(smoothlayer_output_folder, bedrock_heightmap_mask_name)
    lakes_heightmap_make_path = os.path.join(smoothlayer_output_folder, lakes_heightmap_make_name)
    level1_lakes_heightmap_make_path = os.path.join(smoothlayer_output_folder, level1_lakes_heightmap_make_name)        
    
    toplayer_image_upload_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.jpg'
    toplayer_image_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.png'
    toplayer_image_meta_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.jgw'
    
    toplevel_file_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz'
    level1_file_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_level1.xyz'
    bedrock_file_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_bedrock.xyz'
    lakes_file_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_lakes.xyz'
    lakes_bottom_file_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_lakes_bottom.xyz'
    lakes_bottom_level1_file_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_lakes_bottom_level1.xyz'
    lakes_level1_file_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_lakes_level1.xyz'
    ocean_top_file_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_ocean_top.xyz'
    ocean_bottom_file_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_ocean_bottom.xyz'
    
    toplayer_image_upload_path = os.path.join(smoothlayer_output_folder, toplayer_image_upload_name)
    toplayer_image_path = os.path.join(smoothlayer_output_folder, toplayer_image_name)
    toplayer_image_meta_path = os.path.join(smoothlayer_output_folder, toplayer_image_meta_name)
    toplevel_file_path = os.path.join(smoothlayer_output_folder, toplevel_file_name)
    level1_file_path = os.path.join(smoothlayer_output_folder, level1_file_name)
    bedrock_file_path = os.path.join(smoothlayer_output_folder, bedrock_file_name)
    lakes_file_path = os.path.join(smoothlayer_output_folder, lakes_file_name)
    lakes_bottom_file_path = os.path.join(smoothlayer_output_folder, lakes_bottom_file_name)
    lakes_bottom_level1_file_path = os.path.join(smoothlayer_output_folder, lakes_bottom_level1_file_name)
    lakes_level1_file_path = os.path.join(smoothlayer_output_folder, lakes_level1_file_name)
    ocean_top_file_path = os.path.join(smoothlayer_output_folder, ocean_top_file_name)
    ocean_bottom_file_path = os.path.join(smoothlayer_output_folder, ocean_bottom_file_name)
    
    basemeshes_caves_db_output_level0_folder = os.path.join(basemeshes_caves_db_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}_0')
    basemeshes_caves_db_output_level1_folder = os.path.join(basemeshes_caves_db_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}_1')
    caves_point_cloud_level_0_file_name = f'{tiles_count}_{tiles_x}_{tiles_y}_0_caves.xyz'
    caves_point_cloud_level_1_file_name = f'{tiles_count}_{tiles_x}_{tiles_y}_1_caves.xyz'
    caves_point_cloud_level_0_file_path = os.path.join(basemeshes_caves_db_output_level0_folder, caves_point_cloud_level_0_file_name)
    caves_point_cloud_level_1_file_path = os.path.join(basemeshes_caves_db_output_level1_folder, caves_point_cloud_level_1_file_name)
    

    print(f'End to to prepare input data parameter for TreesInstancesAbsolutePathWin.ini')

    print(f'Start to prepare command line for programs')

    api = voxelfarmclient.rest(cloud_url)

    project_entity = api.get_entity(project_id)
    project_output_version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
    api.update_entity(project=project_id, id=project_id, fields={'version': project_output_version})
    
    dont_run_road_game = 1
    road_exe_command = f'{road_exe_path} {tiles_count} {tiles_x} {tiles_y} {road_Heightmap_width} {road_heightmap_height} {road_input_folder} {road_output_folder} {dont_run_road_game}'
    worldgen_level = 5
    worldgen_command =  f'{worldgen_exe_path} {tiles_count} {tiles_x} {tiles_y} {worldgen_level} {qtree_assets_folder} {smoothlayer_output_base_folder} {road_output_folder}'
    if run_generate_road_input:
        worldgen_command =  f'{worldgen_exe_path} {tiles_count} {tiles_x} {tiles_y} {worldgen_level} {qtree_assets_folder} {smoothlayer_output_base_folder}'
    
    basemeshvoxelizer_ini_command = f'{basemeshes_exe_path} {basemeshes_ini_path}'
    tree_exe_command = f'{tree_exe_path} {tree_ini_path}'
    
    basemeshvoxelizer1_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level1} {basemeshes_assets_folder} {basemeshes_db_base_folder} {basemeshes_cache_base_folder} {basemeshes_debug_level} {basemeshes_heightmap_folder}'
    basemeshvoxelizer0_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level0} {basemeshes_assets_folder} {basemeshes_db_base_folder} {basemeshes_cache_base_folder} {basemeshes_debug_level} {basemeshes_heightmap_folder}' 
    cave_meshes_flag = 1
    cave_meshvoxelizer1_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level1} {basemeshes_assets_folder} {basemeshes_db_base_folder} {basemeshes_cache_base_folder} {basemeshes_all_level} {basemeshes_heightmap_folder} {basemeshes_caves_db_base_folder} {cave_meshes_flag}'
    cave_meshvoxelizer0_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level0} {basemeshes_assets_folder} {basemeshes_db_base_folder} {basemeshes_cache_base_folder} {basemeshes_all_level} {basemeshes_heightmap_folder} {basemeshes_caves_db_base_folder} {cave_meshes_flag}'       

    if use_basemesh_ini:
        print(f'Start to write standard basemeshes ini files : {basemeshes_ini_path}')
        create_or_overwrite_empty_file(basemeshes_ini_path)
        
        create_or_update_ini_file(basemeshes_ini_path, section_tiles, 'Tiles_Count', tiles_count)
        create_or_update_ini_file(basemeshes_ini_path, section_tiles, 'Tiles_X_Index', tiles_x)
        create_or_update_ini_file(basemeshes_ini_path, section_tiles, 'Tiles_Y_Index', tiles_y)
        
        create_or_update_ini_file(basemeshes_ini_path, section_input, 'Assets_Folder', basemeshes_assets_folder)
        create_or_update_ini_file(basemeshes_ini_path, section_input, 'BaseMeshesCSV_Name', basemeshes_csv_name)
        
        create_or_update_ini_file(basemeshes_ini_path, section_output, 'DB_Base_Folder', basemeshes_db_base_folder)
        create_or_update_ini_file(basemeshes_ini_path, section_output, 'Caves_DB_Base_Folder', basemeshes_caves_db_base_folder)
        create_or_update_ini_file(basemeshes_ini_path, section_output, 'Cache_Base_Folder', basemeshes_cache_base_folder)
        create_or_update_ini_file(basemeshes_ini_path, section_output, 'Heightmap_Folder', basemeshes_heightmap_folder)
        create_or_update_ini_file(basemeshes_ini_path, section_output, 'Whole_WC_Cache_Folder', basemeshes_whole_cache_folder)
        create_or_update_ini_file(basemeshes_ini_path, section_output, 'QTree_PointCloud_Folder', basemeshes_qtree_pointcloud_folder)
        
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_APPLY_COASTLINE', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_APPLY_TOP_MIDDLE_NAIL_COASTLINE', False)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_REPLACE_COASTLINE_WITH_CUBE', False)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_ONLY_GENERATE_COASTLINE_MESHES', False)
        
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_ROAD_ATTRIBUTE', False)
        
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_GET_HEIGHT_BY_VERTICES', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_GET_HEIGHT_BY_RAYTEST_VF', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_GET_HEIGHT_BY_RAYTEST_WC', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_GET_HEIGHT_BY_RAYTEST_RTREE_WC', True)
        
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_ALSO_OUTPUT_CACHE_MESHES_IN_WC', False)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_WHOLE_MESHES_WC', True)
        
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_AVERAGE_HEIGHT', False)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_HEIGHTMAP_MASK', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_SHORT_HEIGHTMAP', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_FLOAT_HEIGHTMAP', False)
        
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_QTREE_CSV', False)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_COASTLINE_JSON', False)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_COASTLINE_CSV', False)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_QTREE_NODE_CSV', False)
        
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_BASEMESHES_OUTPUT_PROGRESS', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_NODE_MAX_DIMS', False)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_QUADBOUNDS_ERROR', False)
        
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_APPLY_RANDOM_SCALE', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_APPLY_RANDOM_ROTATION', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_APPLY_RANDOM_NOISE_BASEMESH_SELECTION', False)
        
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_APPLY_SIZE_LIMIT_FOR_VOXELIZE_NODE', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_APPLY_NO_TOP_DELTA_FOR_BASEMESH_MASK', False)

        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_APPLY_QUICK_VOXELIZE_MESH_DEBUG', False)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_APPLY_IGNORE_LOWER_MINHEIGHT_MESHES', True)

        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_WHOLE_MESH_POINT_CLOUD', False)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_WHOLE_MESH_HEIGHT_DIFFERENT', False)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_WHOLE_MESH_DIFF_CELLS', False)
        
        print(f'End to write standard basemeshes ini files : {basemeshes_ini_path}')
        basemeshes_ini_string = ini_file_to_string(basemeshes_ini_path)
        print(f'Basemeshes standard ini file content is :')
        print(f'{basemeshes_ini_string}')
        
    if run_road_exe:    
        print(f'road_exe_command : {road_exe_command}')
    if run_worldgen_road:
        print(f'worldgen_command : {worldgen_command}')
    if run_make_basemeshes:
        if use_basemesh_ini:
            print(f'basemeshvoxelizer_ini_command : {basemeshvoxelizer_ini_command}')
        else:
            print(f'basemeshvoxelizer0_command : {basemeshvoxelizer0_command}')
            print(f'basemeshvoxelizer1_command : {basemeshvoxelizer1_command}')
    
    if run_make_tree_instances:
        print(f'tree_exe_command : {tree_exe_command}')
        
    print(f'End to prepare command line for programs') 
        
    if run_road_exe:
        ##### Generate the road obj and image for smooth layer. 
        #return_code_road = launch_process(road_exe_command)
        print(f'step for to run_road_exe : {road_exe_command}')
        return_code_road = xc_run_tool(road_exe_command, 21, 40)
        if return_code_road == 0:
            print(f'Process ({road_exe_command}) executed successfully.')
        else:
            print(f'Error: The process ({road_exe_command}) returned a non-zero exit code ({run_road_exe}).')
            return -1
    
    toplevel_image_entity_base_name = f'RoadImage_{tiles_count}_{tile_x}_{tiles_y}'
    toplevel_layer_entity_base_name = f'TopLevel_{tiles_count}_{tile_x}_{tiles_y}'
    level1_layer_entity_base_name = f'Level1_{tiles_count}_{tile_x}_{tiles_y}'
    bedrock_layer_entity_base_name = f'BedRock_{tiles_count}_{tile_x}_{tiles_y}'
    lakes_layer_entity_base_name = f'lakes_{tiles_count}_{tile_x}_{tiles_y}'
    lakes_bottom_layer_entity_base_name = f'lakes_bottom_{tiles_count}_{tile_x}_{tiles_y}'
    lakes_bottom_level1_layer_entity_base_name = f'lakes_bottom_level1_{tiles_count}_{tile_x}_{tiles_y}'
    lakes_level1_layer_entity_base_name = f'lakes_level1_{tiles_count}_{tile_x}_{tiles_y}'
    ocean_top_layer_entity_base_name = f'ocean_top_{tiles_count}_{tile_x}_{tiles_y}'
    ocean_bottom_layer_entity_base_name = f'ocean_bottom_{tiles_count}_{tile_x}_{tiles_y}'
    
    if run_worldgen_road:
        print(f'step for to run_worldgen_road : {worldgen_command}')
        ##### Generate the height map and image for smooth layer. 
        #return_code_worldgen_road = launch_process(worldgen_command)
        return_code_worldgen_road = xc_run_tool(worldgen_command, 41, 60)
        if return_code_worldgen_road == 0:
            print(f'Process ({worldgen_command}) executed successfully.')
        else:
            print(f'Error: The process ({worldgen_command}) returned a non-zero exit code ({return_code_worldgen_road}).')
            return -1
        
    basemeshes_output_folder = basemeshes_db_base_folder
    
    level0_db_output_folder = os.path.join(basemeshes_output_folder, f'{tile_size}_{tile_x}_{tile_y}_0')
    level1_db_output_folder = os.path.join(basemeshes_output_folder, f'{tile_size}_{tile_x}_{tile_y}_1')
    index_db_file_name = f'index.vf'
    data_db_file_name = f'data.vf'
    level0_index_db_file_path = os.path.join(level0_db_output_folder, index_db_file_name)
    level0_data_db_file_path = os.path.join(level0_db_output_folder, data_db_file_name)
    level1_index_db_file_path = os.path.join(level1_db_output_folder, index_db_file_name)
    level1_data_db_file_path = os.path.join(level1_db_output_folder, data_db_file_name)
    
    level0_index_cave_file_path = os.path.join(basemeshes_caves_db_output_level0_folder, index_db_file_name)
    level0_data_cave_file_path = os.path.join(basemeshes_caves_db_output_level0_folder, data_db_file_name)
    level1_index_cave_file_path = os.path.join(basemeshes_caves_db_output_level1_folder, index_db_file_name)
    level1_data_cave_file_path = os.path.join(basemeshes_caves_db_output_level1_folder, data_db_file_name)
    
    if (run_generate_road_input or run_make_tree_instances) and run_make_basemeshes:
        RemoveBaseMeshesdata(level0_index_db_file_path, level0_data_db_file_path, level1_index_db_file_path, level1_data_db_file_path)
            
        if use_basemesh_ini:
            print(f'step for run basemeshes with ini : {basemeshvoxelizer_ini_command}')
            
            print(f'step for run basemeshes with ini level : {basemeshes_level1}')
            if run_generate_road_input:
                create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_WHOLE_MESHES_WC', False)
            else:
                create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_WHOLE_MESHES_WC', True)
            create_or_update_ini_file(basemeshes_ini_path, section_others, 'LodDebugLevel', basemeshes_debug_level)
            create_or_update_ini_file(basemeshes_ini_path, section_others, 'Level', basemeshes_level1)
            basemeshes_ini_string = ini_file_to_string(basemeshes_ini_path)
            print(f'Basemeshes ini file for level {basemeshes_level1} content is :')
            print(f'{basemeshes_ini_string}')
            return_code_basemesh_ini_1 = xc_run_tool(basemeshvoxelizer_ini_command, 61, 80)
            if return_code_basemesh_ini_1 == 0:
                print(f'Process level {basemeshes_level1} with ({basemeshvoxelizer_ini_command}) executed successfully.')
            else:
                print(f'Error: The process level {basemeshes_level1} with ({basemeshvoxelizer_ini_command}) returned a non-zero exit code ({return_code_basemesh_ini_1}).')
                return -1
            
            print(f'step for run basemeshes with ini level : {basemeshes_level0}')
            create_or_update_ini_file(basemeshes_ini_path, section_others, 'Level', basemeshes_level0)
            basemeshes_ini_string = ini_file_to_string(basemeshes_ini_path)
            print(f'Basemeshes ini file for level {basemeshes_level0} content is :')
            print(f'{basemeshes_ini_string}')
            return_code_basemesh_ini_0 = xc_run_tool(basemeshvoxelizer_ini_command, 81, 90)
            if return_code_basemesh_ini_0 == 0:
                print(f'Process level {basemeshes_level0} with ({basemeshvoxelizer_ini_command}) executed successfully.')
            else:
                print(f'Error: The process level {basemeshes_level0} with ({basemeshvoxelizer_ini_command}) returned a non-zero exit code ({return_code_basemesh_ini_0}).')
                return -1
            
        else:
            ##### Generate the height map from level 0 of BaseMeshes.  
            print(f'step for to run_make_basemeshes : {basemeshvoxelizer0_command}')
            return_code_basemash0 = xc_run_tool(basemeshvoxelizer0_command, 0, 100)
            if return_code_basemash0 == 0:
                print(f'Process ({basemeshvoxelizer0_command}) executed successfully.')
            else:
                print(f'Error: The process ({basemeshvoxelizer0_command}) returned a non-zero exit code ({return_code_basemash0}).')
                return -1
            
            ##### Generate the height map from level 1 of BaseMeshes. 
            print(f'step for to run_make_basemeshes : {basemeshvoxelizer1_command}')
            return_code_basemash1 = xc_run_tool(basemeshvoxelizer1_command, 0, 100)
            if return_code_basemash1 == 0:
                print(f'Process ({basemeshvoxelizer1_command}) executed successfully.')
            else:
                print(f'Error: The process ({basemeshvoxelizer1_command}) returned a non-zero exit code ({return_code_basemash1}).')
                return -1
        
    run_level_0_instances = True
    run_level_1_instances = True
        
    if run_make_tree_instances:
         ##### Make ini config file for tree exe.
        #clear_all_sections(tree_ini_path)
        useWithBaseMeshesLevel1 = True   
        
        if only_run_level_0_instances:
            run_level_1_instances = False
        if only_run_level_1_instances:
            run_level_0_instances = False
            
        print(f'Start to write tree instance ini files : {tree_ini_path}')
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
        create_or_update_ini_file(tree_ini_path, section_input, 'BedrokLayer_heightMap', bedrock_heightmap_path)
        create_or_update_ini_file(tree_ini_path, section_input, 'BaseMeshes_Level_0_HeightMap_Mask', basemeshes_0_heightmap_mask_path)
        create_or_update_ini_file(tree_ini_path, section_input, 'BaseMeshes_Level_1_HeightMap_Mask', basemeshes_1_heightmap_mask_path)
        create_or_update_ini_file(tree_ini_path, section_input, 'TopLayer_HeightMap_Mask', toplayer_heightmap_mask_path)
        create_or_update_ini_file(tree_ini_path, section_input, 'Level1Layer_heightMap_Mask', level1_heightmap_mask_path)
        create_or_update_ini_file(tree_ini_path, section_input, 'Bedrock_heightMap_Mask', bedrock_heightmap_mask_path)
        create_or_update_ini_file(tree_ini_path, section_input, 'Lakes_HeightMap_Mask', lakes_heightmap_make_path)
        create_or_update_ini_file(tree_ini_path, section_input, 'Level1_Lakes_HeightMap_Mask', level1_lakes_heightmap_make_path)
        create_or_update_ini_file(tree_ini_path, section_input, 'Most_Travelled_Points', most_travelled_points_path)
        create_or_update_ini_file(tree_ini_path, section_input, 'Most_Distant_Points', most_distant_points_path)
        create_or_update_ini_file(tree_ini_path, section_input, 'Region_Centroid_Points', region_centroid_points_path)
        create_or_update_ini_file(tree_ini_path, section_input, 'Caves_Point_Cloud_Level_0', caves_point_cloud_level_0_file_path)
        create_or_update_ini_file(tree_ini_path, section_input, 'Caves_Point_Cloud_Level_1', caves_point_cloud_level_1_file_path)
        create_or_update_ini_file(tree_ini_path, section_input, 'Tree_List', tree_list)
        create_or_update_ini_file(tree_ini_path, section_input, 'Level1_Tree_List', level1_tree_list)
        create_or_update_ini_file(tree_ini_path, section_input, 'Regions_Raw', regions_raw_path)
        create_or_update_ini_file(tree_ini_path, section_input, 'Regions_Info', regions_info_path)
        
        create_or_update_ini_file(tree_ini_path, section_output, 'Output_Dir', tree_output_base_folder)
        create_or_update_ini_file(tree_ini_path, section_others, 'Lod', tree_lod)
        create_or_update_ini_file(tree_ini_path, section_others, 'Forest_Age', forest_age)
        create_or_update_ini_file(tree_ini_path, section_others, 'Tree_Iteration', tree_iteration)
        
        create_or_update_ini_file(tree_ini_path, section_options,'Only_Road_Data', run_generate_road_input)
        create_or_update_ini_file(tree_ini_path, section_options,'Use_With_BaseMeshes_Level1', useWithBaseMeshesLevel1)
        create_or_update_ini_file(tree_ini_path, section_options,'Level0_Instances', run_level_0_instances)
        create_or_update_ini_file(tree_ini_path, section_options,'Level1_Instances', run_level_1_instances)
        
        print(f'End to write tree instance ini files : {tree_ini_path}')
        tree_ini_string = ini_file_to_string(tree_ini_path)
        print(f'Tree standard ini file content is :')
        print(f'{tree_ini_string}')
        print(f'step for to run_make_tree_instances : {tree_exe_command}')
        ##### Run tree exe to generate to tree instances.
        return_code_tree = xc_run_tool(tree_exe_command, 91, 100)
        if return_code_tree == 0:
            print(f'Process ({tree_exe_command}) executed successfully.')
        else:
            print(f'Error: The process ({tree_exe_command}) returned a non-zero exit code ({return_code_tree}).')
            return -1
        
    if run_generate_road_input:
        # todo to triger update road data
        print(f'Start to run_generate_road_input')
        
        new_road_heightmap_file_path = os.path.join(tree_output_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}', road_heightmap_file_name)
        new_road_humidity_file_path = os.path.join(tree_output_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}', road_humidity_file_name)
        print(f'new_road_heightmap_file_path is : {new_road_heightmap_file_path}')
        print(f'new_road_humidity_file_path is : {new_road_humidity_file_path}')
        print(f'original_road_heightmap_file_path is : {original_road_heightmap_file_path}')
        print(f'original_road_humidity_file_path is : {original_road_humidity_file_path}')
        print(f'road_regions_name_file_path is : {road_regions_name_file_path}')
        print(f'road_regions_namelist_file_path is : {road_regions_namelist_file_path}')
    
        #shutil.copy2(new_road_heightmap_file_path, original_road_heightmap_file_path)
        shutil.copy2(new_road_heightmap_file_path, roaddata_active_version_property)
        
        #shutil.copy2(new_road_humidity_file_path, original_road_humidity_file_path)
        shutil.copy2(new_road_humidity_file_path, roaddata_active_version_property)
            
        print(f'End to run_generate_road_input')

    if run_upload_smooth_layer:
        print(f'step for to run_upload_smooth_layer : {worldgen_command}')
        process_file_image(api, Project_id, Workflow_Output_Result_Folder_id, toplayer_image_upload_path, toplayer_image_meta_path, toplevel_image_entity_base_name, project_output_version)
        
        process_point_cloud(api, txt2las_exe_path, Project_id, Workflow_Output_Result_Folder_id, toplevel_file_path, api.entity_type.VoxelTerrain, toplevel_layer_entity_base_name, project_output_version, color=True)
        process_point_cloud(api, txt2las_exe_path, Project_id, Workflow_Output_Result_Folder_id, level1_file_path, api.entity_type.VoxelTerrain, level1_layer_entity_base_name, project_output_version, color=True)
        process_point_cloud(api, txt2las_exe_path, Project_id, Workflow_Output_Result_Folder_id, bedrock_file_path, api.entity_type.VoxelTerrain, bedrock_layer_entity_base_name, project_output_version, color=True)
        
        process_point_cloud(api, txt2las_exe_path, Project_id, Workflow_Output_Result_Folder_id, lakes_file_path, api.entity_type.VoxelTerrain, lakes_layer_entity_base_name, project_output_version, color=True)
        process_point_cloud(api, txt2las_exe_path, Project_id, Workflow_Output_Result_Folder_id, lakes_bottom_file_path, api.entity_type.VoxelTerrain, lakes_bottom_layer_entity_base_name, project_output_version, color=True)
        process_point_cloud(api, txt2las_exe_path, Project_id, Workflow_Output_Result_Folder_id, lakes_bottom_level1_file_path, api.entity_type.VoxelTerrain, lakes_bottom_level1_layer_entity_base_name, project_output_version, color=True)
        process_point_cloud(api, txt2las_exe_path, Project_id, Workflow_Output_Result_Folder_id, lakes_level1_file_path, api.entity_type.VoxelTerrain, lakes_level1_layer_entity_base_name, project_output_version, color=True)
        process_point_cloud(api, txt2las_exe_path, Project_id, Workflow_Output_Result_Folder_id, ocean_top_file_path, api.entity_type.VoxelTerrain, ocean_top_layer_entity_base_name, project_output_version, color=True)
        process_point_cloud(api, txt2las_exe_path, Project_id, Workflow_Output_Result_Folder_id, ocean_bottom_file_path, api.entity_type.VoxelTerrain, ocean_bottom_layer_entity_base_name, project_output_version, color=True)
        
    tree_instance_output_folder_name = 'instanceoutput'
    tree_instance_level0_output_folder_name = 'instanceoutput_level0'
    tree_instance_level1_output_folder_name = 'instanceoutput_level1'
    regions_output_folder_name = 'regionoutput'
    geo_chemical_folder_name = 'GeoChemical'
    tree_height_file_name = f'{tiles_count}_{tiles_x}_{tiles_y}_short_height_map_export.xyz'
    
    tree_instance_output_folder_path = os.path.join(tree_output_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}', tree_instance_output_folder_name)
    tree_instance_level0_output_folder_path = os.path.join(tree_output_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}', tree_instance_level0_output_folder_name)
    tree_instance_level1_output_folder_path = os.path.join(tree_output_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}', tree_instance_level1_output_folder_name)
    regions_output_folder_path = os.path.join(tree_output_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}', regions_output_folder_name)
    geo_chemical_folder_path = os.path.join(tree_output_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}', geo_chemical_folder_name)
    tree_height_file_path = os.path.join(tree_output_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}', tree_height_file_name)
    final_height_layer_entity_base_name = f'final_height_{tiles_count}_{tile_x}_{tiles_y}'
    
    if run_create_geochem_entity:
        print(f'step for to run_create_geochem_entity!')
        ##### create the geochem entity for tree instance files.
        geochem_result_folder_id = Workflow_Output_Result_Folder_id
    
        if run_level_0_instances:
            create_geochem_tree_entity(api, Project_id, geochem_result_folder_id, geo_chemical_folder_path, Tiles_size, Tiles_x, Tiles_y, 0, project_output_version)
            print(f'create_geochem_tree_entity level 0 from {geo_chemical_folder_path}')
        
        if run_level_1_instances:
            create_geochem_tree_entity(api, Project_id, geochem_result_folder_id, geo_chemical_folder_path, Tiles_size, Tiles_x, Tiles_y, 1, project_output_version)
            print(f'create_geochem_tree_entity level 1 from {geo_chemical_folder_path}')
        
        if os.path.exists(tree_height_file_path):
            process_point_cloud(api, txt2las_exe_path, Project_id, Workflow_Output_Result_Folder_id, tree_height_file_path, api.entity_type.VoxelTerrain, final_height_layer_entity_base_name, project_output_version, color=True)
        
    if run_upload_tree_instances:
        print(f'step for to run_upload_tree_instances')
        ##### Update the tree instance files of tree entity. 
        #update_attach_files_for_entity(api, project_id, tree_entity_id, tree_instance_output_folder, f'instances_lod8_{tiles_count}_{tiles_x}_{tiles_y}-{version}', version=version, color=True)
        merge_instances_csv_files(tree_instance_level0_output_folder_path, tree_instance_level1_output_folder_path, tree_instance_output_folder_path)
        update_attach_files_for_entity(api, project_id, tree_entity_id, tree_instance_output_folder_path)
        print(f'update_attach_files_for_entity tree instances from {tree_instance_output_folder_path} for {tree_entity_id}')
        
        ##### Update the tree region files of tree entity. 
        update_attach_files_for_entity(api, project_id, tree_entity_id, regions_output_folder_path)
        print(f'update_attach_files_for_entity cell regions from {regions_output_folder_path} for {tree_entity_id}')

    if run_make_basemeshes and run_upload_basemeshes: 
        RemoveBaseMeshesdata(level0_index_db_file_path, level0_data_db_file_path, level1_index_db_file_path, level1_data_db_file_path)
            
        if use_basemesh_ini:
            print(f'step for run basemeshes with ini : {basemeshvoxelizer_ini_command}')
            print(f'step for run basemeshes with ini level : {basemeshes_level1}')
            create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_WHOLE_MESHES_WC', False)
            create_or_update_ini_file(basemeshes_ini_path, section_others, 'LodDebugLevel', basemeshes_all_level)
            create_or_update_ini_file(basemeshes_ini_path, section_others, 'Level', basemeshes_level1)
            print(f'Adjust base meshes ini level {basemeshes_level1} to all LOD level')
            basemeshes_ini_string = ini_file_to_string(basemeshes_ini_path)
            print(f'Basemeshes ini file for level {basemeshes_level1} content is :')
            print(f'{basemeshes_ini_string}')
            return_code_basemesh_ini_1 = xc_run_tool(basemeshvoxelizer_ini_command, 61, 80)
            if return_code_basemesh_ini_1 == 0:
                print(f'Process level {basemeshes_level1} with ({basemeshvoxelizer_ini_command}) executed successfully.')
            else:
                print(f'Error: The process level {basemeshes_level1} with ({basemeshvoxelizer_ini_command}) returned a non-zero exit code ({return_code_basemesh_ini_1}).')
                return -1
            
            print(f'step for run basemeshes with ini level : {basemeshes_level0}')
            create_or_update_ini_file(basemeshes_ini_path, section_others, 'Level', basemeshes_level0)
            print(f'Adjust base meshes ini level {basemeshes_level0} to all LOD level')
            basemeshes_ini_string = ini_file_to_string(basemeshes_ini_path)
            print(f'Basemeshes ini file for level {basemeshes_level0} content is :')
            print(f'{basemeshes_ini_string}')
            return_code_basemesh_ini_0 = xc_run_tool(basemeshvoxelizer_ini_command, 81, 90)
            if return_code_basemesh_ini_0 == 0:
                print(f'Process level {basemeshes_level0} with ({basemeshvoxelizer_ini_command}) executed successfully.')
            else:
                print(f'Error: The process level {basemeshes_level0} with ({basemeshvoxelizer_ini_command}) returned a non-zero exit code ({return_code_basemesh_ini_0}).')
                return -1
        else:
            basemeshvoxelizer1_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level1} {basemeshes_assets_folder} {basemeshes_db_base_folder} {basemeshes_cache_base_folder} {basemeshes_all_level} {basemeshes_heightmap_folder}'
            basemeshvoxelizer0_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level0} {basemeshes_assets_folder} {basemeshes_db_base_folder} {basemeshes_cache_base_folder} {basemeshes_all_level} {basemeshes_heightmap_folder}'
            print("Adjust base meshes command line to all level")
            ##### Generate the height map from level 0 of BaseMeshes.  
            print(f'step for to run_make_basemeshes : {basemeshvoxelizer0_command}')
            return_code_basemash0 = xc_run_tool(basemeshvoxelizer0_command, 0, 100)
            if return_code_basemash0 == 0:
                print(f'Process ({basemeshvoxelizer0_command}) executed successfully.')
            else:
                print(f'Error: The process ({basemeshvoxelizer0_command}) returned a non-zero exit code ({return_code_basemash0}).')
                return -1
            
            ##### Generate the height map from level 1 of BaseMeshes. 
            print(f'step for to run_make_basemeshes : {basemeshvoxelizer1_command}')
            return_code_basemash1 = xc_run_tool(basemeshvoxelizer1_command, 0, 100)
            if return_code_basemash1 == 0:
                print(f'Process ({basemeshvoxelizer1_command}) executed successfully.')
            else:
                print(f'Error: The process ({basemeshvoxelizer1_command}) returned a non-zero exit code ({return_code_basemash1}).')
                return -1

    if run_upload_basemeshes:
        print(f'step for to run_upload_basemeshes')
        ##### upload basemeshes voxel database to cloud.
        
        basemeshes_result_folder_id = Workflow_Output_Result_Folder_id
        xc_process_base_meshes(api, basemeshes_output_folder, Project_id, basemeshes_result_folder_id, project_output_version)
        print(f'xc_process_base_meshes for {basemeshes_output_folder}')
        
    if run_make_caves:
        RemoveBaseMeshesdata(level0_index_cave_file_path, level0_data_cave_file_path, level1_index_cave_file_path, level1_data_cave_file_path)
        return_code_cave_mash0 = xc_run_tool(cave_meshvoxelizer0_command, 0, 100)
        if return_code_cave_mash0 == 0:
            print(f'Process ({cave_meshvoxelizer0_command}) executed successfully.')
        else:
            print(f'Error: The process ({cave_meshvoxelizer0_command}) returned a non-zero exit code ({return_code_cave_mash0}).')
            return -1
        '''
        return_code_cave_mash1 = xc_run_tool(cave_meshvoxelizer1_command, 0, 100)
        if return_code_cave_mash1 == 0:
            print(f'Process ({cave_meshvoxelizer1_command}) executed successfully.')
        else:
            print(f'Error: The process ({cave_meshvoxelizer1_command}) returned a non-zero exit code ({return_code_cave_mash1}).')
            return -1
        '''
        
    if run_upload_caves:
        print(f'step for to run_upload_caves')
        cave_meshes_result_folder_id = Workflow_Output_Result_Folder_id
        xc_process_cave_meshes(api, basemeshes_caves_db_base_folder, Project_id, cave_meshes_result_folder_id, project_output_version)
        print(f'xc_process_cave_meshes for {basemeshes_caves_db_base_folder}')

    print(f'end for step tree_instances_generation')
    return 0

def RemoveBaseMeshesdata(level0_index_db_file_path, level0_data_db_file_path, level1_index_db_file_path, level1_data_db_file_path):
    if os.path.exists(level0_index_db_file_path):
        os.remove(level0_index_db_file_path)  # Remove the file
        print(f"File '{level0_index_db_file_path}' has been removed.")
    if os.path.exists(level0_data_db_file_path):
        os.remove(level0_data_db_file_path)  # Remove the file
        print(f"File '{level0_data_db_file_path}' has been removed.")
    if os.path.exists(level1_index_db_file_path):
        os.remove(level1_index_db_file_path)  # Remove the file
        print(f"File '{level1_index_db_file_path}' has been removed.")
    if os.path.exists(level1_data_db_file_path):
        os.remove(level1_data_db_file_path)  # Remove the file
        print(f"File '{level1_data_db_file_path}' has been removed.")

def tree_config_creation(ini_path):
    #road_input_folder = f'{Data_folder}\\RoadRawInit'
    print(f'start to create tree_config_creation : {ini_path}')

    road_input_folder = f'{Data_folder}'
    road_exe_name = f'NPCTest2.exe'
    road_exe_path = os.path.join(Tools_folder, road_exe_name)
    
    basemeshes_exe_name = f'BaseMeshVoxelizer.exe'
    if not use_basemesh_ini:
        basemeshes_exe_name = f'BaseMeshVoxelizerCmd.exe'
                
    print(f'basemeshes_exe_name is {basemeshes_exe_name}')

    basemeshes_exe_path = os.path.join(Tools_folder, basemeshes_exe_name)
    worldgen_exe_name = f'WorldGen.exe'
    worldgen_exe_path = os.path.join(Tools_folder, worldgen_exe_name)
    #txt2las_exe_name = 'txt2las.exe'
    txt2las_exe_name = 'txt2las64.exe'
    txt2las_exe_path = os.path.join(Tools_folder, txt2las_exe_name)
    tree_exe_name = f'PlantsSimulation.exe'
    tree_exe_path = os.path.join(Tools_folder, tree_exe_name)
    qtree_assets_folder = Data_folder
    
    treelist_data_path = os.path.join(Data_folder, 'TreeList.csv')
    level1_treelist_data_path = os.path.join(Data_folder, 'Level1_TreeList.csv')

    road_output_folder = os.path.join(Data_folder, f'RoadObjInfo')
    smoothlayer_output_base_folder = os.path.join(Data_folder, f'sommothlayer_output')
    basemeshes_db_base_folder = os.path.join(Data_folder, f'db')
    basemeshes_caves_db_base_folder = os.path.join(Data_folder, f'cavesdb')
    basemeshes_cache_base_folder = os.path.join(Data_folder, f'cache')
    basemeshes_heightmap_folder = os.path.join(Data_folder, f'heightmap')
    tree_output_base_folder = os.path.join(Data_folder, f'tree_output')

    create_or_overwrite_empty_file(ini_path)

    create_or_update_ini_file(ini_path, section_main, 'cloud_url', Cloud_url)
    create_or_update_ini_file(ini_path, section_main, 'project_id', Project_id)
    create_or_update_ini_file(ini_path, section_main, 'tree_entity_id', Game_Tree_Entity_id)

    create_or_update_ini_file(ini_path, section_tiles, 'tiles_count', Tiles_size)
    create_or_update_ini_file(ini_path, section_tiles, 'tiles_x', Tiles_x)
    create_or_update_ini_file(ini_path, section_tiles, 'tiles_y', Tiles_y)

    create_or_update_ini_file(ini_path, section_input, 'road_input_folder', road_input_folder)
    create_or_update_ini_file(ini_path, section_input, 'road_exe_path', road_exe_path)
    create_or_update_ini_file(ini_path, section_input, 'basemeshes_exe_path', basemeshes_exe_path)
    create_or_update_ini_file(ini_path, section_input, 'worldgen_exe_path', worldgen_exe_path)
    create_or_update_ini_file(ini_path, section_input, 'txt2las_exe_path', txt2las_exe_path)
    create_or_update_ini_file(ini_path, section_input, 'tree_exe_path', tree_exe_path)
    create_or_update_ini_file(ini_path, section_input, 'qtree_assets_folder', qtree_assets_folder)
    create_or_update_ini_file(ini_path, section_input, 'treelist_data_path', treelist_data_path)
    create_or_update_ini_file(ini_path, section_input, 'level1_treelist_data_path', level1_treelist_data_path)

    create_or_update_ini_file(ini_path, section_output, 'road_output_folder', road_output_folder)
    create_or_update_ini_file(ini_path, section_output, 'smoothlayer_output_base_folder', smoothlayer_output_base_folder)
    create_or_update_ini_file(ini_path, section_output, 'basemeshes_db_base_folder', basemeshes_db_base_folder)
    create_or_update_ini_file(ini_path, section_output, 'basemeshes_caves_db_base_folder', basemeshes_caves_db_base_folder)
    create_or_update_ini_file(ini_path, section_output, 'basemeshes_cache_base_folder', basemeshes_cache_base_folder)
    create_or_update_ini_file(ini_path, section_output, 'basemeshes_heightmap_folder', basemeshes_heightmap_folder)
    create_or_update_ini_file(ini_path, section_output, 'tree_output_base_folder', tree_output_base_folder)

    create_or_update_ini_file(ini_path, section_run, 'run_road_exe', is_run_road_exe)
    create_or_update_ini_file(ini_path, section_run, 'run_worldgen_road', is_run_worldgen_road)
    create_or_update_ini_file(ini_path, section_run, 'run_upload_smooth_layer', is_run_upload_smooth_layer)
    create_or_update_ini_file(ini_path, section_run, 'run_make_basemeshes', is_run_make_basemeshes)
    create_or_update_ini_file(ini_path, section_run, 'run_upload_basemeshes', is_run_upload_basemeshes)
    create_or_update_ini_file(ini_path, section_run, 'run_make_caves', is_run_make_caves)
    create_or_update_ini_file(ini_path, section_run, 'run_upload_caves', is_run_upload_caves)
    create_or_update_ini_file(ini_path, section_run, 'run_make_tree_instances', is_run_make_tree_instances)
    create_or_update_ini_file(ini_path, section_run, 'run_upload_tree_instances', is_run_upload_tree_instances)
    create_or_update_ini_file(ini_path, section_run, 'run_create_geochem_entity', is_run_create_geochem_entity)
    create_or_update_ini_file(ini_path, section_run, 'run_generate_road_input', is_run_generate_road_input)

    create_or_update_ini_file(ini_path, section_road, 'road_Heightmap_width', Road_Input_Width)
    create_or_update_ini_file(ini_path, section_road, 'road_heightmap_height', Road_Input_Height)

    create_or_update_ini_file(ini_path, section_others, 'basemeshes_debug_level', Basemeshes_debug_level)
    create_or_update_ini_file(ini_path, section_others, 'tree_lod', Tree_load)
    create_or_update_ini_file(ini_path, section_others, 'forest_age', Forest_age)
    create_or_update_ini_file(ini_path, section_others, 'tree_iteration', Tree_iteration)
    
    print(f'end to create tree_config_creation : {ini_path}')
    ini_string = ini_file_to_string(ini_path)
    print(f'Tree config creation file content is :')
    print(f'{ini_string}')
    return

#-------Program Main Start ---------------------------------------------------------------------------------------------------------------------------------------------------------

scrap_folder= f'D:\\Downloads\\XCTreeCreation'

start_time = time.time()

now = datetime.datetime.now()

# Format the current time into a string (e.g., 2024-10-17_14-35-50)
filename = now.strftime("%Y-%m-%d_%H-%M-%S")

# Add file extension if needed (e.g., .txt)
log_file_name = f"Tree_creation_{filename}.log"
log_file_path = os.path.join(scrap_folder, log_file_name)
sys.stdout = DualOutput(log_file_path)  # Redirect stdout

print(f'This message will go to both the console and the log file : {log_file_path}.')

params = sys.argv[1:]

section_main = 'Main'
section_tiles = 'Tiles'
section_input = 'Input'
section_output = 'Output'
section_run = 'Run'
section_others = 'Others'
section_road = 'Road'
section_entity = 'Entity'
section_options = 'Options'
section_config = 'Configuration'

use_basemesh_ini = False

#lambda_host = process_lambda.process_lambda_host()

print(f'Start workflow')

print(f'scrap_folder: {scrap_folder} \n')
tools = f'D:\\Downloads\\XCTreeWorkFlow\\Tools'
print(f'system tools: {tools} \n')

workflow_project_id = '1D4CBBD1D957477E8CC3FF376FB87470' #Pangea Next Project
print(f'project_id: {workflow_project_id}')
tile_size = 25 #10
print(f'tile_size: {tile_size}')
tile_x = 12 #8
print(f'tile_x: {tile_x}')
tile_y = 11 #5
print(f'tile_y: {tile_y}')
level = 6
print(f'level: {level}')

Tree_load = 8
print(f'Tree_load: {Tree_load}')
Forest_age = 15000
print(f'foreForest_agest_age: {Forest_age}')
Tree_iteration = 300
print(f'Tree_iteration: {Tree_iteration}')

Road_Input_Width = 300
print(f'Road_Input_Width: {Road_Input_Width}')
Road_Input_Height = 300
print(f'Road_Input_Height: {Road_Input_Height}')

pythoncode_active_version_property = f'D:\\Downloads\\XCTreeWorkFlow\\PythonCode' 
treelist_active_version_property = f'D:\\Downloads\\XCTreeWorkFlow\\TreeList'
roaddata_active_version_property = f'D:\\Downloads\\XCTreeWorkFlow\\RoadData'
basemeshes_active_version_property = f'D:\\Downloads\\XCTreeWorkFlow\\BaseMeshes'
displacement_active_version_property = f'D:\\Downloads\\XCTreeWorkFlow\\DisplacementMaps'
qtree_active_version_property = f'D:\\Downloads\\XCTreeWorkFlow\\QTree'
tools_active_version_property = f'D:\\Downloads\\XCTreeWorkFlow\\Tools'
tileinfo_active_version_property = f'D:\\Downloads\\XCTreeWorkFlow\\TileInfo'

print('pythoncode_active_version_property: ' + pythoncode_active_version_property)
print('treelist_active_version_property: ' + treelist_active_version_property)
print('roaddata_active_version_property: ' + roaddata_active_version_property)
print('basemeshes_active_version_property: ' + basemeshes_active_version_property)
print('displacement_active_version_property: ' + displacement_active_version_property)
print('qtree_active_version_property: ' + qtree_active_version_property)
print('tools_active_version_property: ' + tools_active_version_property)
print('tileinfo_active_version_property: ' + tileinfo_active_version_property)

###############
Need_Copy_Assets = False
# XC Options
is_run_road_exe = True
is_run_worldgen_road = True
is_run_upload_smooth_layer = True
is_run_make_basemeshes = True
is_run_upload_basemeshes = False
is_run_make_caves = False
is_run_upload_caves = False
is_run_make_tree_instances = True
is_run_upload_tree_instances = True
is_run_create_geochem_entity = True
is_run_generate_road_input = False

Game_Tree_Entity_id = '6387B1C0BBC24AC3A7830F27807657A7' # xuan test 
#Game_Tree_Entity_id = "BE04D7A0D18142AE9D024C1A3FD50BED"  #game entity 
Workflow_Output_Result_Folder_id = '19AACA0A507546C192C4179E8370CC92' #Pangea Next > Workflow Output

tree_generation = False
basemeshes_generation = False
smooth_layer_generation = False
road_input_generation = False
whole_result_generation = False
test_tree_generation = False
test_whole_result_generation = False
basemeshes_upload_generation = False

test_only_tree_generation = False
only_tree_generation = False
only_upload_smooth_layer_generation = False
test_only_upload_tree_generation = False
only_upload_tree_generation = False
caves_voxelization_generation = False
caves_upload_generation = False

test_tree_generation = True

only_run_level_0_instances = False
only_run_level_1_instances = False

if tree_generation:
    print("Choose tree_generation to Run")
    Game_Tree_Entity_id = "BE04D7A0D18142AE9D024C1A3FD50BED"  #game entity 
    Workflow_Output_Result_Folder_id = 'C2C9E711B8A74E0FB8401646BCF3396C'  #Pangea Next > Workflow Output > Workflow Tree GeoChems Output
    is_run_road_exe = True
    is_run_worldgen_road = True
    is_run_upload_smooth_layer = True
    is_run_make_basemeshes = True
    is_run_upload_basemeshes = False
    is_run_make_caves = False
    is_run_upload_caves = False
    is_run_make_tree_instances = True
    is_run_upload_tree_instances = True
    is_run_create_geochem_entity = True
    is_run_generate_road_input = False
    
if basemeshes_generation:
    print("Choose basemeshes_generation to Run")
    Game_Tree_Entity_id = "BE04D7A0D18142AE9D024C1A3FD50BED"  #game entity 
    Workflow_Output_Result_Folder_id = '68396F90F7CE48B4BA1412EA020ED92A'  #Pangea Next > Workflow Output > Workflow BaseMeshes Output
    is_run_road_exe = False
    is_run_worldgen_road = False
    is_run_upload_smooth_layer = False
    is_run_make_basemeshes = True
    is_run_upload_basemeshes = True
    is_run_make_caves = False
    is_run_upload_caves = False
    is_run_make_tree_instances = False
    is_run_upload_tree_instances = False
    is_run_create_geochem_entity = False
    is_run_generate_road_input = False
    
if smooth_layer_generation:
    print("Choose smooth_layer_generation to Run")
    Game_Tree_Entity_id = "BE04D7A0D18142AE9D024C1A3FD50BED"  #game entity 
    Workflow_Output_Result_Folder_id = 'B24E708E13C5473FA3BFDBCBA0E68B42'  #Pangea Next > Workflow Output > Workflow Smooth layer Output
    is_run_road_exe = True
    is_run_worldgen_road = True
    is_run_upload_smooth_layer = True
    is_run_make_basemeshes = False
    is_run_upload_basemeshes = False
    is_run_make_caves = False
    is_run_upload_caves = False
    is_run_make_tree_instances = False
    is_run_upload_tree_instances = False
    is_run_create_geochem_entity = False
    is_run_generate_road_input = False
        
if road_input_generation:
    print("Choose road_input_generation to Run")
    Game_Tree_Entity_id = "BE04D7A0D18142AE9D024C1A3FD50BED"  #game entity 
    Workflow_Output_Result_Folder_id = '971A16307C4B44838A3BA72A974C1F43'  #Pangea Next > Workflow Output > Workflow Road Input Output
    is_run_road_exe = False
    is_run_worldgen_road = True
    is_run_upload_smooth_layer = False
    is_run_make_basemeshes = True
    is_run_upload_basemeshes = False
    is_run_make_caves = False
    is_run_upload_caves = False
    is_run_make_tree_instances = True
    is_run_upload_tree_instances = False
    is_run_create_geochem_entity = False
    is_run_generate_road_input = True
    
if whole_result_generation:
    print("Choose whole_result_generation to Run")
    Game_Tree_Entity_id = "BE04D7A0D18142AE9D024C1A3FD50BED"  #game entity 
    Workflow_Output_Result_Folder_id = '5D29A65F612D4EEEBFABC4ECF439A88C'  #Pangea Next > Workflow Output > Workflow Whole Result Output
    is_run_road_exe = True
    is_run_worldgen_road = True
    is_run_upload_smooth_layer = True
    is_run_make_basemeshes = True
    is_run_upload_basemeshes = True
    is_run_make_caves = True
    is_run_upload_caves = True
    is_run_make_tree_instances = True
    is_run_upload_tree_instances = True
    is_run_create_geochem_entity = True
    is_run_generate_road_input = False
    
if test_tree_generation:
    print("Choose test_tree_generation to Run")
    Game_Tree_Entity_id = "6387B1C0BBC24AC3A7830F27807657A7"  #xuan chen 
    Workflow_Output_Result_Folder_id = '82EC2324CC584DCEB3FF3281676F42A4'  #Pangea Next > Workflow Output > Workflow Tree GeoChems Output
    is_run_road_exe = True
    is_run_worldgen_road = True
    is_run_upload_smooth_layer = True
    is_run_make_basemeshes = True
    is_run_upload_basemeshes = False
    is_run_make_caves = False
    is_run_upload_caves = False
    is_run_make_tree_instances = True
    is_run_upload_tree_instances = True
    is_run_create_geochem_entity = True
    is_run_generate_road_input = False
    
if test_whole_result_generation:
    print("Choose test_whole_result_generation to Run")
    Game_Tree_Entity_id = "6387B1C0BBC24AC3A7830F27807657A7"  #xuan chen 
    Workflow_Output_Result_Folder_id = '75FDBF01261147F3A50E4A6CFDE059D3'  #Pangea Next > Workflow Output > Workflow Test Whole Result Output
    is_run_road_exe = True
    is_run_worldgen_road = True
    is_run_upload_smooth_layer = True
    is_run_make_basemeshes = True
    is_run_upload_basemeshes = True
    is_run_make_caves = False
    is_run_upload_caves = True
    is_run_make_tree_instances = True
    is_run_upload_tree_instances = True
    is_run_create_geochem_entity = True
    is_run_generate_road_input = False
    
if basemeshes_upload_generation:
    print("Choose basemeshes_upload_generation to Run")
    Game_Tree_Entity_id = "BE04D7A0D18142AE9D024C1A3FD50BED"  #game entity 
    Workflow_Output_Result_Folder_id = '68396F90F7CE48B4BA1412EA020ED92A'  #Pangea Next > Workflow Output > Workflow BaseMeshes Output
    is_run_road_exe = False
    is_run_worldgen_road = False
    is_run_upload_smooth_layer = False
    is_run_make_basemeshes = False
    is_run_upload_basemeshes = True
    is_run_make_caves = False
    is_run_upload_caves = False
    is_run_make_tree_instances = False
    is_run_upload_tree_instances = False
    is_run_create_geochem_entity = False
    is_run_generate_road_input = False
    
if test_only_tree_generation:
    print("Choose test_only_tree_generation to Run")
    Game_Tree_Entity_id = "6387B1C0BBC24AC3A7830F27807657A7"  #xuan chen 
    Workflow_Output_Result_Folder_id = '82EC2324CC584DCEB3FF3281676F42A4'  #Pangea Next > Workflow Output > Workflow Test Tree GeoChems Output
    is_run_road_exe = False
    is_run_worldgen_road = False
    is_run_upload_smooth_layer = False
    is_run_make_basemeshes = False
    is_run_upload_basemeshes = False
    is_run_make_caves = False
    is_run_upload_caves = False
    is_run_make_tree_instances = True
    is_run_upload_tree_instances = True
    is_run_create_geochem_entity = True
    is_run_generate_road_input = False
    
if only_tree_generation:
    print("Choose only_tree_generation to Run")
    Game_Tree_Entity_id = "BE04D7A0D18142AE9D024C1A3FD50BED"  #game entity 
    Workflow_Output_Result_Folder_id = 'C2C9E711B8A74E0FB8401646BCF3396C'  #Pangea Next > Workflow Output > Workflow Tree GeoChems Output
    is_run_road_exe = False
    is_run_worldgen_road = False
    is_run_upload_smooth_layer = False
    is_run_make_basemeshes = False
    is_run_upload_basemeshes = False
    is_run_make_caves = False
    is_run_upload_caves = False
    is_run_make_tree_instances = True
    is_run_upload_tree_instances = True
    is_run_create_geochem_entity = True
    is_run_generate_road_input = False
    
if test_only_upload_tree_generation:
    print("Choose test_only_upload_tree_generation to Run")
    Game_Tree_Entity_id = "6387B1C0BBC24AC3A7830F27807657A7"  #xuan chen 
    Workflow_Output_Result_Folder_id = '82EC2324CC584DCEB3FF3281676F42A4'  #Pangea Next > Workflow Output > Workflow Test Tree GeoChems Output
    is_run_road_exe = False
    is_run_worldgen_road = False
    is_run_upload_smooth_layer = False
    is_run_make_basemeshes = False
    is_run_upload_basemeshes = False
    is_run_make_caves = False
    is_run_upload_caves = False
    is_run_make_tree_instances = False
    is_run_upload_tree_instances = True
    is_run_create_geochem_entity = True
    is_run_generate_road_input = False
    
if only_upload_tree_generation:
    print("Choose only_upload_tree_generation to Run")
    Game_Tree_Entity_id = "BE04D7A0D18142AE9D024C1A3FD50BED"  #game entity 
    Workflow_Output_Result_Folder_id = 'C2C9E711B8A74E0FB8401646BCF3396C'  #Pangea Next > Workflow Output > Workflow Tree GeoChems Output
    is_run_road_exe = False
    is_run_worldgen_road = False
    is_run_upload_smooth_layer = False
    is_run_make_basemeshes = False
    is_run_upload_basemeshes = False
    is_run_make_caves = False
    is_run_upload_caves = False
    is_run_make_tree_instances = False
    is_run_upload_tree_instances = True
    is_run_create_geochem_entity = True
    is_run_generate_road_input = False
    
if only_upload_smooth_layer_generation:
    print("Choose only_upload_smooth_layer_generation to Run")
    Game_Tree_Entity_id = "BE04D7A0D18142AE9D024C1A3FD50BED"  #game entity 
    Workflow_Output_Result_Folder_id = 'B24E708E13C5473FA3BFDBCBA0E68B42'  #Pangea Next > Workflow Output > Workflow Smooth layer Output
    is_run_road_exe = False
    is_run_worldgen_road = False
    is_run_upload_smooth_layer = True
    is_run_make_basemeshes = False
    is_run_upload_basemeshes = False
    is_run_make_caves = False
    is_run_upload_caves = False
    is_run_make_tree_instances = False
    is_run_upload_tree_instances = False
    is_run_create_geochem_entity = False
    is_run_generate_road_input = False
    
if caves_voxelization_generation:
    print("Choose only_upload_smooth_layer_generation to Run")
    Game_Tree_Entity_id = "BE04D7A0D18142AE9D024C1A3FD50BED"  #game entity 
    Workflow_Output_Result_Folder_id = '68396F90F7CE48B4BA1412EA020ED92A'  #Pangea Next > Workflow Output > Workflow BaseMeshes Output
    is_run_road_exe = False
    is_run_worldgen_road = False
    is_run_upload_smooth_layer = False
    is_run_make_basemeshes = False
    is_run_upload_basemeshes = False
    is_run_make_caves = True
    is_run_upload_caves = True
    is_run_make_tree_instances = False
    is_run_upload_tree_instances = False
    is_run_create_geochem_entity = False
    is_run_generate_road_input = False
    
if caves_upload_generation:
    print("Choose only_upload_smooth_layer_generation to Run")
    Game_Tree_Entity_id = "BE04D7A0D18142AE9D024C1A3FD50BED"  #game entity 
    Workflow_Output_Result_Folder_id = '68396F90F7CE48B4BA1412EA020ED92A'  #Pangea Next > Workflow Output > Workflow BaseMeshes Output
    is_run_road_exe = False
    is_run_worldgen_road = False
    is_run_upload_smooth_layer = False
    is_run_make_basemeshes = False
    is_run_upload_basemeshes = False
    is_run_make_caves = False
    is_run_upload_caves = True
    is_run_make_tree_instances = False
    is_run_upload_tree_instances = False
    is_run_create_geochem_entity = False
    is_run_generate_road_input = False

print(f'is_run_road_exe: {is_run_road_exe}')
print(f'is_run_worldgen_road: {is_run_worldgen_road}')
print(f'is_run_upload_smooth_layer: {is_run_upload_smooth_layer}')
print(f'is_run_make_basemeshes: {is_run_make_basemeshes}')
print(f'is_run_upload_basemeshes: {is_run_upload_basemeshes}')
print(f'is_run_make_caves: {is_run_make_caves}')
print(f'is_run_upload_caves: {is_run_upload_caves}')
print(f'is_run_make_tree_instances: {is_run_make_tree_instances}')
print(f'is_run_upload_tree_instances: {is_run_upload_tree_instances}')
print(f'is_run_create_geochem_entity: {is_run_create_geochem_entity}')
print(f'is_run_generate_road_input: {is_run_generate_road_input}')

print(f'is_run_make_tree_instances is: {is_run_make_tree_instances}')

print('Start to download files')
pythoncode_data_folder = pythoncode_active_version_property
treelist_data_folder = treelist_active_version_property
#treelist_data_path = os.path.join(treelist_data_folder, 'TreeList.csv')
roaddata_data_path = roaddata_active_version_property
basemeshes_data_path = basemeshes_active_version_property
displacement_data_path = displacement_active_version_property
qtree_data_path = qtree_active_version_property
tools_data_path = tools_active_version_property
tileinfo_data_path = tileinfo_active_version_property

print('pythoncode_data_folder: ' + pythoncode_data_folder)
print('treelist_data_folder: ' + treelist_data_folder)
print('roaddata_data_path: ' + roaddata_data_path)
print('basemeshes_data_path: ' + basemeshes_data_path)
print('displacement_data_path: ' + displacement_data_path)
print('qtree_data_path: ' + qtree_data_path)
print('tools_data_path: ' + tools_data_path)
print('tileinfo_data_path: ' + tileinfo_data_path)

#Tree_Data_Folder_Name = 'Tree_Instances_Creation'
Tree_Data_Folder_Name = 'Tree_Big_Creation'
Data_folder = os.path.join(scrap_folder, Tree_Data_Folder_Name)
g_Lambda_Info_ini_name = 'lambda_info.ini'
g_Lambda_Info_ini_path = os.path.join(Data_folder, g_Lambda_Info_ini_name)
print(f'Data_folder: {Data_folder}')
if not os.path.exists(Data_folder):
    os.makedirs(Data_folder)
#Tools_folder = os.path.join(Data_folder, 'Tools')
#Tools_folder = f'{Data_folder}\\Tools'
Tools_folder = tools_data_path
print(f'Tools_folder: {Tools_folder}')
if not os.path.exists(Tools_folder):
    os.makedirs(Tools_folder)

print('Start to copy files')

print(f'start to copy from {tileinfo_data_path} to {Data_folder}')
copy_files_in_folder(tileinfo_data_path, Data_folder)
print(f'end to copy from {tileinfo_data_path} to {Data_folder}')
print(f'start to copy from {roaddata_data_path} to {Data_folder}')
copy_files_in_folder(roaddata_data_path, Data_folder)
print(f'end to copy from {roaddata_data_path} to {Data_folder}')

if Need_Copy_Assets:
    print(f'start to copy from {pythoncode_data_folder} to {Data_folder}')
    copy_files_in_folder(pythoncode_data_folder, Data_folder)
    print(f'end to copy from {pythoncode_data_folder} to {Data_folder}')
    print(f'start to copy from {treelist_data_folder} to {Data_folder}')
    copy_files_in_folder(treelist_data_folder, Data_folder)
    print(f'end to copy from {treelist_data_folder} to {Data_folder}')
    
    print(f'start to copy from {basemeshes_data_path} to {Data_folder}')
    copy_files_in_folder(basemeshes_data_path, Data_folder)
    print(f'end to copy from {basemeshes_data_path} to {Data_folder}')
    print(f'start to copy from {displacement_data_path} to {Data_folder}')
    copy_files_in_folder(displacement_data_path, Data_folder)
    print(f'end to copy from {displacement_data_path} to {Data_folder}')
    print('Start to copy big files')
    print(f'start to copy from {qtree_data_path} to {Data_folder}')
    copy_files_in_folder(qtree_data_path, Data_folder)
    print(f'end to copy from {qtree_data_path} to {Data_folder}')
    
print('End to copy files')  

Tile_Info_ini_name = 'TileInfo.ini'
Tile_Info_ini_path = os.path.join(Data_folder, Tile_Info_ini_name)
if os.path.exists(Tile_Info_ini_path):
    tile_size = read_ini_value(Tile_Info_ini_path, section_tiles, 'Tiles_Count', value_type=int)
    tile_x = read_ini_value(Tile_Info_ini_path, section_tiles, 'Tiles_X_Index', value_type=int)
    tile_y = read_ini_value(Tile_Info_ini_path, section_tiles, 'Tiles_Y_Index', value_type=int)
    print(f'Tiles_Count of {Tile_Info_ini_path} is {tile_size}')
    print(f'Tiles_X_Index of {Tile_Info_ini_path} is {tile_x}')
    print(f'Tiles_Y_Index of {Tile_Info_ini_path} is {tile_y}')

print('Start to get input parameters')
Cloud_url = 'https://demo.voxelfarm.com/'
Project_id = workflow_project_id

print(f'Game_Tree_Entity_id: {Game_Tree_Entity_id}')

print(f'Workflow_Output_Result_Folder_id: {Workflow_Output_Result_Folder_id}')

Tiles_size = tile_size if tile_size else 10
Tiles_x = tile_x if tile_x else 8
Tiles_y = tile_y if tile_y else 5
print(f'Tiles_size: {Tiles_size}')
print(f'Tiles_x: {Tiles_x}')
print(f'Tiles_y: {Tiles_y}')
Basemeshes_debug_level = level if level else 6
print(f'Basemeshes_debug_level: {Basemeshes_debug_level}')
configfile_path = f'{Data_folder}\\TreeInstancesCreationConfig.ini'
#configfile_path = params[0]
print(f'Tree instance generation configfile_path: {configfile_path}')
print(f'Tree instance generation config file : {configfile_path}')

run_result = 0

print('Start to config files')
print(f'start tree_config_creation: {configfile_path}')
tree_config_creation(configfile_path)
print(f'end tree_config_creation: {configfile_path}')
print(f'start tree_instances_generation: {configfile_path}')
run_result = tree_instances_generation(configfile_path)
print(f'end tree_instances_generation: {configfile_path}')


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
print(f'Tree instance generation Whole Execution time : {formatted_time}')
print(f'Whole Execution time :", {formatted_time}')

if run_result == 0:
    print('QuadTree lambda finished')
else:
    print('QuadTree lambda failed')

#exit()