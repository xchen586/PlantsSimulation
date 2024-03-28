import os
import subprocess
from zipfile import ZipFile
import glob
from timeit import default_timer as timer
from datetime import timedelta
from voxelfarm import voxelfarmclient as vfc
from os import path
import configparser
import voxelfarmlambda 

OrgId = 2343243456678890

lambda_host = voxelfarmlambda.process_host()
workflow_host = voxelfarmlambda.workflow_host()

lambda_host.progress(0, 'Starting DB Upload lambda...')
scrap_folder= lambda_host.get_scrap_folder()
lambda_host.log(f'scrap_folder: {scrap_folder}')
tools = lambda_host.get_tools_folder()
lambda_host.log(f'tools: {tools}\n')
bucket_name = lambda_host.get_cfg("vf_bucket_name")
lambda_host.log(f'bucket_name: {bucket_name}')
access_key_id : str = lambda_host.get_cfg("vf_access_key")
secret_access_key = lambda_host.get_cfg("vf_secret_access")
region = lambda_host.get_cfg("vf_region")
lambda_host.log(f'region: {region}')

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

def swarm_db_upload(projectId, itemId, outputDBFolder, dbName, dbTitle):
    lambda_host.progress(85, 'Create configuration file for uploading dataset')
    
    uploaddb_cfg = f'''
[Configuration]
Organization={OrgId}
Instance={itemId}
Project={projectId}
OutputFolder={outputDBFolder}
dbName={dbName}
dbTitle={dbTitle}
aws_bucket_name={bucket_name}
aws_access_key_id={access_key_id}
aws_secret_access_key={secret_access_key}
aws_region={region}
storage_provider=AWS
'''  
    lambda_host.log(f'{uploaddb_cfg}')
    ini_file = os.path.join(scrap_folder, f'uploaddb_{dbName}.ini')
    lambda_host.log(f'Create surveys configuration file {ini_file}')
    ini = open(ini_file, "w")
    ini.write(uploaddb_cfg)
    ini.close() 
    lambda_host.log(f'{uploaddb_cfg}')
    swarmIndex = 0
    swarmSize = 1
    uploaddb_path = f'{tools}\\Tool.UploadDB.exe {ini_file} {swarmIndex} {swarmSize}'

    lambda_host.attach_file(ini_file)

    start = timer()
    return_code = run_tool(uploaddb_path, 86, 98)
    end = timer()
    lambda_host.log(f'UploadDB: {timedelta(seconds=end - start)}, exit code: {return_code}')
    return return_code

lambda_host.log(f'UploadDB lambda started')
lambda_host.progress(1, 'UploadDB lambda started')
project_id = lambda_host.input_string('project_id', 'Project Id', '')
entity_id = lambda_host.input_string('entity_id', 'Entity Id', '')
data_path = lambda_host.download_entity_files(entity_id)
lambda_host.log(f'data_path: {data_path}')
lambda_host.log(f'project_id: {project_id}')
lambda_host.log(f'entity_id: {entity_id}')

code = swarm_db_upload(project_id, entity_id, scrap_folder, 'vox-pc', 'Voxel Data')

lambda_host.progress(100, 'UploadDB lambda finished')

vf = vfc.rest('http://127.0.0.1')
result = vf.update_entity(
    id=entity_id,
    project=project_id, 
    fields={
        'state' : 'COMPLETE'
    })
if not result.success:
    lambda_host.log(result.error_info)
    exit_code(111)

exit_code(code)
    