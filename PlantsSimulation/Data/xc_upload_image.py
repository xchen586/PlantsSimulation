from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
import os

import subprocess

def process_file_image(api : voxelfarmclient.rest, project_id, folder_id, file_path, jgw_path : str, name : str, version : int):

    if not os.path.exists(file_path):
        print(f'File {file_path} does not exist')
        return
    if not os.path.exists(jgw_path):
        print(f'File {jgw_path} does not exist')
        return
    
    result = api.get_project_crs(project_id)
    crs = result.crs
    result = api.create_entity_raw(project=project_id, 
            type=api.entity_type.IndexedOrthoImagery, 
            name=f'{name}_src', 
            fields={
                'file_folder': folder_id,
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
        name=f'{name}_IdxOrtho', 
        fields={
            'source': entity_id,
            'source_type': 'ORTHO',
            'file_folder': folder_id,
            'source_ortho': entity_id
        }, crs = crs)
    print(f'Created entity {result.id} for {name} {version}')

api = voxelfarmclient.rest('http://52.226.195.5/')
workflow_api = workflow_lambda.workflow_lambda_host()

# create point cloud from a file
project_id = '1D4CBBD1D957477E8CC3FF376FB87470' #Pangea Next
folder_id = '90F6348AD5D94FCEA85C7C1CD081CE97' #Pangea Next > instances

project_entity = api.get_entity(project_id)
version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
api.update_entity(project=project_id, id=project_id, fields={'version': version})

result = api.create_folder(project=project_id, name=f'Version {version}', folder=folder_id)
if not result.success:
    print(f'Failed to create folder for version!')
    exit(4)
folder_id = result.id
print(f'Successful to create folder {folder_id} for version!')

tiles = 10
x = 8
y = 5

process_file_image(api, project_id, folder_id, 
                   f'D:\Downloads\\Low\\sommothlayer_output\\10_8_5\\points_{tiles}_{x}_{y}_toplevel.xyz.jpg', 
                   f'D:\Downloads\\Low\\sommothlayer_output\\10_8_5\\points_{tiles}_{x}_{y}_toplevel.xyz.jgw', 
                   f'RoadImage_{tiles}_{x}_{y}-{version}', version=version)