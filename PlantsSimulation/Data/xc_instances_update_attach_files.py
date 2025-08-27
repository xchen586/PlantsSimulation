from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
import os

import subprocess

def update_attach_file_for_entity(api : voxelfarmclient.rest, project_id, entity_id, file_path):

    if not os.path.exists(file_path):
        print(f'Attach File {file_path} does not exist')
        return
    
    file_paths = [file_path]    
    print(f'{file_paths}')

    print(f'Attaching file {file_paths} to entity {entity_id}')
    for file_path in file_paths:
        with open(file_path, "rb") as file:
            api.attach_files(project=project_id, id=entity_id, files={'file': file})
            
def update_attach_files_for_entity(api : voxelfarmclient.rest, project_id, entity_id, folder_path):

    if not os.path.exists(folder_path):
        print(f'Attach folder of File {folder_path} does not exist')
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

isTestResult = False

# create point cloud from a file
project_id = '1D4CBBD1D957477E8CC3FF376FB87470' # Pangea Next

folder_id = '90F6348AD5D94FCEA85C7C1CD081CE97' # Pangea Next > Instances

needDeleteFiles = False
# Delete all files inside an entity
# lambda_code = r'PATH TO YOUR PYTHON FILE' # For example c:\workspace\voxel\merging_old_gradeblocks_to_maxreturn\xuan_delete_file_inside_entity\delete_attached_files.py

################################
#game_tree_entity_id_property = 'E0070AD37D4543FCB9E70D60AE47541D' # cosmin new
#game_tree_entity_id_property = "536674D5E8D440D9A7EFCD1D879AD57A" # cosmin old
#game_tree_entity_id_property = "3A3CFEBA226B4692A8719C78335470DD"  #game entity 
#game_tree_entity_id_property = '0B4C084415C744B48B4BD13D9990E713' # xuan test 
#entity_id = game_tree_entity_id_property

#tiles = 10
#x = 8
#y = 5
#update_attach_files_for_entity(api, project_id, entity_id, f'D:\\Downloads\\XCTreeBackup\\Output\\Tree_output\\20241017Low\\{tiles}_{x}_{y}\\regionoutput')
#update_attach_files_for_entity(api, project_id, entity_id, f'D:\\Downloads\\XCTreeBackup\\Output\\Tree_output\\20241017Low\\{tiles}_{x}_{y}\\instanceoutput')


################################
#tiles = 25
#x = 8
#y = 5

#game_tree_entity_id_property = 'BE04D7A0D18142AE9D024C1A3FD50BED'  #game entity "Pangea Next Game Instances Entity"
#if isTestResult:
#    game_tree_entity_id_property = '6387B1C0BBC24AC3A7830F27807657A7' # xc test "Pangea Next Test Instances Entity"
#entity_id = game_tree_entity_id_property

#update_attach_files_for_entity(api, project_id, entity_id, f'D:\\Downloads\\XCTreeBackup\\Output\\Tree_output\\20250206QTree16Ver464For25_8_5_NewCave_Tree_150_POI_100_Spwan_DecreaseNPC\\{tiles}_{x}_{y}\\regionoutput')
#update_attach_files_for_entity(api, project_id, entity_id, f'D:\\Downloads\\XCTreeBackup\\Output\\Tree_output\\20250206QTree16Ver464For25_8_5_NewCave_Tree_150_POI_100_Spwan_DecreaseNPC\\{tiles}_{x}_{y}\\instanceoutput')

#######################################
tiles = 12
x = 4
y = 2

game_tree_entity_id_property = '546217C1DE184353AFD6F14EFAED35C9'  #game entity "12_4_2_Pangea_Next_Game_Instances_Entity_573"
if isTestResult:
   game_tree_entity_id_property = 'DB5EFD08F7AF4725BABBFDC74E2C5ED8' # xc test "12_4_2_Pangea_Next_Test_Instances_Entity_573"
entity_id = game_tree_entity_id_property
backup_base_folder = f'D:\\Downloads\\XCTreeBackup\\Output\\Tree_output\\20250824QTree16GeoVer811_12_4_2_RegenerateNewInstancesFileWithLevelColumn_RoadRoughness'
backup_folder = os.path.join(backup_base_folder, f'{tiles}_{x}_{y}')
regions_info_file = os.path.join(backup_folder, 'regions_info.csv')
region_output_folder = os.path.join(backup_folder, 'regionoutput')
instance_output_folder = os.path.join(backup_folder, 'instanceoutput')

update_attach_file_for_entity(api, project_id, entity_id, regions_info_file)
update_attach_files_for_entity(api, project_id, entity_id, region_output_folder)
update_attach_files_for_entity(api, project_id, entity_id, instance_output_folder)
