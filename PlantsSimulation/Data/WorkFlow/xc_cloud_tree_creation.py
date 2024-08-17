import os
from os import path
import sys
import time
import glob

import shutil
import subprocess
import argparse
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

def is_valid_float_string(s):
    try:
        float(s)
        return True
    except ValueError:
        return False
    
def xc_run_tool(tool_path, progress_start, progress_end):    
    
    lambda_host.log(f'run tree tool_path:\n{tool_path}')
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
                            lambda_host.log(f'Cannot convert input progress tokens[1] {progress_string} to float:')
                        progress = start + tool_progress * scale
                        message = ""
                        for token in tokens[2:]:                   
                            message += token + " "
                        lambda_host.progress(progress, message)
                if progress == -1:
                    lambda_host.log(realtime_output.replace('\n', ''))
            else: 
                poll = tool_process.poll() 
                if poll is not None:
                    break
    except Exception as e:
        exception_message = str(e)
        exception_repr_message = repr(e)
        lambda_host.log(f'exception_message of xc_run_tool for {tool_path} with exception of {exception_message}')
        lambda_host.log(f'exception_repr_message of xc_run_tool {tool_path} with repr message of {exception_repr_message}')
    
    return tool_process.returncode 

#---------------------------------------------------------------------------------------------------------------------------------------------------------------

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

def xc_process_files_entity(api : voxelfarmclient.rest, project_id, folder_id, raw_entity_type, entity_type, folder_path, name : str, version : int, color : bool, zipped:bool = False):

    lambda_host.log(f'start to result = api.creation_result()')
    result = api.creation_result()
    lambda_host.log(f'end to result = api.creation_result()')

    lambda_host.log(f'start to os.path.exists({folder_path})')
    if not os.path.exists(folder_path):
        lambda_host.log(f'File {folder_path} does not exist')
        result.success = False
        result.error_info = f'File {folder_path} does not exist'
        return result
    lambda_host.log(f'end to os.path.exists({folder_path})')

    # Use the os.listdir() function to get a list of filenames in the folder
    lambda_host.log(f'start to os.listdir({folder_path})')
    file_names = os.listdir(folder_path)
    lambda_host.log(f'end to os.listdir({folder_path})')

    # Create a list of file paths by joining the folder path with each file name
    file_paths = [os.path.join(folder_path, file_name) for file_name in file_names]   
    delimiter = ' '  # You can specify any delimiter you want, e.g., ',' or '-'
    # Using join() method
    file_paths_string = delimiter.join(file_paths)
    lambda_host.log(file_paths_string)

    lambda_host.log(f'start to api.get_project_crs({project_id})')
    result = api.get_project_crs(project_id)
    crs = result.crs
    if not result.success:
        return result
    lambda_host.log(f'end to api.get_project_crs({project_id})')
    
    lambda_host.log(f'start to api.create_entity_raw')
    result = api.create_entity_raw(project=project_id, 
        type = raw_entity_type,
        name=f'{name}', 
        fields={
            'file_folder': folder_id,
        }, crs = crs)
    entity_id = result.id
    if not result.success:
        return result
    lambda_host.log(f'end to api.create_entity_raw')

    lambda_host.log(f'Attaching file {file_paths} to entity {entity_id}')
    if zipped:
        zip_path = f'{folder_path}_zip.zip'
        lambda_host.log(f'start to zip folder {folder_path} to entity {zip_path}')
        canzip = zip_folder(folder_path, zip_path)
        lambda_host.log(f'end to zip folder {folder_path} to entity {zip_path}')
        if canzip:
            with open(zip_path, 'rb') as zfile:
                try:
                    #result = api.attach_files(project=project_id, id=entity_id, files={'file': (zip_path, zfile, 'application/zip')})
                    result = api.attach_files(project=project_id, id=entity_id, files={'file': zfile})
                except Exception as e:
                    lambda_host.log("An error occurred:", e)
                if not result.success:
                    lambda_host.log(result.error_info)
                    return result
            lambda_host.log(f'attach zip file {zip_path} for entity {entity_id}')
        else:
            result.success = False
            result.error_info = f'Fail to zip {folder_path} as file'
            return result
    else:
        for file_path in file_paths:
            with open(file_path, "rb") as file:
                result = api.attach_files(project=project_id, id=entity_id, files={'file': file})
                if not result.success:
                    lambda_host.log(result.error_info)
                    return result
                lambda_host.log(f'attach file {file_path} for entity {entity_id}')

    
    #api.attach_files(project=project_id, id=entity_id, files={'file': file})
    for file_path in file_paths:
        with open(file_path, "rb") as file:
            api.attach_files(project=project_id, id=entity_id, files={'file': file})
            lambda_host.log(f'attach file {file_path} for entity {entity_id}')

    lambda_host.log(f'start to only create the raw entity {entity_id}')
    if raw_entity_type == entity_type:
        lambda_host.log(f'only create the raw entity {entity_id}')
        return result
    lambda_host.log(f'start to only create the raw entity {entity_id}')

    lambda_host.log(f'start to api.create_entity_processed')
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
        lambda_host.log(f'--------Fail to create entity {result.id} for {name} {version}--------')
        return result
    lambda_host.log(f'end to api.create_entity_processed')

    lambda_host.log(f'--------Created entity {result.id} for {name} {version}--------')
    return result

def create_geochem_tree_entity(api, geo_chemical_folder):
    extra_column_name = 'Id'
    geochems_project_id = '1D4CBBD1D957477E8CC3FF376FB87470'
    geochems_folder_id = '36F2FD37D03B4DDE8C2151438AA47804'

    merged_csv_name = f'{Tiles_size}_{Tiles_x}_{Tiles_y}_geo_merged.csv'
    merged_csv_path = os.path.join(geo_chemical_folder, merged_csv_name)
    geo_meta_name = 'process.meta'
    geo_meta_path = os.path.join(geo_chemical_folder, geo_meta_name)

    lambda_host.log('Start to Add Id field to  the csv file {merged_csv_path}')
    add_extra_column_to_csv(merged_csv_path, merged_csv_path, extra_column_name)
    lambda_host.log('End with raw data file {merged_csv_path}')

    lambda_host.log('Start with geochem meta file {geo_meta_path}')

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
    geo_meta_string = ini_file_to_string(geo_meta_path)
    lambda_host.log(f'Geo meta file content is :')
    lambda_host.log(f'{geo_meta_string}')

    geochems_project_entity = api.get_entity(geochems_project_id)
    version = int(geochems_project_entity['version']) + 1 if 'version' in geochems_project_entity else 1
    api.update_entity(project=geochems_project_id, id=geochems_project_id, fields={'version': version})
    result = api.create_folder(project=geochems_project_id, name=f'Version {version}', folder=geochems_folder_id)
    if not result.success:
        lambda_host.log(f'Failed to create geochem folder for version!')
        exit(4)
    geochems_folder_id = result.id
    lambda_host.log(f'-----------------Successful to create geochem folder {geochems_folder_id} for version!-----------------')

    lambda_host.log('Start with create geo chem entity')

    geochem_entity_name = f'GeoChemical_instances_{Tiles_size}_{Tiles_x}_{Tiles_y}-{version}'
    result = xc_process_files_entity(api, geochems_project_id, geochems_folder_id, api.entity_type.RawGeoChem, api.entity_type.GeoChem, geo_chemical_folder, geochem_entity_name, version=version, color=True)
    if not result.success:
        lambda_host.log(f'Failed to create geochem entity {geochem_entity_name} with {api} basemeshes_result_project_id: {geochems_project_id} geo_chemical_folder: {geo_chemical_folder} raw: api.entity_type.RawGeoChem index: api.entity_type.GeoChem version: {version} !')
        exit(4)
    lambda_host.log('End with create geo chem entity')

