from voxelfarm import voxelfarmclient
import os

import subprocess

api = voxelfarmclient.rest('http://52.226.195.5/')

tiles = 12
x = 4
y = 2

# create point cloud from a file
project_id = '1D4CBBD1D957477E8CC3FF376FB87470' # Pangea Next

folder_id = '90F6348AD5D94FCEA85C7C1CD081CE97' # Pangea Next > Instances

entity_type = api.entity_type.RawPointCloud

game_entity_base_name = 'Pangea_Next_Game_Instances_Entity'

test_entity_base_name = 'Pangea_Next_Test_Instances_Entity'

project_entity = api.get_entity(project_id)
version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
api.update_entity(project=project_id, id=project_id, fields={'version': version})

game_entity_name = f'{tiles}_{x}_{y}_{game_entity_base_name}_{version}'
test_entity_name = f'{tiles}_{x}_{y}_{test_entity_base_name}_{version}'

result = api.get_project_crs(project_id)
if not result.success:
    print(result.error_info)
    exit()
crs = result.crs

# Create game instanes entity
result = api.create_entity_raw(
    project=project_id, 
    type=entity_type, 
    name=game_entity_name, 
    fields={
        'file_folder': folder_id,
        }, crs=crs)

if not result.success:
    print(result.error_info)
    exit()
    
entity_id = result.id
print(f'Pangea Next Game Instances Entity : {game_entity_name} ; Entity type : {entity_type} ; Entity id is {entity_id}')
 
# Create test instanes entity
result = api.create_entity_raw(
    project=project_id, 
    type=entity_type, 
    name=test_entity_name, 
    fields={
        'file_folder': folder_id,
        }, crs=crs)

if not result.success:
    print(result.error_info)
    exit()
    
entity_id = result.id
print(f'Pangea Next Test Instances Entity : {test_entity_name} ; Entity type : {entity_type} ; Entity id is {entity_id}')
 