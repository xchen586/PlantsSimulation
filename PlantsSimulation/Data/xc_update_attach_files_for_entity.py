from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
import os

import subprocess

def update_attach_files_for_entity(api : voxelfarmclient.rest, project_id, entity_id, folder_path):

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

needDeleteFiles = False

#tiles = 10
#x = 8
#y = 5
#update_attach_files_for_entity(api, project_id, entity_id, f'D:\\Downloads\\XCTreeBackup\\Output\\Tree_output\\20241017Low\\{tiles}_{x}_{y}\\regionoutput')
#update_attach_files_for_entity(api, project_id, entity_id, f'D:\\Downloads\\XCTreeBackup\\Output\\Tree_output\\20241017Low\\{tiles}_{x}_{y}\\instanceoutput')

########################
# Delete all files inside an entity
# lambda_code = r'PATH TO YOUR PYTHON FILE' # For example c:\workspace\voxel\merging_old_gradeblocks_to_maxreturn\xuan_delete_file_inside_entity\delete_attached_files.py
lambda_code = r'D:\xWork\VoxelFarm\PlantsSimulation\PlantsSimulation\Data\delete_attached_files.py'

vf = api
if needDeleteFiles:
    result = vf.create_process_entity(
        project=project_id,
        name='Delete Attached Files',
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


tiles = 25
x = 8
y = 5

update_attach_files_for_entity(api, project_id, entity_id, f'D:\\Downloads\\XCTreeBackup\\Output\\Tree_output\\20250127QTree16Ver460For25_8_5_NewCave_Tree_150_POI_100\\{tiles}_{x}_{y}\\regionoutput')
update_attach_files_for_entity(api, project_id, entity_id, f'D:\\Downloads\\XCTreeBackup\\Output\\Tree_output\\20250127QTree16Ver460For25_8_5_NewCave_Tree_150_POI_100\\{tiles}_{x}_{y}\\instanceoutput')