from voxelfarm import voxelfarmclient
from voxelfarm import workflow_lambda
import os

import subprocess

def process_file_image(api : voxelfarmclient.rest, project_id, folder_id, file_path, jgw_path : str, name : str, version):

    print(f'process_file_image project id = {project_id}')
    print(f'process_file_image parent folder id = {folder_id}')
    print(f'process_file_image image file path = {file_path}')
    print(f'process_file_image image meta path = {jgw_path}')   
     
    if not os.path.exists(file_path):
        print(f'Image File {file_path} does not exist')
        return
    if not os.path.exists(jgw_path):
        print(f'Image Meta File {jgw_path} does not exist')
        return
    
    result = api.get_project_crs(project_id)
    crs = result.crs
    
    '''
    project_entity = api.get_entity(project_id)
    version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
    api.update_entity(project=project_id, id=project_id, fields={'version': version})
    '''
    result = api.create_folder(project=project_id, name=f'Road Image Version {version}', folder=folder_id)
    if not result.success:
        print(f'Failed to create image file folder for version!')
        return 
    #entity_folder_id = result.id
    #print(f'Successful to create image file folder {entity_folder_id} for version!')

    entity_folder_id = folder_id
    result = api.create_entity_raw(project=project_id, 
            #type=api.entity_type.IndexedOrthoImagery, 
            type=api.entity_type.OrthoImagery, 
            name=f'{name}-{version}_src', 
            fields={
                'file_folder': entity_folder_id,
            }, crs = crs)
    entity_id = result.id
    
    with open(jgw_path, 'rb') as j:
        api.attach_files(project=project_id, id=entity_id, files={'file': j})
    print(f'Attaching file {jgw_path} to entity {entity_id}')
    with open(file_path, 'rb') as f:
        api.attach_files(project=project_id, id=entity_id, files={'file': f})
    print(f'Attaching file {file_path} to entity {entity_id}')

    result = api.create_entity_processed(project=project_id, 
        type=api.entity_type.IndexedOrthoImagery, 
        name=f'{name}-{version}', 
        fields={
            'source': entity_id,
            'source_type': 'ORTHO',
            'file_folder': entity_folder_id,
            'source_ortho': entity_id
        }, crs = crs)
    print(f'Created entity {result.id} for {name}-{version}')

api = voxelfarmclient.rest('http://52.226.195.5/')
workflow_api = workflow_lambda.workflow_lambda_host()



tiles = 12
x = 4
#y = 5
y = 2

project_id = '1D4CBBD1D957477E8CC3FF376FB87470' #Pangea Next
#folder_id = '90F6348AD5D94FCEA85C7C1CD081CE97' #Pangea Next > instances
folder_id = '218233195003437A881AEFB3DAFE364A' #Pangea Next > Smooth
#folder_id = '75FDBF01261147F3A50E4A6CFDE059D3' #Pangea Next > Workflow Output > Workflow Test Whole Result Output

folder_id = 'B24E708E13C5473FA3BFDBCBA0E68B42' #Pangea Next > Workflow Output > Workflow Smooth Layer Output
#image_file_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\25_8_5\\points_{tiles}_{x}_{y}_toplevel.xyz.jpg'
#image_meta_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\25_8_5\\points_{tiles}_{x}_{y}_toplevel.xyz.jgw'

project_entity = api.get_entity(project_id)
version = int(project_entity['version']) + 1 if 'version' in project_entity else 1
api.update_entity(project=project_id, id=project_id, fields={'version': version})
    
min = 0
max = 4000
image_file_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\{tiles}_{x}_{y}\\points_{tiles}_{x}_{y}_toplevel.xyz.jpg'
image_meta_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\{tiles}_{x}_{y}\\points_{tiles}_{x}_{y}_toplevel.xyz.jgw'

image_file_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_slopes_blur_{min}_{max}.png'
image_meta_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\sommothlayer_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_slopes_blur_{min}_{max}.pgw'

#process_file_image(api, project_id, folder_id, image_file_path, image_meta_path, f'Slope_blur_600_600_{tiles}_{x}_{y}_vmin_{min}_vmax_{max}')

Exposure_map_red_image_file_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_exposure_map_red.png'
Exposure_map_red_image_meta_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_exposure_map_red.pgw'
process_file_image(api, project_id, folder_id, Exposure_map_red_image_file_path, Exposure_map_red_image_meta_path, f'{tiles}_{x}_{y}_exposure_map_red', version)

Exposure_map_rgb_image_file_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_exposure_map_rgb.png'
Exposure_map_rgb_image_meta_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_exposure_map_rgb.pgw'
process_file_image(api, project_id, folder_id, Exposure_map_rgb_image_file_path, Exposure_map_rgb_image_meta_path, f'{tiles}_{x}_{y}_exposure_map_rgb', version)

Exposure_map_grey_image_file_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_exposure_map_grey.png'
Exposure_map_grey_image_meta_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_exposure_map_grey.pgw'
process_file_image(api, project_id, folder_id, Exposure_map_grey_image_file_path, Exposure_map_grey_image_meta_path, f'{tiles}_{x}_{y}_exposure_map_grey', version)

Exposure_init_map_red_image_file_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_exposure_init_map_red.png'
Exposure_init_map_red_image_meta_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_exposure_init_map_red.pgw'
process_file_image(api, project_id, folder_id, Exposure_init_map_red_image_file_path, Exposure_init_map_red_image_meta_path, f'{tiles}_{x}_{y}_exposure_init_map_red', version)

Exposure_init_map_rgb_image_file_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_exposure_init_map_rgb.png'
Exposure_init_map_rgb_image_meta_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_exposure_init_map_rgb.pgw'
process_file_image(api, project_id, folder_id, Exposure_init_map_rgb_image_file_path, Exposure_init_map_rgb_image_meta_path, f'{tiles}_{x}_{y}_exposure_init_map_rgb', version)

Exposure_init_map_grey_image_file_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_exposure_init_map_grey.png'
Exposure_init_map_grey_image_meta_path = f'D:\\Downloads\\XCTreeCreation\\Tree_Big_Creation\\tree_output\\{tiles}_{x}_{y}\\{tiles}_{x}_{y}_exposure_init_map_grey.pgw'
process_file_image(api, project_id, folder_id, Exposure_init_map_grey_image_file_path, Exposure_init_map_grey_image_meta_path, f'{tiles}_{x}_{y}_exposure_initmap_grey', version)