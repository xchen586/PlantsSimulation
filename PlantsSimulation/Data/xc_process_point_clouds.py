from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
import os

import subprocess

def process_point_cloud(api : voxelfarmclient.rest, project_id, folder_id, file_path, name : str, version : int, color : bool):

    if not os.path.exists(file_path):
        print(f'File {file_path} does not exist')
        return
    
    file_path_tool = 'D:\\xWork\\VoxelFarm\\LAStools\\bin\\txt2las.exe'
    if not os.path.exists(file_path_tool):
        print(f'Tool File {file_path_tool} does not exist')
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

    result = api.get_project_crs(project_id)
    crs = result.crs
    with open(file_path_laz, 'rb') as f:
        result = api.create_entity_raw(project=project_id, 
            type=api.entity_type.RawPointCloud, 
            name=f'{name}', 
            fields={
                'file_folder': folder_id,
            }, crs = crs)
        entity_id = result.id
        print(f'Attaching file {file_path_laz} to entity {entity_id}')
        api.attach_files(project=project_id, id=entity_id, files={'file': f})
    result = api.create_entity_processed(project=project_id, 
        #type=api.entity_type.VoxelTerrain, 
        type=api.entity_type.IndexedPointCloud,
        name=f'{name}', 
        fields={
            'source': entity_id,
            'source_type': 'RAWPC',
            'file_folder': folder_id,
            #'source_ortho' if color else '_source_ortho': entity_id
        }, crs = crs)
    print(f'Created entity {result.id} for {name} {version}')

api = voxelfarmclient.rest('http://52.226.195.5/')
workflow_api = workflow_lambda.workflow_lambda_host()

# create point cloud from a file
project_id = '1D4CBBD1D957477E8CC3FF376FB87470' #'74F0C96BF0F24DA2BB5AE4ED65D81D8C'
folder_id = '218233195003437A881AEFB3DAFE364A'
project_entity = api.get_entity(project_id)
version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
api.update_entity(project=project_id, id=project_id, fields={'version': version})

result = api.create_folder(project=project_id, name=f'Version {version}', folder=folder_id)
if not result.success:
    print(f'Failed to create folder for version!')
    exit(4)
folder_id = result.id
print(f'Successful to create folder {folder_id} for version!')
tiles = 10
#for x in range(8, 10):
#    for y in range(5, 7):
#        process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_toplevel.xyz', f'toplevel_{tiles}_{x}_{y}', version=version, color=False)
#        process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_bedrock.xyz', f'bottom_{tiles}_{x}_{y}', version=version, color=False)
#        process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_level1.xyz', f'level1_{tiles}_{x}_{y}', version=version, color=False)

x = 8
y = 5

process_point_cloud(api, project_id, folder_id, f'D:\\Downloads\\PlantsSimulation\\output\\mesh_heightmap_raw_export.xyz', f'Mesh_0_height_{tiles}_{x}_{y}-{version}', version=version, color=True)
process_point_cloud(api, project_id, folder_id, f'D:\\Downloads\\PlantsSimulation\\output\\mesh2_heightmap_raw_export.xyz', f'Mesh_1_height_{tiles}_{x}_{y}-{version}', version=version, color=True)
process_point_cloud(api, project_id, folder_id, f'D:\\Downloads\\PlantsSimulation\\output\\pc_heightmap_raw_export.xyz', f'Toplevel_1_height_{tiles}_{x}_{y}-{version}', version=version, color=True)
process_point_cloud(api, project_id, folder_id, f'D:\\Downloads\\PlantsSimulation\\output\\short_height_map_export.xyz', f'Final_short_height_{tiles}_{x}_{y}-{version}', version=version, color=True)
#process_point_cloud(api, project_id, folder_id, f'D:\\Downloads\\PlantsSimulation\\output\\double_height_map_exportout.xyz', f'Final_double_height_{tiles}_{x}_{y}-{version}', version=version, color=True)
process_point_cloud(api, project_id, folder_id, f'D:\\Downloads\\PlantsSimulation\\output\\points_10_8_5_toplevel.xyz.ratio.xyz', f'Toplevel_Ratio_{tiles}_{x}_{y}-{version}', version=version, color=True)
process_point_cloud(api, project_id, folder_id, f'D:\\Downloads\\PlantsSimulation\\output\\points_10_8_5_tree.xyz.ratio.xyz', f'Tree_Ratio_{tiles}_{x}_{y}-{version}', version=version, color=True)

process_point_cloud(api, project_id, folder_id, f'D:\\Downloads\\PlantsSimulation\\output\\points_{tiles}_{x}_{y}_tree.xyz', f'tree_{tiles}_{x}_{y}-{version}', version=version, color=True)

#process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_toplevel.xyz', f'toplevel_{tiles}_{x}_{y}', version=version, color=False)
#process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_bedrock.xyz', f'bottom_{tiles}_{x}_{y}', version=version, color=False)
#process_point_cloud(api, project_id, folder_id, f'C:\\Work\\Voxel Games\\ProcgrenAssets\\output\\pc\\points_{tiles}_{x}_{y}_level1.xyz', f'level1_{tiles}_{x}_{y}', version=version, color=False)
