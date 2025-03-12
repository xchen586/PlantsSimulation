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

    
    if not result.success:
        print(result.error_info)
        exit_code(111)

    print(f'End do_simple_upload_basemeshes_swarm Created entity {result.id} for {entity_name}')
    
start_time = time.time()
now = datetime.datetime.now()
# Format the current time into a string (e.g., 2024-10-17_14-35-50)
filename = now.strftime("%Y-%m-%d_%H-%M-%S")

scrap_folder= f'D:\\Downloads\\XCTreeCreation'
if not os.path.exists(scrap_folder):
    os.makedirs(scrap_folder)
# Add file extension if needed (e.g., .txt)
log_file_name = f"Upload_db_{filename}.log"
log_file_path = os.path.join(scrap_folder, log_file_name)
sys.stdout = DualOutput(log_file_path)  # Redirect stdout

pythoncode_data_folder = f'D:\\Downloads\\XCTreeWorkFlow\\PythonCode'
workflow_project_id = '1D4CBBD1D957477E8CC3FF376FB87470' #Pangea Next Project

tools = f'D:\\Downloads\\XCTreeWorkFlow\\Tools'
Tree_Data_Folder_Name = 'Tree_Big_Creation'
Data_folder = os.path.join(scrap_folder, Tree_Data_Folder_Name)
cloud_url = 'https://demo.voxelfarm.com/'
project_id = workflow_project_id
api = voxelfarmclient.rest(cloud_url)

tiles = 25
x = 8
y = 5
l = 1

project_entity = api.get_entity(project_id)
version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
api.update_entity(project=project_id, id=project_id, fields={'version': version})

basemeshes_db_folder_Id = '68396F90F7CE48B4BA1412EA020ED92A' # Pangea Next > Workflow Output > Workflow BaseMeshes Output

db_output_folder = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\cavesdb\\{tiles}_{x}_{y}_{l}'
entity_name = f'TopCaves_{tiles}_{x}_{y}_{l}-ver-{version}'

isBaseMesh = False
if isBaseMesh:
    db_output_folder = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\db\\{tiles}_{x}_{y}_{l}'
    entity_name = f'Workflow_Basemeshes_{tiles}_{x}_{y}_{l}-ver-{version}'

do_simple_upload_basemeshes_swarm(api, project_id, basemeshes_db_folder_Id, db_output_folder, version, entity_name, pythoncode_data_folder)