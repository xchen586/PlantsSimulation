import os
import subprocess
from zipfile import ZipFile
import glob
from timeit import default_timer as timer
from datetime import timedelta
from voxelfarm import voxelfarmclient as vfc
from os import path
import configparser
from voxelfarm import process_lambda
from distutils.dir_util import copy_tree

OrgId = 2343243456678890

lambda_host = process_lambda.process_lambda_host()

lambda_host.progress(0, 'Starting Lambda...')
scrap_folder= lambda_host.get_scrap_folder()
lambda_host.log(f'scrap_folder: {scrap_folder}')
tools = lambda_host.get_tools_folder()
lambda_host.log(f'tools: {tools}\n')

def exit_code(code):
    lambda_host.set_exit_code(code)

def run_tool(tool_path, progress_start, progress_end):    
    
    lambda_host.log(f'tool_path:\n{tool_path}')

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

lambda_host.log(f'QuadTree lambda started')
lambda_host.progress(1, 'QuadTree lambda started')

project_id = lambda_host.input_string('project_id', 'Project Id', '')
entity_id = lambda_host.input_string('entity_id', 'Entity Id', '')
output_id = lambda_host.input_string('output_id', 'Output Id', '')
tile_size = lambda_host.input_string('tile_size', 'Tile Size', '')
tile_x = lambda_host.input_string('tile_x', 'Tile X', '')
tile_y = lambda_host.input_string('tile_y', 'Tile Y', '')
level = lambda_host.input_string('level', 'Level', '')
entity_folder = lambda_host.get_entity_folder()

data_path = lambda_host.download_entity_files(entity_id)
override_data_path = entity_folder
lambda_host.log(f'data_path: {data_path}')
lambda_host.log(f'override_data_path: {override_data_path}')
lambda_host.log(f'project_id: {project_id}')
lambda_host.log(f'entity_id: {entity_id}')
lambda_host.log(f'output_id: {output_id}')
lambda_host.log(f'entity_folder: {entity_folder}')

lambda_host.log(f'Copying from {override_data_path} to {data_path}')
copy_tree(override_data_path, data_path)
lambda_host.log(f'Done copying.')

vf = vfc.rest('http://127.0.0.1')

vox_db = os.path.join(scrap_folder, f'vox_{level}_{tile_size}_{tile_x}_{tile_y}')
cache = os.path.join(scrap_folder, f'cache_{level}_{tile_size}_{tile_x}_{tile_y}')
lambda_host.log(f'vox_db: {vox_db}')
code = run_tool(f'{entity_folder}\Tool.BaseMeshVoxelizer.exe {tile_size} {tile_x} {tile_y} {level} {data_path} {vox_db} {cache}', 0, 100)
lambda_host.log(f'Tool.BaseMeshVoxelizer {level}: {code}')
if code == 8:
    lambda_host.log(f'Nothing to process, deleting...')
    vf.delete_entities(project=project_id, ids=[output_id])
    exit_code(0)
elif code == 0:
    if not lambda_host.upload_db(output_id, vox_db, "vox-pc", "Voxel Data"):
        lambda_host.log('Error on Tool.UploadDB')
        exit_code(3) 
        exit()

    result = vf.update_entity(
        id=output_id,
        project=project_id, 
        fields={
            'state' : 'COMPLETE'
        })
    if not result.success:
        lambda_host.log(result.error_info)
        exit_code(4)
        exit()
else:
    exit_code(2)
    exit()

lambda_host.progress(100, 'QuadTree lambda finished')
exit_code(0)
    