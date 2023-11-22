from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
import os

import subprocess

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

api = voxelfarmclient.rest('http://52.226.195.5/')
workflow_api = workflow_lambda.workflow_lambda_host()

# create point cloud from a file
project_id = '1D4CBBD1D957477E8CC3FF376FB87470'

folder_id = '90F6348AD5D94FCEA85C7C1CD081CE97'

entity_id = 'E0070AD37D4543FCB9E70D60AE47541D'

#entity_id = "536674D5E8D440D9A7EFCD1D879AD57A"

entity_type_IndexedPointCloud = api.entity_type.IndexedPointCloud

version = 80

tiles = 10

x = 8

y = 5

update_attach_files_for_entity(api, project_id, entity_id, f'D:\\Downloads\\PlantsSimulation\\output\\instanceoutput', f'instances_lod8_{tiles}_{x}_{y}-{version}', version=version, color=True)

