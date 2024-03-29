import os
import subprocess
from zipfile import ZipFile
import glob
from timeit import default_timer as timer
from datetime import timedelta
from voxelfarm import voxelfarmclient as vfc
from os import path
import configparser

from voxelfarm import workflow_lambda
from voxelfarm import process_lambda

lambda_host = process_lambda.process_lambda_host()
workflow_host = workflow_lambda.workflow_lambda_host()
vf = vfc.rest('http://127.0.0.1')

lambda_host.progress(0, 'Starting Base Meshes DB Upload lambda...')

scrap_folder= lambda_host.get_scrap_folder()
lambda_host.log(f'Base Meshes DB Upload scrap_folder: {scrap_folder}')
tools = lambda_host.get_tools_folder()
lambda_host.log(f'Base Meshes DB Upload tools: {tools}\n')

def on_exit_succeessfull(vf, project_id, entity_id, output_dir):
    lambda_host.progress(99, 'Uploading results')

    # save entity properties and complete state
    properties_file = f'{output_dir}/properties.vf'
    if path.exists(properties_file):
        entity_data= {}
        config = configparser.ConfigParser()
        config.read(properties_file)
        lambda_host.log('Entity Properties')

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
        lambda_host.log("extended.meta File not found")

def exit_code(code):
    lambda_host.set_exit_code(code)
    exit()

def run_tool(tool_path, progress_start, progress_end):    
    start = progress_start
    end = progress_end
    scale = (end - start) / 100
    tool_process = subprocess.Popen(tool_path, stdout=subprocess.PIPE, stderr=None, text=True)
    while True:
        realtime_output = tool_process.stdout.readline()

        if realtime_output:
            tokens = realtime_output.split()
            progress = -1
            if len(tokens) > 2:
                if tokens[0] == 'progress':
                    tool_progress = float(tokens[1])
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
    return tool_process.returncode 

lambda_host.log(f'Base Meshes UploadDB lambda started')
lambda_host.progress(10, 'UploadDB lambda started')
project_id = lambda_host.input_string('project_id', 'Project Id', '')
entity_db_id = lambda_host.input_string('entity_db_id', 'Entity Db Id', '')
#data_path = lambda_host.input_string('db_path', 'Db Path', '')
data_path = lambda_host.download_entity_files(entity_db_id)
lambda_host.log(f'Base Meshes UploadDB data_path: {data_path}')
lambda_host.log(f'Base Meshes UploadDB project_id: {project_id}')
lambda_host.log(f'Base Meshes UploadDB entity_id: {entity_db_id}')

lambda_host.progress(30, 'Base Meshes UploadDB lambda working')

if lambda_host.upload_db(entity_db_id, data_path, 'vox-pc', 'Voxel Data'):
    on_exit_succeessfull(vf, project_id, entity_db_id, data_path)
else:
    lambda_host.log('Base Meshes UploadDB Error on Tool.UploadDB')
    exit_code(3) 

lambda_host.progress(90, 'Base Meshes UploadDB lambda processing finished')

result = vf.update_entity(
    id=entity_db_id,
    project=project_id, 
    fields={
        'state' : 'COMPLETE'
    })

if not result.success:
    lambda_host.log(result.error_info)
    exit_code(111)

lambda_host.progress(100, 'Base Meshes UploadDB lambda finished')

lambda_host.set_exit_code(0)

#exit_code(0)
    