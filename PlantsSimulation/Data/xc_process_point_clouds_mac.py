from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
import os

import subprocess

def process_point_cloud(api : voxelfarmclient.rest, file_path_txt2las, project_id, folder_id, file_path,  entity_basename : str, version : int, color : bool):

    if not os.path.exists(file_path):
        print(f'File {file_path} does not exist')
        return
    
    if not os.path.exists(file_path_txt2las):
        print(f'Tool File {file_path_txt2las} does not exist')
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
    result = api.create_entity_raw(project=project_id, 
            type=api.entity_type.RawPointCloud, 
            name=f'{entity_basename}_src', 
            fields={
                'file_folder': folder_id,
            }, crs = crs)
    entity_id = result.id
    with open(file_path_laz, 'rb') as f:
        print(f'Attaching file {file_path_laz} to entity {entity_id}')
        api.attach_files(project=project_id, id=entity_id, files={'file': f})
    result = api.create_entity_processed(project=project_id, 
        type=api.entity_type.VoxelTerrain, 
        #type=api.entity_type.IndexedPointCloud,
        name=f'{entity_basename}', 
        fields={
            'source': entity_id,
            'source_type': 'RAWPC',
            'file_folder': folder_id,
            #'source_ortho' if color else '_source_ortho': entity_id
        }, crs = crs)
    print(f'Created entity {result.id} for {entity_basename} {version}')

api = voxelfarmclient.rest('http://52.226.195.5/')
workflow_api = workflow_lambda.workflow_lambda_host()

# create point cloud from a file
project_id = '1D4CBBD1D957477E8CC3FF376FB87470' #Pangea Next #'74F0C96BF0F24DA2BB5AE4ED65D81D8C'
#folder_id = '218233195003437A881AEFB3DAFE364A' #Pangea Next > Smooth
folder_id = '68396F90F7CE48B4BA1412EA020ED92A' #Pangea Next > Workflow Output > Workflow BaseMeshes Output
project_entity = api.get_entity(project_id)
version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
api.update_entity(project=project_id, id=project_id, fields={'version': version})

result = api.create_folder(project=project_id, name=f'Version {version}', folder=folder_id)
if not result.success:
    print(f'Failed to create folder for version!')
    exit(4)
folder_id = result.id
print(f'Successful to create folder {folder_id} for version!')
tiles = 25
#for x in range(8, 10):
#    for y in range(5, 7):
#        process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxe#l Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_toplevel.xyz', f'toplevel_{tiles}_{x}_{y}', version=version, color=False)
#        process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_bedrock.xyz', f'bottom_{tiles}_{x}_{y}', version=version, color=False)
#        process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_level1.xyz', f'level1_{tiles}_{x}_{y}', version=version, color=False)

x = 9
y = 6
level = 0
#file_path_txt2las = '/Users/xchen586/xWork/VoxelFarm/LAStools/bin/txt2las'
file_path_txt2las = 'D:\\Downloads\\XCTreeWorkFlow\\Tools\\txt2las.exe'
#file_path_txt2las = 'D:\\xWork\\VoxelFarm\\LAStools\\bin\\txt2las.exe'

#process_point_cloud(api, file_path_txt2las, project_id, folder_id, f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\25_8_5\\points_25_8_5_toplevel.xyz', f'toplevel_{tiles}_{x}_{y}-{version}', version=version, color=True)
#process_point_cloud(api, file_path_txt2las, project_id, folder_id, f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\25_8_5\\points_25_8_5_level1.xyz', f'level1_{tiles}_{x}_{y}-{version}', version=version, color=True)
#process_point_cloud(api, file_path_txt2las, project_id, folder_id, f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\25_8_5\\points_25_8_5_bedrock.xyz', f'bedrock_{tiles}_{x}_{y}-{version}', version=version, color=True)

#process_point_cloud(api, project_id, folder_id, f'/Users/xchen586/Downloads/PlantsSimulationAssets/outputpoints_{tiles}_{x}_{y}_tree.xyz', f'tree_{tiles}_{x}_{y}-{version}', version=version, color=True)
#process_point_cloud(api, project_id, folder_id, f'D:\\Downloads\\PlantsSimulation\\output\\points_{tiles}_{x}_{y}_tree.xyz', f'tree_{tiles}_{x}_{y}-{version}', version=version, color=True)
#process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_toplevel.xyz', f'toplevel_{tiles}_{x}_{y}', version=version, color=False)
#process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_bedrock.xyz', f'bottom_{tiles}_{x}_{y}', version=version, color=False)
#process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_level1.xyz', f'level1_{tiles}_{x}_{y}', version=version, color=False)

#process_point_cloud(api, file_path_txt2las, project_id, folder_id, f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_short_height_map_export.xyz', f'Final_Height_{tiles}_{x}_{y}-{version}', version=version, color=True)

#process_point_cloud(api, file_path_txt2las, project_id, folder_id, f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\db\\{tiles}_{x}_{y}_{level}\\{tiles}_{x}_{y}_{level}_basemeshes_qtree_nodes_pointcloud.csv', f'Basemeshes_Nodes_Height_{tiles}_{x}_{y}_{level}-{version}', version=version, color=True)
process_point_cloud(api, file_path_txt2las, project_id, folder_id, f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\db\\{tiles}_{x}_{y}_{level}\\{tiles}_{x}_{y}_{level}_basemeshes_qtree_nodes_minheight_pointcloud.xyz', f'Basemeshes_Nodes_MinHeight_{tiles}_{x}_{y}_{level}-{version}', version=version, color=True)