#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def on_upload_db_succeessfull(vf, project_id, entity_id, output_dir):
    #lambda_host.progress(99, 'Uploading results')

    # save entity properties and complete state
    properties_file = f'{output_dir}/properties.vf'

    if path.exists(properties_file):
        entity_data= {}
        config = configparser.ConfigParser()
        config.read(properties_file)
        lambda_host.log('XC Entity Properties')

        section = config.sections()[0]

        for key in config[section]:  
            entity_data[key]=config[section][key]
            lambda_host.log(f'  {key}={entity_data[key]}')

        # update entity
        result = vf.update_entity(
            id=entity_id,
            project=project_id, 
            fields=entity_data)

    unified_file = f'{output_dir}/unified.vf'
    if path.exists(unified_file):
        lambda_host.upload(unified_file, "extended.meta", entity_id)
    else:
        lambda_host.log("XC extended.meta File not found")

#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def do_upload_base_meshes(api : voxelfarmclient.rest, project_id, basemeshes_db_folderId, file_path : str, version : int, entity_name : str, code_path : str):
    lambda_host.log(f'Start do_upload_base_meshes Created entity {entity_name}')
 
    result = api.get_project_crs(project_id)
    crs = result.crs
    entity_id = None
    index_vf_path = os.path.join(file_path, 'index.vf')
    with open(index_vf_path, 'rb') as f:
        lambda_host.log(f'start create_entity_raw file for entity {entity_name}')
        result = api.create_entity_raw(project=project_id, 
            type=api.entity_type.VoxelPC, 
            name=entity_name, 
            fields={
                'state': 'PARTIAL',
                'file_folder': basemeshes_db_folderId,
            }, crs = crs)
        entity_id = result.id
        if not result.success:
            lambda_host.log(f'Fail to create_entity_raw Created entity for {entity_name} : {result.error_info}')
        else:
            lambda_host.log(f'Successfully to create_entity_raw Created entity for {result.id} for {entity_name}')
        index_vf_size = os.path.getsize(index_vf_path)
        lambda_host.log(f'Attaching file index vf {index_vf_path} with size of {index_vf_size} to entity {entity_id}')
        try:
            result = api.attach_files(project=project_id, id=entity_id, files={'file': f})
        except Exception as e:
            lambda_host.log(f'Exception of Attach_files index vf to 1 attach file {index_vf_path} to entity {entity_id} with exception of {str(e)}')
        if not result.success:
            lambda_host.log(f'Failed to 1 attach file {index_vf_path} to entity {entity_id} with error of {result.error_info}')
            return

    data_vf_path = os.path.join(file_path, 'data.vf')
    with open(data_vf_path, 'rb') as f:
        data_vf_size = os.path.getsize(data_vf_path)
        lambda_host.log(f'Attaching file data vf {data_vf_path} with size of {data_vf_size} to entity {entity_id}')
        try: 
            result = api.attach_files(project=project_id, id=entity_id, files={'file': f})
        except Exception as e:
            lambda_host.log(f'Exception of Attach_files data vf to 1 attach file {data_vf_path} to entity {entity_id} with exception of {str(e)}')
        if not result.success:
            lambda_host.log(f'Failed to 2 attach file {data_vf_path} to entity {entity_id} with error of {result.error_info}')
            return
    lambda_host.log(f'Start to upload_db entity_id : {entity_id} ---- with folder ; {file_path}')

    try:
        uploadDbOk = lambda_host.upload_db(entity_id, file_path, 'vox', 'Voxel Data')
    except Exception as e:
        lambda_host.log(f'Exception of lambda_host.upload_db: files folder: {file_path} to entity {entity_id} with exception of {str(e)}')   
    if uploadDbOk:
        lambda_host.log(f'lambda_host.upload_db is successful in do_upload_base_meshes with {file_path} to entity {entity_id}')
        on_upload_db_succeessfull(api, project_id, entity_id, file_path)
        result = api.update_entity(
        id = entity_id,
        project = project_id, 
        fields = {
            'state' : 'COMPLETE'
        })
    else:
        lambda_host.log(f'lambda_host.upload_db is failed in do_upload_base_meshes with {file_path} to entity {entity_id}')
        result = api.update_entity(
        id = entity_id,
        project = project_id, 
        fields = {
            'state' : 'ERROR'
        })
        #exit_code(3) 

    if not result.success:
        lambda_host.log(result.error_info)
        exit(3)

    lambda_host.log(f'End do_upload_base_meshes Created entity {result.id} for {entity_name}')
    
#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def do_swarm_db_upload(projectId, itemId, outputDBFolder, dbName, dbTitle):
    # Log progress for creating configuration file
    lambda_host.progress(85, 'XC Create configuration file for uploading dataset')
    
    # Define organization ID
    OrgId = 2343243456678890
    
    # Create configuration string
    uploaddb_cfg = f'''
[Configuration]
Organization={OrgId}
Instance={itemId}
Project={projectId}
OutputFolder={outputDBFolder}
dbName={dbName}
dbTitle={dbTitle}
azure_container_name=vfcloudstorage
azure_storage_connection_string=DefaultEndpointsProtocol=https;AccountName=vfstpangea;AccountKey=qo+5MnyJBELDbjQUBIOyl7mlyg9FlYnz7XShIyao2wd6Et+vVNMv3Szuvc5uY++zhba8TaWq/uXc+AStuouKIQ==;EndpointSuffix=core.windows.net
storage_provider=AZURE
'''
    # Log the configuration string
    lambda_host.log(f'XC Tool.UploadDB.exe surveys configuration ini file content is : ')
    lambda_host.log(f'{uploaddb_cfg}')
    
    # Create path for the ini file
    ini_file = os.path.join(scrap_folder, f'uploaddb_{dbName}.ini')
    lambda_host.log(f'XC Create surveys configuration file {ini_file}')
    
    # Write configuration string to the ini file
    with open(ini_file, "w") as ini:
        ini.write(uploaddb_cfg)
    
    # Log the configuration string again
    lambda_host.log(f'{uploaddb_cfg}')
    
    # Define swarm index and size
    swarmIndex = 0
    swarmSize = 1
    
    # Construct the path for the upload tool
    uploaddb_path = f'{tools}\\Tool.UploadDB.exe {ini_file} {swarmIndex} {swarmSize}'

    # Attach the configuration file for logging
    lambda_host.attach_file(ini_file)

    # Execute the upload tool and time the operation
    start = timer()
    return_code = xc_run_tool(uploaddb_path, 86, 98)
    end = timer()
    
    # Log the duration and exit code of the upload operation
    lambda_host.log(f'XC Swarm UploadDB: {timedelta(seconds=end - start)}, exit code: {return_code}')
    
    # Return the exit code
    return return_code
