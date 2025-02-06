from voxelfarm import voxelfarmclient
import os

import subprocess

api = voxelfarmclient.rest('http://52.226.195.5/')

# create point cloud from a file
project_id = '1D4CBBD1D957477E8CC3FF376FB87470' # Pangea Next

folder_id = '90F6348AD5D94FCEA85C7C1CD081CE97' # Pangea Next > Instances

entity_type = api.entity_type.RawPointCloud

entity_name = 'Pangea Next Test Instances Entity'

result = api.get_project_crs(project_id)
if not result.success:
    print(result.error_info)
    exit()
crs = result.crs

# Create raw point cloud entity
result = api.create_entity_raw(
    project=project_id, 
    type=entity_type, 
    name=entity_name, 
    fields={
        'file_folder': folder_id,
        }, crs=crs)

if not result.success:
    print(result.error_info)
    exit()
    
entity_id = result.id
print(f'Raw entity_name : {entity_name} ; Entity type : {entity_type} ; Entity id is {entity_id}')
 