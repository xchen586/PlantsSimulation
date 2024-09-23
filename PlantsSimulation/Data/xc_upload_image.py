from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
import os

import subprocess

def process_file_image(api : voxelfarmclient.rest, project_id, folder_id, file_path, jgw_path : str, name : str):

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
    
    project_entity = api.get_entity(project_id)
    version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
    api.update_entity(project=project_id, id=project_id, fields={'version': version})

    result = api.create_folder(project=project_id, name=f'Road Image Version {version}', folder=folder_id)
    if not result.success:
        print(f'Failed to create image file folder for version!')
        return 
    entity_folder_id = result.id
    print(f'Successful to create image file folder {entity_folder_id} for version!')

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

api = voxelfarmclient.rest('http://52.226.195.5/')
workflow_api = workflow_lambda.workflow_lambda_host()

tiles = 10
x = 8
y = 5

project_id = '1D4CBBD1D957477E8CC3FF376FB87470' #Pangea Next
folder_id = '90F6348AD5D94FCEA85C7C1CD081CE97' #Pangea Next > instances
image_file_path = f'D:\\Downloads\\Low\\sommothlayer_output\\10_8_5\\points_{tiles}_{x}_{y}_toplevel.xyz.jpg'
image_meta_path = f'D:\\Downloads\\Low\\sommothlayer_output\\10_8_5\\points_{tiles}_{x}_{y}_toplevel.xyz.jgw'


process_file_image(api, project_id, folder_id, image_file_path, image_meta_path, f'RoadImage_{tiles}_{x}_{y}')