#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def do_upload_base_meshes_swarm(api : voxelfarmclient.rest, project_id, basemeshes_db_folderId, file_path : str, version : int, entity_name : str, code_path : str):
    lambda_host.log(f'Start do_upload_base_meshes_swarm Created entity {entity_name}')

    result = api.get_project_crs(project_id)
    crs = result.crs
    entity_id = None
    index_vf_path = os.path.join(file_path, 'index.vf')
    with open(index_vf_path, 'rb') as f:
        lambda_host.log(f'start create_entity_raw file for entity {entity_name}')
        result = api.create_entity_raw(project=project_id, 
            type=api.entity_type.VoxelPC, 
            name=entity_name, 
            fields={
                'state': 'PARTIAL',
                'file_folder': basemeshes_db_folderId,
            }, crs = crs)
        entity_id = result.id
        if not result.success:
            lambda_host.log(f'Fail to create_entity_raw Created entity for {entity_name} : {result.error_info}')
        else:
            lambda_host.log(f'Successfully to create_entity_raw Created entity for {result.id} for {entity_name}')
        index_vf_size = os.path.getsize(index_vf_path)
        lambda_host.log(f'Attaching file index vf {index_vf_path} with size of {index_vf_size} to entity {entity_id}')
        try:
            result = api.attach_files(project=project_id, id=entity_id, files={'file': f})
        except Exception as e:
            exception_message = str(e)
            lambda_host.log(f'Exception of Attach_files index vf to 1 attach file {index_vf_path} to entity {entity_id} with exception of {exception_message}')
        if not result.success:
            lambda_host.log(f'Failed to 1 attach file {index_vf_path} to entity {entity_id} with error of {result.error_info}')
            #return

    data_vf_path = os.path.join(file_path, 'data.vf')
    with open(data_vf_path, 'rb') as f:
        data_vf_size = os.path.getsize(data_vf_path)
        lambda_host.log(f'Attaching file data vf {data_vf_path} with size of {data_vf_size} to entity {entity_id}')
        try: 
            result = api.attach_files(project=project_id, id=entity_id, files={'file': f})
        except Exception as e:
            exception_message = str(e)
            exception_repr_message = repr(e)
            lambda_host.log(f'Exception of Attach_files data vf to 1 attach file {data_vf_path} to entity {entity_id} with exception of {exception_message}')
            lambda_host.log(f'Exception of Attach_files data vf exception repr message of {exception_repr_message}')
        if not result.success:
            lambda_host.log(f'Failed to 2 attach file {data_vf_path} to entity {entity_id} with error of {result.error_info}')
            #return
    lambda_host.log(f'Start to do_swarm_db_upload entity_id : {entity_id} ---- with folder ; {file_path}')
    
    uploadcode = None
    try:
        # Attempt to upload the database
        dbName = f'vox-mesh-{entity_name}'
        dbTitle = f'Voxel Mesh Data For {entity_name}'
        uploadcode = do_swarm_db_upload(project_id, entity_id, file_path, dbName, dbTitle)
    except Exception as e:
        # Log any exceptions that occur during the upload
        lambda_host.log(f'Exception during do_swarm_db_upload: files folder: {file_path} to entity {entity_id} with exception: {str(e)}')
    
    # Check if uploadcode was assigned
    if uploadcode is not None:
        # Proceed with further logic if needed
        lambda_host.log(f'Upload completed with exit code: {uploadcode}')
    else:
        # Handle the case where uploadcode was not assigned (likely due to an exception)
        lambda_host.log('Upload did not complete successfully. Please check the logs for details.')

    result = api.update_entity(
    id=entity_id,
    project=project_id, 
    fields={
        'state' : 'COMPLETE'
    })
    
    create_or_update_ini_file(g_Lambda_Info_ini_path, section_entity, entity_name, entity_id)
    
    if not result.success:
        lambda_host.log(result.error_info)
        exit_code(111)

    lambda_host.log(f'End do_upload_base_meshes_swarm Created entity {result.id} for {entity_name}')
#-----------------------------------------------------------------------------------------------------------

    
#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def do_process_base_meshes(api : voxelfarmclient.rest, project_id, basemeshes_db_folderId, file_path : str, version : int, entity_name : str, code_path : str):
    lambda_host.log(f'Start do_process_base_meshes Created entity {entity_name}')

    file_extension = 'py'
    code_files = find_files_of_type_in_folder(code_path, file_extension)
    code_files_string = ', '.join(code_files)
    lambda_host.log(f'Python code filse are : {code_files_string}')
    
    result = api.get_project_crs(project_id)
    crs = result.crs
    entity_id = None
    index_vf_path = os.path.join(file_path, 'index.vf')
    with open(index_vf_path, 'rb') as f:
        lambda_host.log(f'start create_entity_raw file for entity {entity_name}')
        result = api.create_entity_raw(project=project_id, 
            type=api.entity_type.VoxelMesh, 
            name=entity_name, 
            fields={
                'state': 'PARTIAL',
                'file_folder': basemeshes_db_folderId,
            }, crs = crs)
        entity_id = result.id
        if not result.success:
            lambda_host.log(f'Fail to create_entity_raw Created entity for {entity_name} : {result.error_info}')
        else:
            lambda_host.log(f'Successfully to create_entity_raw Created entity for {result.id} for {entity_name}')
        index_vf_size = os.path.getsize(index_vf_path)
        lambda_host.log(f'Attaching file {index_vf_path} with size of {index_vf_size} to entity {entity_id}')
        try:
            result = api.attach_files(project=project_id, id=entity_id, files={'file': f})
        except Exception as e:
            lambda_host.log(f'Exception of Attach_files index vf to 1 attach file {data_vf_path} to entity {entity_id} with exception of {str(e)}')
        if not result.success:
            lambda_host.log(f'Failed to 3 attach index vf file {index_vf_path} to entity {entity_id} with error of {result.error_info}')
            return

    data_vf_path = os.path.join(file_path, 'data.vf')
    with open(data_vf_path, 'rb') as f:
        data_vf_size = os.path.getsize(data_vf_path)
        lambda_host.log(f'Attaching file {data_vf_path} with size of {data_vf_size} to entity {entity_id}')
        try:
            result = api.attach_files(project=project_id, id=entity_id, files={'file': f})
        except Exception as e:
            lambda_host.log(f'Exception of Attach_files data vf to 1 attach file {data_vf_path} to entity {entity_id} with exception of {str(e)}')
        if not result.success:
            lambda_host.log(f'Failed to 4 attach data vf file {data_vf_path} to entity {entity_id} with error of {result.error_info}')
            return
    lambda_host.log(f'Start to upload_db entity_id : {entity_id} ---- with folder ; {file_path}')
    
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
        lambda_host.log(f'Exception of create_process_entity to entity {entity_name} with exception of {str(e)}')
    
    if not result.success:
        lambda_host.log(f'Fail to do_process_base_meshes Created entity for {entity_name} : {result.error_info}')
    else:
        lambda_host.log(f'Successfully to do_process_base_meshes Created entity for {result.id} for {entity_name}')

    lambda_host.log(f'End do_process_base_meshes Created entity {result.id} for {entity_name}')

#---------------------------------------------------------------------------------------------------------------------------------------------------------------
def xc_process_base_meshes(api : voxelfarmclient.rest, basemeshes_output_folder_path):
    basemeshes_result_project_id = Project_id
    basemeshes_result_folder_id = '3A18892690F940759590B782AA80FC13'
    level0_db_output_folder = os.path.join(basemeshes_output_folder_path, f'{tile_size}_{tile_x}_{tile_y}_0')
    level1_db_output_folder = os.path.join(basemeshes_output_folder_path, f'{tile_size}_{tile_x}_{tile_y}_1')
    project_entity = api.get_entity(basemeshes_result_project_id)
    version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
    #api.update_entity(project=basemeshes_result_project_id, id=basemeshes_result_project_id, fields={'version': version})
    #level0_entity_name = f'Workflow_Basemeshes_{tile_size}_{tile_x}_{tile_y}_0-ver-{version}'
    #level1_entity_name = f'Workflow_Basemeshes_{tile_size}_{tile_x}_{tile_y}_1-ver-{version}'

    #basemeshes_project_id = '74F0C96BF0F24DA2BB5AE4ED65D81D8C'
    basemeshes_project_id = '1D4CBBD1D957477E8CC3FF376FB87470' #Project: "My Projects > Pangea Next"
    basemeshes_db_parent_folderId = 'CBF17A5E89EF4BA2A9A619CC57FBDA93' #Folder: "My Projects > Pangea Next > Basemeshes_Workflow"
    basemeshes_project_entity = api.get_entity(basemeshes_project_id)
    test_version = basemeshes_project_entity['basemeshes_version']
    lambda_host.log(f'-----------------test version: {test_version}!-----------------')
    basemeshes_version = (int(test_version) + 1) if 'basemeshes_version' in basemeshes_project_entity else 1
    api.update_entity(project=basemeshes_project_id, id=basemeshes_project_id, fields={'basemeshes_version': basemeshes_version})  
    lambda_host.log(f'-----------------Successful to get basemeshes_version {basemeshes_version}!-----------------')
    

    result = api.create_folder(project=basemeshes_project_id, name=f'Version {basemeshes_version}', folder=basemeshes_db_parent_folderId)
    if not result.success:
        print(f'Failed to create base meshes db folder for version!')
        exit(4)
    basemeshes_db_folder_Id = result.id
    print(f'Successful to create base meshes db folder {basemeshes_db_folder_Id} for version!')
    
    level0_entity_name = f'Workflow_Basemeshes_{tile_size}_{tile_x}_{tile_y}_0-ver-{basemeshes_version}'
    level1_entity_name = f'Workflow_Basemeshes_{tile_size}_{tile_x}_{tile_y}_1-ver-{basemeshes_version}'

    lambda_host.log(f'basemeshes_result_project_id :  {basemeshes_result_project_id}')
    lambda_host.log(f'basemeshes_result_folder_id :  {basemeshes_result_folder_id}')
    lambda_host.log(f'level0_db_output_folder :  {level0_db_output_folder}')
    lambda_host.log(f'level1_db_output_folder :  {level1_db_output_folder}')
    lambda_host.log(f'version :  {version}')
    lambda_host.log(f'level0_entity_name :  {level0_entity_name}')
    lambda_host.log(f'level1_entity_name :  {level1_entity_name}')
    
    #do_process_base_meshes(api, basemeshes_project_id, basemeshes_db_folder_Id, level0_db_output_folder, basemeshes_version, level0_entity_name, pythoncode_data_folder)
    #do_process_base_meshes(api, basemeshes_project_id, basemeshes_db_folder_Id, level1_db_output_folder, basemeshes_version, level1_entity_name, pythoncode_data_folder)
    #do_upload_base_meshes(api, basemeshes_project_id, basemeshes_db_folder_Id, level0_db_output_folder, basemeshes_version, level0_entity_name, pythoncode_data_folder)
    #do_upload_base_meshes(api, basemeshes_project_id, basemeshes_db_folder_Id, level1_db_output_folder, basemeshes_version, level1_entity_name, pythoncode_data_folder)
    do_upload_base_meshes_swarm(api, basemeshes_project_id, basemeshes_db_folder_Id, level0_db_output_folder, basemeshes_version, level1_entity_name, pythoncode_data_folder)
    do_upload_base_meshes_swarm(api, basemeshes_project_id, basemeshes_db_folder_Id, level1_db_output_folder, basemeshes_version, level0_entity_name, pythoncode_data_folder)
    
