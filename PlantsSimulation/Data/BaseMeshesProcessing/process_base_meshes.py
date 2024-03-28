from voxelfarm import voxelfarmclient

import subprocess
import os

def process_point_cloud(api : voxelfarmclient.rest, project_id, file_path, name : str, version : int):

    # Run txt2las to convert the txt file to a laz file
    file_path_laz = file_path + '.laz'
    subprocess.run([
        'C:\\Work\\SDK\\Voxel-Farm-Examples\\Python\\txt2las.exe', '-i', file_path, '-o', file_path_laz, '-parse', 'xyz', 
        '-set_version', '1.4', 
        '-set_system_identifier', 'Pangea Next Procgren', 
        '-set_generating_software', 'Pangea Next Procgren',
        '-parse', 'xyz'], 
        stdout=subprocess.PIPE)

    result = api.get_project_crs(project_id)
    crs = result.crs
    with open(file_path_laz, 'rb') as f:
        result = api.create_entity_raw(project=project_id, 
            type=api.entity_type.RawPointCloud, 
            name=f'{name} {version}', 
            fields={}, crs = crs)
        entity_id = result.id
        print(f'Attaching file {file_path_laz} to entity {entity_id}')
        api.attach_files(project=project_id, id=entity_id, files={'file': f})
    result = api.create_entity_processed(project=project_id, 
        type=api.entity_type.VoxelTerrain, 
        name=f'{name} {version}', 
        fields={
            'source': entity_id,
            'source_type': 'RAWPC',
        }, crs = crs)
    print(f'Created entity {result.id} for {name} {version}')

def process_base_meshes(api : voxelfarmclient.rest, project_id, file_path : str, version : int):
    result = api.get_project_crs(project_id)
    crs = result.crs
    entity_id = None
    with open(os.path.join(file_path, 'index.vf'), 'rb') as f:
        result = api.create_entity_raw(project=project_id, 
            type=api.entity_type.VoxelPC, 
            name=f'Basemesh DB {version}', 
            fields={
                'state': 'PARTIAL',
            }, crs = crs)
        entity_id = result.id
        print(f'Attaching file {file_path}\index.vf to entity {entity_id}')
        result = api.attach_files(project=project_id, id=entity_id, files={'file': f})
        if not result.success:
            print(f'Failed to attach file {file_path}\index.vf to entity {entity_id}')
            return

    with open(os.path.join(file_path, 'data.vf'), 'rb') as f:
        print(f'Attaching file {file_path}\data.vf to entity {entity_id}')
        result = api.attach_files(project=project_id, id=entity_id, files={'file': f})
        if not result.success:
            print(f'Failed to attach file {file_path}\data.vf to entity {entity_id}')
            return
    
    result = api.create_process_entity(
        project=project_id,
        type=api.entity_type.Process,
        name="Upload Voxel DB",
        fields={
            'code': 'lambda-uploaddb.py',
            'input_value_entity_id': entity_id,
            'input_value_project_id': project_id,
        },
        crs=crs,
        files=['lambdas/survey-composite-process.py', 'voxelfarmclient.py'])

    print(f'Created entity {result.id} for {name} {version}')

api = voxelfarmclient.rest('http://192.168.2.11/')

# create point cloud from a file
project_id = '74F0C96BF0F24DA2BB5AE4ED65D81D8C'
project_entity = api.get_entity(project_id)
version = int(project_entity['basemesh_version']) + 1 if 'basemesh_version' in project_entity else 1
api.update_entity(project=project_id, id=project_id, fields={'basemesh_version': version})
x = 8
y = 5
process_base_meshes(api, project_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\vox_db', version=version)