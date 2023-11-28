from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
import os

import subprocess

def process_files_entity(api : voxelfarmclient.rest, project_id, folder_id, raw_entity_type, entity_type, folder_path, name : str, version : int, color : bool):

    if not os.path.exists(folder_path):
        print(f'File {folder_path} does not exist')
        return
    
    # Use the os.listdir() function to get a list of filenames in the folder
    file_names = os.listdir(folder_path)

    # Create a list of file paths by joining the folder path with each file name
    file_paths = [os.path.join(folder_path, file_name) for file_name in file_names]    
    print(file_paths)

    result = api.get_project_crs(project_id)
    crs = result.crs
    
    result = api.create_entity_raw(project=project_id, 
        type = raw_entity_type,
        #type=api.entity_type.RawMesh, 
        #type=entity_type,
        name=f'{name}', 
        fields={
            'file_folder': folder_id,
        }, crs = crs)
    entity_id = result.id
    print(f'Attaching file {file_paths} to entity {entity_id}')
    #api.attach_files(project=project_id, id=entity_id, files={'file': file})
    for file_path in file_paths:
        with open(file_path, "rb") as file:
            api.attach_files(project=project_id, id=entity_id, files={'file': file})

    result = api.create_entity_processed(project=project_id, 
        #type=api.entity_type.VoxelTerrain, 
        #type=api.entity_type.VoxelMesh,
        #type=api.entity_type.IndexedPointCloud,
        type = entity_type,
        name=f'{name}', 
        fields={
            'source': entity_id,
            'source_type': 'RAWPC',
            'file_folder': folder_id,
            #'source_ortho' if color else '_source_ortho': entity_id
        }, crs = crs)
    print(f'--------Created entity {result.id} for {name} {version}--------')

api = voxelfarmclient.rest('http://52.226.195.5/')
workflow_api = workflow_lambda.workflow_lambda_host()

# create point cloud from a file
project_id = '1D4CBBD1D957477E8CC3FF376FB87470' #'74F0C96BF0F24DA2BB5AE4ED65D81D8C'
#folder_id = '218233195003437A881AEFB3DAFE364A'
folder_id = '90F6348AD5D94FCEA85C7C1CD081CE97'
project_entity = api.get_entity(project_id)
entity_type_IndexedPointCloud = api.entity_type.IndexedPointCloud
entity_type_VoxelTerrain = api.entity_type.VoxelTerrain
entity_type_IndexedMesh = api.entity_type.IndexedMesh

version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
api.update_entity(project=project_id, id=project_id, fields={'version': version})

result = api.create_folder(project=project_id, name=f'Version {version}', folder=folder_id)
if not result.success:
    print(f'Failed to create folder for version!')
    exit(4)
folder_id = result.id
print(f'-----------------Successful to create folder {folder_id} for version!-----------------')
tiles = 10
#for x in range(8, 10):
#    for y in range(5, 7):
#        process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_toplevel.xyz', f'toplevel_{tiles}_{x}_{y}', version=version, color=False)
#        process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_bedrock.xyz', f'bottom_{tiles}_{x}_{y}', version=version, color=False)
#        process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_level1.xyz', f'level1_{tiles}_{x}_{y}', version=version, color=False)

x = 8
y = 5

#process_files_point_cloud(api, project_id, folder_id, entity_type_IndexedPointCloud, f'D:\\Downloads\\PlantsSimulation\\output\\instanceoutput', f'instances_{tiles}_{x}_{y}-{version}', version=version, color=True)
#process_files_point_cloud(api, project_id, folder_id, entity_type_IndexedPointCloud, f'D:\\Downloads\\PlantsSimulation\\output\\instanceoutputwithoutoffeset', f'instances_without_offset_{tiles}_{x}_{y}-{version}', version=version, color=True)
#process_files_point_cloud(api, project_id, folder_id, entity_type_IndexedPointCloud, f'D:\\Downloads\\PlantsSimulation\\output\\instanceoutput', f'instances_lod8_{tiles}_{x}_{y}-{version}', version=version, color=True)
#process_files_point_cloud(api, project_id, folder_id, entity_type_IndexedPointCloud, f'D:\\Downloads\\PlantsSimulation\\output\\instanceoutputwithoutoffeset', f'instances_lod8_without_offset_{tiles}_{x}_{y}-{version}', version=version, color=True)

process_files_entity(api, project_id, folder_id, api.entity_type.RawMesh, entity_type_IndexedMesh, f'D:\\Downloads\\ProcgrenAssets\\TestWC', f'IndexedMeshes_{tiles}_{x}_{y}-{version}', version=version, color=True)