#--------------------------------------------------------------------------------------------------------------------------------------------------------------
def xc_attach_file_to_lambda(api : voxelfarmclient.rest):
    workflow_project_id = '0B0B6CCD4F56423C8196B7E9EA690E97'
    lambda_ini_exist = os.path.exists(g_Lambda_Info_ini_path)
    if lambda_ini_exist:
        lambda_ini_string = ini_file_to_string(g_Lambda_Info_ini_path)
        lambda_host.log(f'Lambda ini file : {g_Lambda_Info_ini_path} is exist!')
        lambda_host.log(f'{lambda_ini_string}')
        
        with open(g_Lambda_Info_ini_path, 'rb') as f:
            result = api.attach_files(project=workflow_project_id, id = lambda_entity_id, files={'file': f})
            if not result.success:
                lambda_host.log(f'Failed to attach ini file {g_Lambda_Info_ini_path} to lambda entity {lambda_entity_id}')
            else:
                lambda_host.log(f'Succeed to attach ini file {g_Lambda_Info_ini_path} to lambda entity {lambda_entity_id}')
    else:
        lambda_host.log(f'Lambda ini file : {g_Lambda_Info_ini_path} is not exist!')

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

    lambda_host.log(f'basemeshes_result_project_id :  {basemeshes_result_project_id}')
    lambda_host.log(f'basemeshes_result_folder_id :  {basemeshes_result_folder_id}')
    lambda_host.log(f'level0_output_folder :  {level0_output_folder}')
    lambda_host.log(f'level1_output_folder :  {level1_output_folder}')
    lambda_host.log(f'version :  {version}')
    lambda_host.log(f'level0_entity_name :  {level0_entity_name}')
    lambda_host.log(f'level0_entity_name :  {level1_entity_name}')

    basemeshes_result_project_entity = api.get_entity(basemeshes_result_project_id)
    version = int(basemeshes_result_project_entity['version']) + 1 if 'version' in basemeshes_result_project_entity else 1
    api.update_entity(project=basemeshes_result_project_id, id=basemeshes_result_project_id, fields={'version': version})
    result = api.create_folder(project=basemeshes_result_project_id, name=f'Version {version}', folder=basemeshes_result_folder_id)
    if not result.success:
        lambda_host.log(f'Failed to create basemeshes workflow result folder for version!')
        exit(4)
    basemeshes_result_version_folder_id = result.id
    lambda_host.log(f'-----------------Successful to create basemeshes workflow folder {basemeshes_result_version_folder_id} for version {version}!-----------------')

    lambda_host.log('Start with create basemeshes workflow level 0 entity {level0_entity_name}')
    result = xc_process_files_entity(api, basemeshes_result_project_id, basemeshes_result_version_folder_id, api.entity_type.RawMesh, api.entity_type.RawMesh, level0_output_folder, level0_entity_name, version=version, color=True, zipped=True)
    if not result.success:
        lambda_host.log(f'Failed to create basemeshes workflow result {level0_entity_name} with {api} basemeshes_result_project_id: {basemeshes_result_project_id} level0_output_folder: {level0_output_folder} raw: api.entity_type.RawMesh index: api.entity_type.IndexedMesh version: {version} !')
        exit(4)
    lambda_host.log('End with create basemeshes workflow level 0 entity {level0_entity_name}')

    lambda_host.log('Start with create basemeshes workflow level 1 entity {level1_entity_name}')
    result = xc_process_files_entity(api, basemeshes_result_project_id, basemeshes_result_version_folder_id, api.entity_type.RawMesh, api.entity_type.RawMesh, level1_output_folder, level1_entity_name, version=version, color=True, zipped=True)
    if not result.success:
        lambda_host.log(f'Failed to create basemeshes workflow result {level1_entity_name} with {api} basemeshes_result_project_id: {basemeshes_result_project_id} level1_output_folder: {level1_output_folder} raw: api.entity_type.RawMesh index: api.entity_type.IndexedMesh version: {version} !')
        exit(4)
    lambda_host.log('End with create basemeshes workflow level 1 entity {level1_entity_name}')
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
    lambda_host.log(f'Attaching file {level0_output_folder} to level 0 entity {level0_entity_id}')
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
    lambda_host.log(f'Attaching file {level1_output_folder} to entity {level1_entity_id}')
    for file_path in level1_output_folder:
        with open(file_path, "rb") as file:
            api.attach_files(project=basemeshes_result_project_id, id=level1_entity_id, files={'file': file})
