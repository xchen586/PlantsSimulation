import os
import subprocess
import glob
from timeit import default_timer as timer
from datetime import timedelta
from voxelfarm import voxelfarmclient as vfc
from os import path
import configparser
from voxelfarm import process_lambda
from voxelfarm import workflow_lambda

OrgId = 2343243456678890

lambda_host = process_lambda.process_lambda_host()
lambda_host.progress(0, 'Starting survey composite...')

def exit_code(code):
    lambda_host.set_exit_code(code)
    exit()

def get_dir_size(path='.'):
    total = 0
    with os.scandir(path) as it:
        for entry in it:
            if entry.is_file():
                total += entry.stat().st_size
            elif entry.is_dir():
                total += get_dir_size(entry.path)
    return total

def check_obj(entity_path):
    types = ('*.OBJ')
    if not os.path.isdir(entity_path):
        lambda_host.log(f'Error download mesh: {entity_path}')
        exit_code(1)

    entity_files = []
    for files in types:
        entity_files.extend(glob.glob(entity_path + "\\" + files))
    if len(entity_files)==0:
        lambda_host.log(f'Error not mesh found: {entity_path}')
        exit_code(2)

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

def upload_block_model(vf, project_id, crs, file_folder, file):
    lambda_host.log(f'upload block model: {file}')    

    result = vf.create_entity_raw(
        project=project_id, 
        type=vf.entity_type.RawBlockModel, 
        name="Raw Block Model", 
        fields={
            'file_folder': f'{file_folder}'
        },
        crs=crs
    )
    if not result.success:
        lambda_host.log(result.error_info)
        exit(3)

    rawbm_id = result.id
    lambda_host.log('Raw Block Model Entity created ' + rawbm_id)

    process_meta = '''[Configuration]
LOD_Quality=0
Column_Count=7
Column0_ID=xs
Column0_Class=XS
Column0_Aggregator=Avg
Column0_DefaultValue=0
Column1_ID=x
Column1_Class=XC
Column1_Aggregator=Avg
Column1_DefaultValue=0
Column2_ID=ys
Column2_Class=YS
Column2_Aggregator=Avg
Column2_DefaultValue=0
Column3_ID=y
Column3_Class=YC
Column3_Aggregator=Avg
Column3_DefaultValue=0
Column4_ID=zs
Column4_Class=ZS
Column4_Aggregator=Avg
Column4_DefaultValue=0
Column5_ID=z
Column5_Class=ZC
Column5_Aggregator=Avg
Column5_DefaultValue=0
Column6_ID=delta
Column6_Class=Value
Column6_Aggregator=Avg
Column6_DefaultValue=0
'''

    meta_attributes_filename = os.path.join(lambda_host.get_scrap_folder(), "process.meta")
    lambda_host.log(f'meta_attributes_filename: {meta_attributes_filename}')
    meta_attributes = open(meta_attributes_filename, "w")
    meta_attributes.write(process_meta)
    meta_attributes.close()

    files = {'file': open(meta_attributes_filename, 'rb')}
    result = vf.attach_files(
        project=project_id, 
        id=rawbm_id,
        files=files)
    if not result.success:
        lambda_host.log(f'error: {result.error_info}, attach file: /process.meta')
        exit(4)

    files = {'file': open(file, 'rb')}
    result = vf.attach_files(
        project=project_id, 
        id=rawbm_id,
        files=files)
    if not result.success:
        lambda_host.log(f'error: {result.error_info}, attach file: {file}')
        exit(4)

    # Create a block model
    result = vf.create_entity_processed(
        project=project_id,
        type=vf.entity_type.BlockModel,
        name="Block Model",
        fields={
            'source': rawbm_id,
            'bm_type': "VOXBM2",
            'file_folder': f'{file_folder}'
        },
        crs=crs)
    if not result.success:
        lambda_host.log(result.error_info)
        exit(5)

    bm_id = result.id
    lambda_host.log('Block Model Created ' + bm_id)

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

scrap_folder= lambda_host.get_scrap_folder()
lambda_host.log(f'scrap_folder: {scrap_folder}')
tools = lambda_host.get_tools_folder()
lambda_host.log(f'tools\n{tools}')

voxel_terrain_id = lambda_host.input_string('voxel_terrain_id', 'Voxel Terrain', '')
new_surface_id = lambda_host.input_string('new_surface_id', 'New Surface', '')
nat_surface_id = lambda_host.input_string('nat_surface_id', 'Natural Surface', '')
hist_surface_id = lambda_host.input_string('hist_surface_id', 'Historic Surface', '')
file_folder = lambda_host.input_string('file_folder', 'file_folder', '')
project_id = lambda_host.input_string('project_id', 'Project Id', '')

