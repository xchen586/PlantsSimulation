from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
import os

import subprocess

api = voxelfarmclient.rest('http://52.226.195.5/')
workflow_api = workflow_lambda.workflow_lambda_host()

isTestResult = True
# create point cloud from a file
project_id = '1D4CBBD1D957477E8CC3FF376FB87470' # Pangea Next

folder_id = '90F6348AD5D94FCEA85C7C1CD081CE97' # Pangea Next > Instances

#game_tree_entity_id_property = '0B4C084415C744B48B4BD13D9990E713' # xc test
#game_tree_entity_id_property = "536674D5E8D440D9A7EFCD1D879AD57A" # cosmin old
game_tree_entity_id_property = "3A3CFEBA226B4692A8719C78335470DD"  #game entity 
if isTestResult:
    game_tree_entity_id_property = '0B4C084415C744B48B4BD13D9990E713' # xc test

entity_id = game_tree_entity_id_property


lambda_code = f'D:\\xWork\\VoxelFarm\\PlantsSimulation\\PlantsSimulation\\Data\\delete_attached_files.py'

vf = api

result = vf.create_process_entity(
    project=project_id,
    name='XC Delete Attached Files',
    code='delete_attached_files.py',
    fields={
        'file_folder' : project_id  #Change to any folder you want, right now the process will be created at the root of your project.
    },
    inputs={
        'project_id' : project_id,
        'entity_id' : entity_id,
    },
    files=[lambda_code]
)
print(f'Created processed entity {result.id} to remove all attached files from entity {entity_id}')