'''
#---------------------------------------------------------------------------------------------------------------------------------------------------------------

def exit_code(code):
    lambda_host.set_exit_code(code)

def tree_instances_generation(config_path):
    lambda_host.log(f'start for step tree_instances_generation')

    if not os.path.exists(config_path):
        print(f'Config File {config_path} does not exist')
        lambda_host.log(f'Config File {config_path} does not exist')
        return -1
    
    lambda_host.log(f'Start to read value from {config_path}')

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
    tree_list = read_ini_value(config_path, section_input, 'treelist_data_path')

    road_output_folder = read_ini_value(config_path, section_output, 'road_output_folder')
    smoothlayer_output_base_folder = read_ini_value(config_path, section_output, 'smoothlayer_output_base_folder')
    basemeshes_db_base_folder = read_ini_value(config_path, section_output, 'basemeshes_db_base_folder')
    basemeshes_cache_base_folder = read_ini_value(config_path, section_output, 'basemeshes_cache_base_folder')
    basemeshes_heightmap_folder = read_ini_value(config_path, section_output, 'basemeshes_heightmap_folder')
    
    basemeshes_whole_cache_folder = os.path.join(Data_folder, f'whole')
    basemeshes_qtree_pointcloud_folder = os.path.join(Data_folder, f'qtreepc')
    
    tree_output_base_folder = read_ini_value(config_path, section_output, 'tree_output_base_folder')

    run_update_basemeshes_assets = read_ini_value(config_path, section_run, 'run_update_basemeshes_assets', value_type=bool)
    run_road_exe = read_ini_value(config_path, section_run, 'run_road_exe', value_type=bool)
    run_worldgen_road = read_ini_value(config_path, section_run, 'run_worldgen_road', value_type=bool)
    run_make_basemeshes = read_ini_value(config_path, section_run, 'run_make_basemeshes', value_type=bool)
    run_upload_basemeshes = read_ini_value(config_path, section_run, 'run_upload_basemeshes', value_type=bool)
    run_make_tree_instances = read_ini_value(config_path, section_run, 'run_make_tree_instances', value_type=bool)
    run_upload_tree_instances = read_ini_value(config_path, section_run, 'run_upload_tree_instances', value_type=bool)
    run_create_geochem_entity = read_ini_value(config_path, section_run, 'run_create_geochem_entity', value_type=bool)

    road_Heightmap_width = read_ini_value(config_path, section_road, 'road_Heightmap_width', value_type=int)
    road_heightmap_height = read_ini_value(config_path, section_road, 'road_heightmap_height', value_type=int)

    basemeshes_debug_level = read_ini_value(config_path, section_others, 'basemeshes_debug_level', value_type=int)
    tree_lod = read_ini_value(config_path, section_others, 'tree_lod', value_type=int)
    forest_age = read_ini_value(config_path, section_others, 'forest_age', value_type=int)
    tree_iteration = read_ini_value(config_path, section_others, 'tree_iteration', value_type=int)

    lambda_host.log(f'End to read value from {config_path}')

    lambda_host.log(f'Start to prepare input data parameter for TreesInstancesAbsolutePathWin.ini')
    basemeshes_level0 = 0
    basemeshes_level1 = 1
    version = 80

    most_travelled_points_path = os.path.join(road_output_folder, f'{tiles_count}_{tiles_x}_{tiles_y}_Most_Travelled_Points.csv') 
    most_distant_points_path = os.path.join(road_output_folder, f'{tiles_count}_{tiles_x}_{tiles_y}_Most_Distant_Points.csv') 
    
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
    basemeshes_0_heightmap_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level0}_heightarray.bin'
    basemeshes_1_heightmap_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level1}_heightarray.bin'
    basemeshes_0_heightmap_mask_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level0}_heightmasks.bin'
    basemeshes_1_heightmap_mask_name = f'{tiles_count}_{tiles_x}_{tiles_y}_{basemeshes_level1}_heightmasks.bin'
    basemeshes_0_heightmap_path = os.path.join(basemeshes_heightmap_folder, basemeshes_0_heightmap_name)
    basemeshes_1_heightmap_path = os.path.join(basemeshes_heightmap_folder, basemeshes_1_heightmap_name)
    basemeshes_0_heightmap_mask_path = os.path.join(basemeshes_heightmap_folder, basemeshes_0_heightmap_mask_name)
    basemeshes_1_heightmap_mask_path = os.path.join(basemeshes_heightmap_folder, basemeshes_1_heightmap_mask_name)

    smoothlayer_output_folder = os.path.join(smoothlayer_output_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}')
    toplayer_image_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.jpg'
    toplayer_image_meta_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.jgw'
    toplayer_heightmap_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.height.raw'
    toplayer_heightmap_mask_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_toplevel.xyz.height.masks.raw'
    level1_heightmap_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_level1.xyz.height.raw'
    level1_heightmap_mask_name = f'points_{tiles_count}_{tiles_x}_{tiles_y}_level1.xyz.height.masks.raw'
    toplayer_image_path = os.path.join(smoothlayer_output_folder, toplayer_image_name)
    toplayer_image_meta_path = os.path.join(smoothlayer_output_folder, toplayer_image_meta_name)
    toplayer_heightmap_path = os.path.join(smoothlayer_output_folder, toplayer_heightmap_name)
    toplayer_heightmap_mask_path = os.path.join(smoothlayer_output_folder, toplayer_heightmap_mask_name)
    level1_heightmap_path = os.path.join(smoothlayer_output_folder, level1_heightmap_name)
    level1_heightmap_mask_path = os.path.join(smoothlayer_output_folder, level1_heightmap_mask_name)

    lambda_host.log(f'End to to prepare input data parameter for TreesInstancesAbsolutePathWin.ini')

    lambda_host.log(f'Start to prepare command line for programs')

    api = voxelfarmclient.rest(cloud_url)
    
    basemeshes_asset_download_parent_folder = os.path.join(qtree_assets_folder, f'BaseMeshes_Versions')
    basemeshes_asset_download_folder = os.path.join(basemeshes_asset_download_parent_folder, basemeshes_entity_id)
    dont_run_road_game = 1
    road_exe_command = f'{road_exe_path} {tiles_count} {tiles_x} {tiles_y} {road_Heightmap_width} {road_heightmap_height} {road_input_folder} {road_output_folder} {dont_run_road_game}'
    worldgen_level = 5
    worldgen_command =  f'{worldgen_exe_path} {tiles_count} {tiles_x} {tiles_y} {worldgen_level} {qtree_assets_folder} {smoothlayer_output_base_folder} {road_output_folder}'
    basemeshvoxelizer1_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level1} {basemeshes_assets_folder} {basemeshes_db_base_folder} {basemeshes_cache_base_folder} {basemeshes_debug_level} {basemeshes_heightmap_folder}'
    basemeshvoxelizer0_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level0} {basemeshes_assets_folder} {basemeshes_db_base_folder} {basemeshes_cache_base_folder} {basemeshes_debug_level} {basemeshes_heightmap_folder}'
    basemeshvoxelizer_ini_command = f'{basemeshes_exe_path} {basemeshes_ini_path}'
    tree_exe_command = f'{tree_exe_path} {tree_ini_path}'
    
    if run_upload_basemeshes:
        basemeshes_all_level = 0 # use all level for base meshes 
        basemeshvoxelizer1_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level1} {basemeshes_assets_folder} {basemeshes_db_base_folder} {basemeshes_cache_base_folder} {basemeshes_all_level} {basemeshes_heightmap_folder}'
        basemeshvoxelizer0_command = f'{basemeshes_exe_path} {tiles_count} {tiles_x} {tiles_y} {basemeshes_level0} {basemeshes_assets_folder} {basemeshes_db_base_folder} {basemeshes_cache_base_folder} {basemeshes_all_level} {basemeshes_heightmap_folder}'
        lambda_host.log("Adjust base meshes command line to all level")
    
    if run_upload_basemeshes and use_basemesh_ini:
        lambda_host.log(f'Start to write standard basemeshes ini files : {basemeshes_ini_path}')
        create_or_overwrite_empty_file(basemeshes_ini_path)
        
        create_or_update_ini_file(basemeshes_ini_path, section_tiles, 'Tiles_Count', tiles_count)
        create_or_update_ini_file(basemeshes_ini_path, section_tiles, 'Tiles_X_Index', tiles_x)
        create_or_update_ini_file(basemeshes_ini_path, section_tiles, 'Tiles_Y_Index', tiles_y)
        
        create_or_update_ini_file(basemeshes_ini_path, section_input, 'Assets_Folder', basemeshes_assets_folder)
        create_or_update_ini_file(basemeshes_ini_path, section_input, 'BaseMeshesCSV_Name', basemeshes_csv_name)
        
        create_or_update_ini_file(basemeshes_ini_path, section_output, 'DB_Base_Folder', basemeshes_db_base_folder)
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
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_WHOLE_MESHES_WC', False)
        
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_AVERAGE_HEIGHT', False)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_HEIGHTMAP_MASK', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_SHORT_HEIGHTMAP', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_FLOAT_HEIGHTMAP', False)
        
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_QTREE_CSV', False)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_COASTLINE_JSON', False)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_COASTLINE_CSV', False)
        
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_BASEMESHES_OUTPUT_PROGRESS', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_NODE_MAX_DIMS', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_OUTPUT_QUADBOUNDS_ERROR', False)
        
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_APPLY_RANDOM_SCALE', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_APPLY_RANDOM_ROTATION', True)
        create_or_update_ini_file(basemeshes_ini_path, section_options, 'USE_APPLY_RANDOM_NOISE_BASEMESH_SELECTION', False)
        
        lambda_host.log(f'End to write standard basemeshes ini files : {basemeshes_ini_path}')
        basemeshes_ini_string = ini_file_to_string(basemeshes_ini_path)
        lambda_host.log(f'Basemeshes standard ini file content is :')
        lambda_host.log(f'{basemeshes_ini_string}')
        
    if run_road_exe:    
        lambda_host.log(f'road_exe_command : {road_exe_command}')
    if run_worldgen_road:
        lambda_host.log(f'worldgen_command : {worldgen_command}')
    if run_make_basemeshes:
        if use_basemesh_ini:
            lambda_host.log(f'basemeshvoxelizer_ini_command : {basemeshvoxelizer_ini_command}')
        else:
            lambda_host.log(f'basemeshvoxelizer0_command : {basemeshvoxelizer0_command}')
            lambda_host.log(f'basemeshvoxelizer1_command : {basemeshvoxelizer1_command}')
    
    if run_make_tree_instances:
        lambda_host.log(f'tree_exe_command : {tree_exe_command}')
        
    lambda_host.log(f'End to prepare command line for programs')
        
    ##### Make ini config file for tree exe.
    #clear_all_sections(tree_ini_path)
    if run_make_tree_instances:
        lambda_host.log(f'Start to write tree instance ini files : {tree_ini_path}')
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
        create_or_update_ini_file(tree_ini_path, section_input, 'Tree_List', tree_list)
        
        create_or_update_ini_file(tree_ini_path, section_output, 'Output_Dir', tree_output_base_folder)
        create_or_update_ini_file(tree_ini_path, section_others, 'Lod', tree_lod)
        create_or_update_ini_file(tree_ini_path, section_others, 'Forest_Age', forest_age)
        create_or_update_ini_file(tree_ini_path, section_others, 'Tree_Iteration', tree_iteration)
        lambda_host.log(f'End to write tree instance ini files : {tree_ini_path}')
        tree_ini_string = ini_file_to_string(tree_ini_path)
        lambda_host.log(f'Tree ini file content is :')
        lambda_host.log(f'{tree_ini_string}')
    
    if run_update_basemeshes_assets:
        lambda_host.log(f'step for to run_update_basemeshes_assets')
        ##### Download BaseMeshes(version) assets from Cloud!
        file_list = api.get_file_list(project_id, basemeshes_entity_id)
        for index, file_name in enumerate(file_list):
            lambda_host.log(f"Index: {index}, File Path: {file_name}")
            file_data = api.get_file(project_id, basemeshes_entity_id, file_name)
            file_path = os.path.join(basemeshes_asset_download_folder, file_name)
            save_data_to_file(file_data, file_path)
        ##### Copy BaseMeshes(version) assets to BaseMeshes asset folder!
        copy_files(basemeshes_asset_download_folder, qtree_assets_folder)
  
    if run_road_exe:
        ##### Generate the road obj and image for smooth layer. 
        #return_code_road = launch_process(road_exe_command)
        lambda_host.log(f'step for to run_road_exe : {road_exe_command}')
        return_code_road = xc_run_tool(road_exe_command, 21, 40)
        if return_code_road == 0:
            lambda_host.log(f'Process ({road_exe_command}) executed successfully.')
        else:
            lambda_host.log(f'Error: The process ({road_exe_command}) returned a non-zero exit code ({run_road_exe}).')
            exit_code(2)
            return -1
    
    if run_worldgen_road:
        lambda_host.log(f'step for to run_worldgen_road : {worldgen_command}')
        ##### Generate the height map and image for smooth layer. 
        #return_code_worldgen_road = launch_process(worldgen_command)
        return_code_worldgen_road = xc_run_tool(worldgen_command, 41, 60)
        if return_code_worldgen_road == 0:
            lambda_host.log(f'Process ({worldgen_command}) executed successfully.')
        else:
            lambda_host.log(f'Error: The process ({worldgen_command}) returned a non-zero exit code ({return_code_worldgen_road}).')
            exit_code(2)
            return -1
    
    if run_make_basemeshes:
        if use_basemesh_ini:
            lambda_host.log(f'step for run basemeshes with ini : {basemeshvoxelizer_ini_command}')
            
            lambda_host.log(f'step for run basemeshes with ini level : {basemeshes_level1}')
            if run_upload_basemeshes:
                create_or_update_ini_file(basemeshes_ini_path, section_others, 'Level', basemeshes_level1)
                create_or_update_ini_file(basemeshes_ini_path, section_others, 'LodDebugLevel', basemeshes_all_level)
                lambda_host.log(f'Adjust base meshes ini level {basemeshes_level1} to all LOD level')
            basemeshes_ini_string = ini_file_to_string(basemeshes_ini_path)
            lambda_host.log(f'Basemeshes ini file for level {basemeshes_level0} content is :')
            lambda_host.log(f'{basemeshes_ini_string}')
            return_code_basemesh_ini_1 = xc_run_tool(basemeshvoxelizer_ini_command, 61, 80)
            if return_code_basemesh_ini_1 == 0:
                lambda_host.log(f'Process level {basemeshes_level1} with ({basemeshvoxelizer_ini_command}) executed successfully.')
            else:
                lambda_host.log(f'Error: The process level {basemeshes_level1} with ({basemeshvoxelizer_ini_command}) returned a non-zero exit code ({return_code_basemesh_ini_1}).')
                exit_code(2)
                return -1
            
            lambda_host.log(f'step for run basemeshes with ini level : {basemeshes_level0}')
            if run_upload_basemeshes:
                create_or_update_ini_file(basemeshes_ini_path, section_others, 'Level', basemeshes_level0)
                create_or_update_ini_file(basemeshes_ini_path, section_others, 'LodDebugLevel', basemeshes_all_level)
                lambda_host.log(f'Adjust base meshes ini level {basemeshes_level0} to all LOD level')
            basemeshes_ini_string = ini_file_to_string(basemeshes_ini_path)
            lambda_host.log(f'Basemeshes ini file for level {basemeshes_level0} content is :')
            lambda_host.log(f'{basemeshes_ini_string}')
            return_code_basemesh_ini_0 = xc_run_tool(basemeshvoxelizer_ini_command, 81, 90)
            if return_code_basemesh_ini_0 == 0:
                lambda_host.log(f'Process level {basemeshes_level0} with ({basemeshvoxelizer_ini_command}) executed successfully.')
            else:
                lambda_host.log(f'Error: The process level {basemeshes_level0} with ({basemeshvoxelizer_ini_command}) returned a non-zero exit code ({return_code_basemesh_ini_0}).')
                exit_code(2)
                return -1
            
            
        else:
            ##### Generate the height map from level 0 of BaseMeshes.  
            
            lambda_host.log(f'step for to run_make_basemeshes : {basemeshvoxelizer1_command}')
            #return_code_basemash1 = launch_process(basemeshvoxelizer1_command)
            return_code_basemash1 = xc_run_tool(basemeshvoxelizer1_command, 61, 80)
            if return_code_basemash1 == 0:
                lambda_host.log(f'Process ({basemeshvoxelizer1_command}) executed successfully.')
            else:
                lambda_host.log(f'Error: The process ({basemeshvoxelizer1_command}) returned a non-zero exit code ({return_code_basemash1}).')
                exit_code(2)
                return -1
            
            #return_code_basemash0 = launch_process(basemeshvoxelizer0_command)
            lambda_host.log(f'step for to run_make_basemeshes : {basemeshvoxelizer0_command}')
            return_code_basemash0 = xc_run_tool(basemeshvoxelizer0_command, 81, 90)
            if return_code_basemash0 == 0:
                lambda_host.log(f'Process ({basemeshvoxelizer0_command}) executed successfully.')
            else:
                lambda_host.log(f'Error: The process ({basemeshvoxelizer0_command}) returned a non-zero exit code ({return_code_basemash0}).')
                exit_code(2)
                return -1
            ##### Generate the height map from level 1 of BaseMeshes. 
        
    if run_make_tree_instances:
        lambda_host.log(f'step for to run_make_tree_instances : {tree_exe_command}')
        ##### Run tree exe to generate to tree instances.
        #return_code_tree = launch_process(tree_exe_command)
        return_code_tree = xc_run_tool(tree_exe_command, 91, 100)
        if return_code_tree == 0:
            lambda_host.log(f'Process ({tree_exe_command}) executed successfully.')
        else:
            lambda_host.log(f'Error: The process ({tree_exe_command}) returned a non-zero exit code ({return_code_tree}).')
            exit_code(2)
            return -1

    if run_upload_tree_instances:
        lambda_host.log(f'step for to run_upload_tree_instances')
        ##### Update the tree instance files of tree entity.
        #workflow_api = workflow_lambda.workflow_lambda_host()
        tree_instance_output_folder = os.path.join(tree_output_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}', 'instanceoutput')
        update_attach_files_for_entity(api, project_id, tree_entity_id, tree_instance_output_folder, f'instances_lod8_{tiles_count}_{tiles_x}_{tiles_y}-{version}', version=version, color=True)
        lambda_host.log(f'update_attach_files_for_entity for {tree_entity_id}')

    if run_create_geochem_entity:
        lambda_host.log(f'step for to run_create_geochem_entity!')
        ##### create the geochem entity for tree instance files.
        geo_chemical_folder = os.path.join(tree_output_base_folder, f'{tiles_count}_{tiles_x}_{tiles_y}', 'GeoChemical')
        create_geochem_tree_entity(api, geo_chemical_folder)
        lambda_host.log(f'create_geochem_tree_entity for {geo_chemical_folder}')

    if run_upload_basemeshes:
        lambda_host.log(f'step for to run_upload_basemeshes')
        ##### upload basemeshes voxel database to cloud.
        basemeshes_output_folder = basemeshes_db_base_folder
        #create_basemeshes_result_entity(api, basemeshes_output_folder)
        xc_process_base_meshes(api, basemeshes_output_folder)
        lambda_host.log(f'create_basemeshes_result_entity for {basemeshes_output_folder}')
        
    if run_make_basemeshes and run_upload_basemeshes:
        lambda_host.log(f'step for to xc_attach_file_to_lambda')
        xc_attach_file_to_lambda(api)

    lambda_host.log(f'end for step tree_instances_generation')
    return 0

def tree_config_creation(ini_path):
    #road_input_folder = f'{Data_folder}\\RoadRawInit'
    lambda_host.log(f'start to create tree_config_creation : {ini_path}')

    road_input_folder = f'{Data_folder}'
    road_exe_name = f'NPCTest2.exe'
    road_exe_path = os.path.join(Tools_folder, road_exe_name)
    basemeshes_exe_name = f'BaseMeshVoxelizer.exe'
    if not use_basemesh_ini:
        basemeshes_exe_name = f'BaseMeshVoxelizerOld.exe'
    basemeshes_exe_path = os.path.join(Tools_folder, basemeshes_exe_name)
    worldgen_exe_name = f'WorldGen.exe'
    worldgen_exe_path = os.path.join(Tools_folder, worldgen_exe_name)
    tree_exe_name = f'PlantsSimulation.exe'
    tree_exe_path = os.path.join(Tools_folder, tree_exe_name)
    qtree_assets_folder = Data_folder

    road_output_folder = os.path.join(Data_folder, f'RoadObjInfo')
    smoothlayer_output_base_folder = os.path.join(Data_folder, f'sommothlayer_output')
    basemeshes_db_base_folder = os.path.join(Data_folder, f'db')
    basemeshes_cache_base_folder = os.path.join(Data_folder, f'cache')
    basemeshes_heightmap_folder = os.path.join(Data_folder, f'heightmap')
    tree_output_base_folder = os.path.join(Data_folder, f'tree_output')

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
    create_or_update_ini_file(ini_path, section_input, 'treelist_data_path', treelist_data_path)

    create_or_update_ini_file(ini_path, section_output, 'road_output_folder', road_output_folder)
    create_or_update_ini_file(ini_path, section_output, 'smoothlayer_output_base_folder', smoothlayer_output_base_folder)
    create_or_update_ini_file(ini_path, section_output, 'basemeshes_db_base_folder', basemeshes_db_base_folder)
    create_or_update_ini_file(ini_path, section_output, 'basemeshes_cache_base_folder', basemeshes_cache_base_folder)
    create_or_update_ini_file(ini_path, section_output, 'basemeshes_heightmap_folder', basemeshes_heightmap_folder)
    create_or_update_ini_file(ini_path, section_output, 'tree_output_base_folder', tree_output_base_folder)

    create_or_update_ini_file(ini_path, section_run, 'run_update_basemeshes_assets', is_run_update_basemeshes_assets)
    create_or_update_ini_file(ini_path, section_run, 'run_road_exe', is_run_road_exe)
    create_or_update_ini_file(ini_path, section_run, 'run_worldgen_road', is_run_worldgen_road)
    create_or_update_ini_file(ini_path, section_run, 'run_make_basemeshes', is_run_make_basemeshes)
    create_or_update_ini_file(ini_path, section_run, 'run_upload_basemeshes', is_run_upload_basemeshes)
    create_or_update_ini_file(ini_path, section_run, 'run_make_tree_instances', is_run_make_tree_instances)
    create_or_update_ini_file(ini_path, section_run, 'run_upload_tree_instances', is_run_upload_tree_instances)
    create_or_update_ini_file(ini_path, section_run, 'run_create_geochem_entity', is_run_create_geochem_entity)

    create_or_update_ini_file(ini_path, section_road, 'road_Heightmap_width', 300)
    create_or_update_ini_file(ini_path, section_road, 'road_heightmap_height', 300)

    create_or_update_ini_file(ini_path, section_others, 'basemeshes_debug_level', Basemeshes_debug_level)
    create_or_update_ini_file(ini_path, section_others, 'tree_lod', Tree_load)
    create_or_update_ini_file(ini_path, section_others, 'forest_age', Forest_age)
    create_or_update_ini_file(ini_path, section_others, 'tree_iteration', Tree_iteration)
    
    lambda_host.log(f'end to create tree_config_creation : {ini_path}')
    ini_string = ini_file_to_string(ini_path)
    lambda_host.log(f'Tree config creation file content is :')
    lambda_host.log(f'{ini_string}')
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
section_entity = 'Entity'
section_options = 'Options'
section_config = 'Configuration'

use_basemesh_ini = True

lambda_host = process_lambda.process_lambda_host()

lambda_host.progress(0, 'Starting Lambda...')
scrap_folder= lambda_host.get_scrap_folder()
lambda_host.log(f'scrap_folder: {scrap_folder}')
print(f'scrap_folder: {scrap_folder}')
tools = lambda_host.get_tools_folder()
lambda_host.log(f'system tools: {tools}')
print(f'system tools: {tools}\n')
lambda_entity_id = lambda_host.input_string('lambda_entity_id', 'Lambda Entity Id', '')
lambda_host.log(f'lambda_entity_id: {lambda_entity_id}')
project_id = lambda_host.input_string('project_id', 'Project Id', '')
lambda_host.log(f'project_id: {project_id}')
entity_id = lambda_host.input_string('entity_id', 'Entity Id', '')
lambda_host.log(f'entity_id: {entity_id}')
output_id = lambda_host.input_string('output_id', 'Output Id', '')
lambda_host.log(f'output_id: {output_id}')
tile_size = lambda_host.input_string('tile_size', 'Tile Size', '')
lambda_host.log(f'tile_size: {tile_size}')
tile_x = lambda_host.input_string('tile_x', 'Tile X', '')
lambda_host.log(f'tile_x: {tile_x}')
tile_y = lambda_host.input_string('tile_y', 'Tile Y', '')
lambda_host.log(f'tile_y: {tile_y}')
level = lambda_host.input_string('level', 'Level', '')
lambda_host.log(f'level: {level}')
entity_folder = lambda_host.get_entity_folder()
lambda_host.log(f'entity_folder: {entity_folder}')

Tree_load = lambda_host.input_string('tree_lod', 'tree_lod', '')
lambda_host.log(f'Tree_load: {Tree_load}')
Forest_age = lambda_host.input_string('forest_age', 'forest_age', '')
lambda_host.log(f'foreForest_agest_age: {Forest_age}')
Tree_iteration = lambda_host.input_string('tree_iteration', 'tree_iteration', '')
lambda_host.log(f'Tree_iteration: {Tree_iteration}')

pythoncode_active_version_property = lambda_host.input_string('pythoncode_active_version_property', 'pythoncode_active_version_property', '') 
treelist_active_version_property = lambda_host.input_string('treelist_active_version_property', 'treelist_active_version_property', '') 
roaddata_active_version_property = lambda_host.input_string('roaddata_active_version_property', 'roaddata_active_version_property', '')
basemeshes_active_version_property = lambda_host.input_string('basemeshes_active_version_property', 'basemeshes_active_version_property', '')
displacement_active_version_property = lambda_host.input_string('displacement_active_version_property', 'displacement_active_version_property', '')
qtree_active_version_property = lambda_host.input_string('qtree_active_version_property', 'qtree_active_version_property', '')
tools_active_version_property = lambda_host.input_string('tools_active_version_property', 'tools_active_version_property', '')

lambda_host.log('pythoncode_active_version_property: ' + pythoncode_active_version_property)
lambda_host.log('treelist_active_version_property: ' + treelist_active_version_property)
lambda_host.log('roaddata_active_version_property: ' + roaddata_active_version_property)
lambda_host.log('basemeshes_active_version_property: ' + basemeshes_active_version_property)
lambda_host.log('displacement_active_version_property: ' + displacement_active_version_property)
lambda_host.log('qtree_active_version_property: ' + qtree_active_version_property)
lambda_host.log('tools_active_version_property: ' + tools_active_version_property)

is_run_update_basemeshes_assets = lambda_host.input_string('run_update_basemeshes_assets', 'run_update_basemeshes_assets', '') 
is_run_road_exe = lambda_host.input_string('run_road_exe', 'run_road_exe', '')
is_run_worldgen_road = lambda_host.input_string('run_worldgen_road', 'run_worldgen_road', '')
is_run_make_basemeshes = lambda_host.input_string('run_make_basemeshes', 'run_make_basemeshes', '')
is_run_upload_basemeshes = lambda_host.input_string('run_upload_basemeshes', 'run_upload_basemeshes', '')
is_run_make_tree_instances = lambda_host.input_string('run_make_tree_instances', 'run_make_tree_instances', '')
is_run_upload_tree_instances = lambda_host.input_string('run_upload_tree_instances', 'run_upload_tree_instances', '')
is_run_create_geochem_entity = lambda_host.input_string('run_create_geochem_entity', 'run_create_geochem_entity', '')

lambda_host.log('is_run_update_basemeshes_assets: ' + is_run_update_basemeshes_assets)
lambda_host.log('is_run_road_exe: ' + is_run_road_exe)
lambda_host.log('is_run_worldgen_road: ' + is_run_worldgen_road)
lambda_host.log('is_run_make_basemeshes: ' + is_run_make_basemeshes)
lambda_host.log('is_run_upload_basemeshes: ' + is_run_upload_basemeshes)
lambda_host.log('is_run_make_tree_instances: ' + is_run_make_tree_instances)
lambda_host.log('is_run_upload_tree_instances: ' + is_run_upload_tree_instances)
lambda_host.log('is_run_create_geochem_entity: ' + is_run_create_geochem_entity)

lambda_host.progress(1, 'Start to download files')
pythoncode_data_folder = lambda_host.download_entity_files(pythoncode_active_version_property)
treelist_data_folder = lambda_host.download_entity_files(treelist_active_version_property)
#treelist_data_path = os.path.join(treelist_data_folder, 'TreeList.csv')
roaddata_data_path = lambda_host.download_entity_files(roaddata_active_version_property)
basemeshes_data_path = lambda_host.download_entity_files(basemeshes_active_version_property)
displacement_data_path = lambda_host.download_entity_files(displacement_active_version_property)
qtree_data_path = lambda_host.download_entity_files(qtree_active_version_property)
tools_data_path = lambda_host.download_entity_files(tools_active_version_property)

lambda_host.log('pythoncode_data_folder: ' + pythoncode_data_folder)
lambda_host.log('treelist_data_folder: ' + treelist_data_folder)
lambda_host.log('roaddata_data_path: ' + roaddata_data_path)
lambda_host.log('basemeshes_data_path: ' + basemeshes_data_path)
lambda_host.log('displacement_data_path: ' + displacement_data_path)
lambda_host.log('qtree_data_path: ' + qtree_data_path)
lambda_host.log('tools_data_path: ' + tools_data_path)

Tree_Data_Folder_Name = f'Tree_Instances_Creation'
Data_folder = os.path.join(scrap_folder, Tree_Data_Folder_Name)
g_Lambda_Info_ini_name = 'lambda_info.ini'
g_Lambda_Info_ini_path = os.path.join(Data_folder, g_Lambda_Info_ini_name)
lambda_host.log(f'Data_folder: {Data_folder}')
if not os.path.exists(Data_folder):
    os.makedirs(Data_folder)
#Tools_folder = os.path.join(Data_folder, 'Tools')
#Tools_folder = f'{Data_folder}\\Tools'
Tools_folder = tools_data_path
lambda_host.log(f'Tools_folder: {Tools_folder}')
if not os.path.exists(Tools_folder):
    os.makedirs(Tools_folder)

lambda_host.progress(5, 'Start to copy files')
lambda_host.log(f'start to copy from {pythoncode_data_folder} to {Data_folder}')
copy_files(pythoncode_data_folder, Data_folder)
lambda_host.log(f'end to copy from {pythoncode_data_folder} to {Data_folder}')
lambda_host.log(f'start to copy from {treelist_data_folder} to {Data_folder}')
copy_files(treelist_data_folder, Data_folder)
lambda_host.log(f'end to copy from {treelist_data_folder} to {Data_folder}')
treelist_data_path = os.path.join(Data_folder, 'TreeList.csv')
lambda_host.log(f'start to copy from {roaddata_data_path} to {Data_folder}')
copy_files(roaddata_data_path, Data_folder)
lambda_host.log(f'end to copy from {roaddata_data_path} to {Data_folder}')
lambda_host.log(f'start to copy from {basemeshes_data_path} to {Data_folder}')
copy_files(basemeshes_data_path, Data_folder)
lambda_host.log(f'end to copy from {basemeshes_data_path} to {Data_folder}')
lambda_host.log(f'start to copy from {displacement_data_path} to {Data_folder}')
copy_files(displacement_data_path, Data_folder)
lambda_host.log(f'end to copy from {displacement_data_path} to {Data_folder}')
lambda_host.progress(10, 'Start to copy big files')
lambda_host.log(f'start to copy from {qtree_data_path} to {Data_folder}')
copy_files(qtree_data_path, Data_folder)
lambda_host.log(f'end to copy from {qtree_data_path} to {Data_folder}')

#lambda_host.progress(15, 'Start to download tools files')
#lambda_host.log(f'start to copy from {tools_data_path} to {Tools_folder}')
#copy_files(tools_data_path, Tools_folder)
#lambda_host.log(f'end to copy from {tools_data_path} to {Tools_folder}')

lambda_host.progress(15, 'Start to get input parameters')
Cloud_url = 'http://localhost/'
Project_id = '1D4CBBD1D957477E8CC3FF376FB87470'
Folder_id = '90F6348AD5D94FCEA85C7C1CD081CE97' 
#Tree_entity_id = 'E0070AD37D4543FCB9E70D60AE47541D'
Tree_entity_id = '3A3CFEBA226B4692A8719C78335470DD'  
Basemeshes_entity_id = '4A59F80631E745E39557D23CED145732'
Tiles_size = tile_size if tile_size else 10
Tiles_x = tile_x if tile_x else 8
Tiles_y = tile_y if tile_y else 5
lambda_host.log(f'Tiles_size: {Tiles_size}')
lambda_host.log(f'Tiles_x: {Tiles_x}')
lambda_host.log(f'Tiles_y: {Tiles_y}')
Basemeshes_debug_level = level if level else 6
lambda_host.log(f'Basemeshes_debug_level: {Basemeshes_debug_level}')
configfile_path = f'{Data_folder}\\TreeInstancesCreationConfig.ini'
#configfile_path = params[0]
lambda_host.log(f'Tree instance generation configfile_path: {configfile_path}')
print(f'Tree instance generation config file : {configfile_path}')

run_result = 0

lambda_host.progress(20, 'Start to config files')
lambda_host.log(f'start tree_config_creation: {configfile_path}')
tree_config_creation(configfile_path)
lambda_host.log(f'end tree_config_creation: {configfile_path}')
lambda_host.log(f'start tree_instances_generation: {configfile_path}')
run_result = tree_instances_generation(configfile_path)
lambda_host.log(f'end tree_instances_generation: {configfile_path}')


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
lambda_host.log(f'Tree instance generation Whole Execution time : {formatted_time}')
print("Whole Execution time :", formatted_time)

if run_result == 0:
    lambda_host.progress(100, 'QuadTree lambda finished')
    exit_code(0)
else:
    lambda_host.progress(100, 'QuadTree lambda failed')

#exit()