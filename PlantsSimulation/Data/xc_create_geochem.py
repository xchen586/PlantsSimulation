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

import pandas as pd

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

def merge_csv_files(folder_path, output_file, columns_to_merge):
    # List all CSV files in the folder
    csv_files = [file for file in os.listdir(folder_path) if file.endswith('.csv')]

    # Initialize an empty list to store DataFrames
    dfs = []

    # Read each CSV file, select specific columns, and append its DataFrame to the list
    for file in csv_files:
        file_path = os.path.join(folder_path, file)
        df = pd.read_csv(file_path, usecols=columns_to_merge)
        dfs.append(df)

    # Concatenate all DataFrames into a single DataFrame
    merged_df = pd.concat(dfs, ignore_index=True)
     # Write the merged DataFrame to a CSV file
    merged_df.to_csv(output_file, index=False)

TREE_INSTANCE = 1
POI_INSTANCE = 2
InstanceType_Attribute = 'InstanceType'
Variant_Attribute = 'Variant'

def calculate_id_for_instance(instance_type, tree_index, poi_index):
    # Calculate the extra column value based on the instance type and indices
    instance_string = 'Others'
    index = 0

    if instance_type == TREE_INSTANCE:
        instance_string = 'Tree'
        index = tree_index
    elif instance_type == POI_INSTANCE:
        instance_string = 'POI'
        index = poi_index
    
    extra_value = f'{instance_string} {index}'
    return extra_value

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
        if instance_type == TREE_INSTANCE:
            extra_id = current_tree_id
            current_tree_id += 1
        elif instance_type == POI_INSTANCE:
            extra_id = current_poi_id
            current_poi_id += 1
        else:
            extra_id = 0
        return calculate_id_for_instance(instance_type, extra_id, extra_id)
    
    merged_df[extra_column_name] = merged_df.apply(update_id, axis=1)

    # Write the updated DataFrame to a new CSV file
    merged_df.to_csv(output_file, index=False)

def xc_process_files_entity(api : voxelfarmclient.rest, project_id, folder_id, raw_entity_type, entity_type, folder_path, name : str, version : int, color : bool):

    if not os.path.exists(folder_path):
        print(f'File {folder_path} does not exist')
        return
    
    # Use the os.listdir() function to get a list of filenames in the folder
    file_names = os.listdir(folder_path)

    # Create a list of file paths by joining the folder path with each file name
    file_paths = [os.path.join(folder_path, file_name) for file_name in file_names]    
    print(file_paths)

    result = api.get_project_crs(project_id)
    crs = result.crs
    
    result = api.create_entity_raw(project=project_id, 
        type = raw_entity_type,
        name=f'{name}', 
        fields={
            'file_folder': folder_id,
        }, crs = crs)
    entity_id = result.id
    print(f'Attaching file {file_paths} to entity {entity_id}')
    #api.attach_files(project=project_id, id=entity_id, files={'file': file})
    for file_path in file_paths:
        with open(file_path, "rb") as file:
            api.attach_files(project=project_id, id=entity_id, files={'file': file})

    result = api.create_entity_processed(project=project_id, 
        type = entity_type,
        name=f'{name}', 
        fields={
            'source': entity_id,
            'source_type': raw_entity_type,
            'file_folder': folder_id,
            #'source_ortho' if color else '_source_ortho': entity_id
        }, crs = crs)
    print(f'--------Created entity {result.id} for {name} {version}--------')



Cloud_url = 'http://52.226.195.5/'
api = voxelfarmclient.rest(Cloud_url)
project_id = '1D4CBBD1D957477E8CC3FF376FB87470'
geochems_folder_id = '36F2FD37D03B4DDE8C2151438AA47804'
Tiles_size = 10
Tiles_x = 8
Tiles_y = 5
    
csvfiles_folder_path = 'D:\\Downloads\\XCTreeCreation\\tree_output\\10_8_5\\instanceoutput'
geo_chemical_folder = 'D:\\Downloads\\XCTreeCreation\\tree_output\\10_8_5\\GeoChemical'
merged_csv_name = f'{Tiles_size}_{Tiles_x}_{Tiles_y}_geo_merged.csv'
merged_csv_path = os.path.join(geo_chemical_folder, merged_csv_name)
X_Attribute = 'XWorld'
Y_Attribute = 'YWorld'
Z_Attribute = 'ZWorld'

columns_to_merge = [X_Attribute, Y_Attribute, Z_Attribute, InstanceType_Attribute, Variant_Attribute]  # Specify the columns you want to merge
extra_column_name = 'Id'

#print('Start to Merge the csv files {csvfiles_folder_path} to {merged_csv_path}')
#merge_csv_files(csvfiles_folder_path, merged_csv_path, columns_to_merge)
print('Start to Add Id field to  the csv file {merged_csv_path}')
add_extra_column_to_csv(merged_csv_path, merged_csv_path, extra_column_name)
print('End with raw data file {merged_csv_path}')

geo_meta_name = 'process.meta'
geo_meta_path = os.path.join(geo_chemical_folder, geo_meta_name)
section_config = 'Configuration'

print('Start with geochem meta file {geo_meta_path}')

create_or_overwrite_empty_file(geo_meta_path)
create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile', merged_csv_name)
create_or_update_ini_file(geo_meta_path, section_config, 'SampleFile_ID', 5)
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

project_entity = api.get_entity(project_id)
version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
api.update_entity(project=project_id, id=project_id, fields={'version': version})
result = api.create_folder(project=project_id, name=f'Version {version}', folder=geochems_folder_id)
if not result.success:
    print(f'Failed to create folder for version!')
    exit(4)
geochems_folder_id = result.id
print(f'-----------------Successful to create folder {geochems_folder_id} for version!-----------------')

print('Start with create geo chem entity')

xc_process_files_entity(api, project_id, geochems_folder_id, api.entity_type.RawGeoChem, api.entity_type.GeoChem, geo_chemical_folder, f'GeoChemical_instances_{Tiles_size}_{Tiles_x}_{Tiles_y}-{version}', version=version, color=True)

print('End with create geo chem entity')