lambda_host.log(f'voxel_terrain_id : {voxel_terrain_id}')
lambda_host.log(f'new_surface_id : {new_surface_id}')
lambda_host.log(f'nat_surface_id : {nat_surface_id}')
lambda_host.log(f'hist_surface_id : {hist_surface_id}')
lambda_host.log(f'file_folder : {file_folder}')
lambda_host.log(f'project_id : {project_id}')

vf = vfc.rest('http://127.0.0.1')
result = vf.get_project_crs(project_id)
if not result.success:
    lambda_host.log(result.error_info)
    exit()
crs = result.crs

# 1
lambda_host.progress(1, 'download entity files')

lambda_host.log(f'download new_surface_id: {new_surface_id}')
start = timer()
new_surface_path = lambda_host.download_entity_files(new_surface_id)
check_obj(new_surface_path)
end = timer()
lambda_host.log(f'new_surface_id elapsed: {timedelta(seconds=end - start)}')

lambda_host.log(f'download nat_surface_id: {nat_surface_id}')
start = timer()
nat_surface_path = lambda_host.download_entity_files(nat_surface_id)
check_obj(nat_surface_path)
end = timer()
lambda_host.log(f'nat_surface_path elapsed: {timedelta(seconds=end - start)}')

lambda_host.log(f'download hist_surface_id: {hist_surface_id}')
start = timer()
hist_surface_path = lambda_host.download_entity_files(hist_surface_id)
check_obj(hist_surface_path)
end = timer()
lambda_host.log(f'hist_surface_id elapsed: {timedelta(seconds=end - start)}')

# 2
lambda_host.progress(2, 'Create configuration file')

output_dir = os.path.join(scrap_folder, 'output')
if not os.path.exists(output_dir):
    os.mkdir(output_dir)
    lambda_host.log(f'making output: {output_dir}')

surveys_cfg = f'''
[Configuration]
InSliceCount=3
InSlice0={nat_surface_path}
InSlice1={hist_surface_path}
InSlice2={new_surface_path}
OutputPath={output_dir}
FeaturePointRadius=50.0
MinHeight=0
OriginX={crs["coord_origin_x"]}
OriginY={crs["coord_origin_y"]}
OriginZ={crs["coord_origin_z"]}
VoxelSize={crs["voxel_size"]}
TranslateX=0
TranslateY=0
TranslateZ=0
ScaleX=1
ScaleY=1
ScaleZ=1
EulerAlpha=0
EulerBeta=0
EulerGamma=0
EulerMode=ZXZ
'''

ini_file = os.path.join(scrap_folder, 'surveys.ini')
lambda_host.log(f'Create surveys configuration file\n{ini_file}' )
ini = open(ini_file, "w")
ini.write(surveys_cfg)
ini.close() 

# 3
lambda_host.progress(3, 'run Tool.MeshDifference ')
tool_path = f'{tools}\\Tool.MeshDifference.exe {ini_file}'
lambda_host.log(f'Run {tool_path}')
lambda_host.attach_file(ini_file)
lambda_host.log(surveys_cfg)

start = timer()
return_code = run_tool(tool_path, 3, 80)
end = timer()

if return_code != 33:
    lambda_host.log(f'Error: {return_code} on Tool.MeshDifference, elapsed: {timedelta(seconds=end - start)}')
    exit_code(return_code)
else:
    lambda_host.log(f'Tool.MeshDifference, elapsed: {timedelta(seconds=end - start)}')

# upload block model if exist
lambda_host.progress(81, 'Create block models')
csv_files = glob.glob(f'{output_dir}/*.csv')
if len(csv_files) != 0:
    for file in csv_files:
        upload_block_model(vf, project_id, crs, file_folder, file)
else:
    lambda_host.log(f'Not block_model to upload on: {output_dir}/*.csv' )    

output_size =  f'{get_dir_size(output_dir)}'
lambda_host.log(f'Not block_model to upload on: {output_dir}/*.csv' )    

result = vf.update_entity(
    id=voxel_terrain_id,
    project=project_id, 
    fields={
        'file_size' : output_size
    })

# save entity properties and complete state

# check parameters
lambda_host.log(f'Uploading db from: {output_dir}' )

if lambda_host.upload_db(voxel_terrain_id, output_dir, "vox", "Voxel"):
    on_exit_succeessfull(vf, project_id, voxel_terrain_id, output_dir)
else:
    lambda_host.log('Error on Tool.UploadDB')
    exit_code(3) 

lambda_host.progress(100, 'Done exporting MeshDifference')
lambda_host.set_exit_code(0)
