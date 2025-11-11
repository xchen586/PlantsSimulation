from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
import os

import subprocess

def process_point_cloud(api : voxelfarmclient.rest, file_path_txt2las, project_id, folder_id, file_path,  entityType, entity_basename : str, version : int, color : bool):

    print(f'process_point_cloud project id = {project_id}')
    print(f'process_point_cloud parent folder id = {folder_id}')
    print(f'process_point_cloud point cloud file path = {file_path}') 
    print(f'process_point_cloud txt2las file path = {file_path_txt2las}') 
    
    if not os.path.exists(file_path):
        print(f'Point cloud file {file_path} does not exist')
        return
    if not os.path.exists(file_path_txt2las):
        print(f'Tool txt2las File {file_path_txt2las} does not exist')
        return

    # Run txt2las to convert the txt file to a laz file
    file_path_laz = file_path + '.laz'
    subprocess.run([
        #'C:\\Work\\SDK\\Voxel-Farm-Examples\\Python\\txt2las.exe', '-i', file_path, '-o', file_path_laz,  
        file_path_txt2las, '-i', file_path, '-o', file_path_laz,  
        '-set_version', '1.4', 
        '-set_system_identifier', 'Pangea Next Procgren', 
        '-set_generating_software', 'Pangea Next Procgren',
        '-parse', 'xyzRGB' if color else 'xyz',], 
        stdout=subprocess.PIPE)

    result = api.get_project_crs(project_id)
    crs = result.crs
    
    '''
    project_entity = api.get_entity(project_id)
    version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
    api.update_entity(project=project_id, id=project_id, fields={'version': version})
    
    result = api.create_folder(project=project_id, name=f'Smooth Layer Version {version}', folder=folder_id)
    if not result.success:
        print(f'Failed to Smooth Layer file folder for version {version}!')
        return 
    entity_folder_id = result.id
    print(f'Successful to Smooth Layer file folder {entity_folder_id} for version {version}!')
    '''
    entity_folder_id = folder_id
    result = api.create_entity_raw(project=project_id, 
        type=api.entity_type.RawPointCloud, 
        name=f'{entity_basename}-{entityType}-{version}_src', 
        fields={
            'file_folder': entity_folder_id,
        }, crs = crs)
    entity_id = result.id

    with open(file_path_laz, 'rb') as f:
        api.attach_files(project=project_id, id=entity_id, files={'file': f})
    print(f'Attaching file {file_path_laz} to entity {entity_id}')
    
    result = api.create_entity_processed(project=project_id, 
        #type=api.entity_type.VoxelTerrain, 
        #type=api.entity_type.IndexedPointCloud,
        type = entityType,
        name=f'{entity_basename}-{entityType}-{version}', 
        fields={
            'source': entity_id,
            'source_type': 'RAWPC',
            'file_folder': entity_folder_id,
            #'source_ortho' if color else '_source_ortho': entity_id
        }, crs = crs)
    print(f'Created entity {result.id} for {entity_basename}-{entityType}-{version}')

api = voxelfarmclient.rest('http://52.226.195.5/')
project_id = '1D4CBBD1D957477E8CC3FF376FB87470' #Pangea Next Project

txt2las_exe_path = 'D:\\Downloads\\XCTreeWorkFlow\\Tools\\txt2las.exe'
folder_id = 'B24E708E13C5473FA3BFDBCBA0E68B42' #Pangea Next > Workflow Output > Workflow Smooth Layer Output

tiles = 12
x = 4
y = 2

processed_entity_type = api.entity_type.VoxelTerrain
#src_file_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_double_height_map_exportout.xyz'
#entity_base_name = f'{tiles}_{x}_{y}_double_height_map_surface_export'
src_file_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_short_height_map_export.xyz'
entity_base_name = f'{tiles}_{x}_{y}_short_height_map_surface_export'

project_entity = api.get_entity(project_id)
version = 0
version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
api.update_entity(project=project_id, id=project_id, fields={'version': version})

process_point_cloud(api, txt2las_exe_path, project_id, folder_id, src_file_path, processed_entity_type, entity_base_name, version, color=True)