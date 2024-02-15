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
        instance_type = row['InstanceType']
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

csvfiles_folder_path = 'D:\\Downloads\\XCTreeCreation\\tree_output\\10_8_5\\instanceoutput'
merged_csv_path = 'D:\\Downloads\\XCTreeCreation\\tree_output\\10_8_5\\merged.csv'
columns_to_merge = ['X', 'Y', 'Z', 'InstanceType', 'Variant']  # Specify the columns you want to merge
extra_column_name = 'Id'

print('Start to Merge the csv files {csvfiles_folder_path} to {merged_csv_path}')
merge_csv_files(csvfiles_folder_path, merged_csv_path, columns_to_merge)
print('Start to Add Id field to  the csv file {merged_csv_path}')
add_extra_column_to_csv(merged_csv_path, merged_csv_path, extra_column_name)
print('End with raw data file {merged_csv_path}')




