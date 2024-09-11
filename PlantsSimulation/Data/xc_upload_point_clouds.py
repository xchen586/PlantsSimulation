from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
import os

import subprocess

#file_path_tool = 'D:\\xWork\\VoxelFarm\\LAStools\\bin\\txt2las.exe'

def process_point_cloud(api : voxelfarmclient.rest, project_id, folder_id, entity_type, file_path, file_path_tool, name : str, color : bool):

    print(f'start to process_point_cloud')
    print(f'process_point_cloud project_id : {project_id}')
    print(f'process_point_cloud folder_id : {folder_id}')
    print(f'process_point_cloud file_path : {file_path}')
    print(f'process_point_cloud file_path_tool : {file_path_tool}')
    
    if not os.path.exists(file_path):
        print(f'process_point_cloud File {file_path} does not exist')
        return
    
    if not os.path.exists(file_path_tool):
        print(f'process_point_cloud Tool File {file_path_tool} does not exist')
        return

    # Run txt2las to convert the txt file to a laz file
    file_path_laz = file_path + '.laz'
    subprocess.run([
        #'C:\\Work\\SDK\\Voxel-Farm-Examples\\Python\\txt2las.exe', '-i', file_path, '-o', file_path_laz,  
        file_path_tool, '-i', file_path, '-o', file_path_laz,  
        '-set_version', '1.4', 
        '-set_system_identifier', 'Pangea Next Procgren', 
        '-set_generating_software', 'Pangea Next Procgren',
        '-parse', 'xyzRGB' if color else 'xyz',], 
        stdout=subprocess.PIPE)
    
    project_entity = api.get_entity(project_id)
    version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
    api.update_entity(project=project_id, id=project_id, fields={'version': version})

    result = api.create_folder(project=project_id, name=f'{name} Version {version}', folder=folder_id)
    if not result.success:
        print(f'Failed to create folder for version!')
        return
    entity_folder_id = result.id
    print(f'Successful to create folder {folder_id} for version!')

    result = api.get_project_crs(project_id)
    crs = result.crs
    with open(file_path_laz, 'rb') as f:
        result = api.create_entity_raw(project=project_id, 
            type=api.entity_type.RawPointCloud, 
            name=f'{name}', 
            fields={
                'file_folder': entity_folder_id,
            }, crs = crs)
        entity_id = result.id
        print(f'Attaching file {file_path_laz} to entity {entity_id}')
        api.attach_files(project=project_id, id=entity_id, files={'file': f})
    result = api.create_entity_processed(project=project_id, 
        type = entity_type,
        name=f'{name}', 
        fields={
            'source': entity_id,
            'source_type': 'RAWPC',
            'file_folder': entity_folder_id,
            #'source_ortho' if color else '_source_ortho': entity_id
        }, crs = crs)
    print(f'Created entity {result.id} for {name} {version}')

api = voxelfarmclient.rest('http://52.226.195.5/')
workflow_api = workflow_lambda.workflow_lambda_host()

# create point cloud from a file
project_id = '1D4CBBD1D957477E8CC3FF376FB87470' #'74F0C96BF0F24DA2BB5AE4ED65D81D8C'
folder_id = '90F6348AD5D94FCEA85C7C1CD081CE97'

entity_type_IndexedPointCloud = api.entity_type.IndexedPointCloud
entity_type_VoxelTerrain = api.entity_type.VoxelTerrain
entity_type_IndexedOrthoImagery = api.entity_type.IndexedOrthoImagery
entity_type_VoxelTerrain = api.entity_type.VoxelTerrain
entity_type_IndexedMesh = api.entity_type.IndexedMesh


tiles = 10
x = 8
y = 5

