from voxelfarm import workflow_lambda
from voxelfarm import voxelfarmclient

import os
import configparser
    
def create_view_for_basemesh_entity(vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('start create_view_for_basemesh_entity')
    
    entity_id = request.raw_entity_id
    
    info_ini_name = 'lambda_info.ini'
    section_entity = 'Entity'
    
    ini_file = lambda_host.download_entity_file(info_ini_name, entity_id)
    lambda_host.log(f'down load file {ini_file} with file name {info_ini_name} from entity {entity_id}')
    if not os.path.isfile(ini_file):
        lambda_host.log(f'{ini_file} is not a file')
        lambda_host.log(f'Fail to create_view_for_basemesh_entity')
        return
    
    config = configparser.ConfigParser()
    config.read(ini_file)
    
    # Check if the section exists
    if section_entity in config:
        # Loop through the section and print each key and value
        for key in config[section_entity]:
            value = config[section_entity][key]
            lambda_host('Basemesh entity name : {key}')
            lambda_host('Basemesh entity id : {value}')
            
            view_name = f'View for {key}'
            lambda_host.log(f'Creating View {view_name}...') 
            try:
                lambda_host.create_view(request, view_name, 'com.voxelfarm.program.view.terrain', None , {'e' : f'{value}'}, {})
            except Exception as e:
                error_message = str(e)
                repr_message = repr(e)
                lambda_host.log(f'Exception of create view {view_name} with entity id {value} with exception error_message of {error_message}')
                lambda_host.log(f'Exception of create view {view_name} with entity id {value} with exception repr_message of {repr_message}')
            lambda_host.log(f'Created View {view_name}...') 
    else:
        lambda_host.log(f'Section {section_entity} not found in the ini file {ini_file}')
        
    lambda_host.log(f'end create_view_for_basemesh_entity')
    
def python_code_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received Python code')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating Python code raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id

    return {'success': True, 'complete': True, 'error_info': 'None'}

def tree_list_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received Tree list')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating Tree list raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id

    return {'success': True, 'complete': True, 'error_info': 'None'}

def road_data_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received road data')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating road data raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id

    return {'success': True, 'complete': True, 'error_info': 'None'}

def base_meshes_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received base meshes')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating base meshes raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
    return {'success': True, 'complete': True, 'error_info': 'None'}

def displacement_maps_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received displacement maps')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating displacement maps raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id

    return {'success': True, 'complete': True, 'error_info': 'None'}

def quadtree_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received quadtree')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating quadtree raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id
    
    return {'success': True, 'complete': True, 'error_info': 'None'}

def tools_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received tools')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating tools raw entity...') 
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Input files', 
            'file_folder' : folder_id
        })
    if not result.success:
        return {'success': False, 'error_info': result.error_info}
    
    # Save the entity ID that has the input files in the request properties
    request.properties['raw_data'] = result.id

    return {'success': True, 'complete': True, 'error_info': 'None'}

def tree_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received tree generation data')
    request.properties['my_property'] = 'my_value'

    pythoncode_active_version_property = request.get_product_property('PYTHON_CODE', 'raw_data')
    treelist_active_version_property = request.get_product_property('TREE_LIST', 'raw_data')
    roaddata_active_version_property = request.get_product_property('ROAD_DATA', 'raw_data')
    basemeshes_active_version_property = request.get_product_property('BASE_MESHES', 'raw_data')
    displacement_active_version_property = request.get_product_property('DISPLACEMENT_MAPS', 'raw_data')
    qtree_active_version_property = request.get_product_property('QUADTREE', 'raw_data')
    tools_active_version_property = request.get_product_property('TOOLS', 'raw_data')

    result = lambda_host.process_lambda_entity(
        workflow_request=request,
        name="Lambda",
        inputs={
            'lambda_entity_id': request.raw_entity_id,
            'project_id': request.project_id,
            'pythoncode_active_version_property': pythoncode_active_version_property,
            'treelist_active_version_property': treelist_active_version_property,
            'roaddata_active_version_property': roaddata_active_version_property,
            'basemeshes_active_version_property': basemeshes_active_version_property,
            'displacement_active_version_property': displacement_active_version_property,
            'qtree_active_version_property': qtree_active_version_property,
            'tools_active_version_property': tools_active_version_property,
            'run_update_basemeshes_assets': False,
            'run_road_exe': True,
            'run_worldgen_road': True,
            'run_make_basemeshes': True,
            'run_upload_basemeshes': False,
            'run_make_tree_instances':True,
            'run_upload_tree_instances': True,
            'run_create_geochem_entity': True,
            'tile_size': 10,
            'tile_x': 8,
            'tile_y': 5,
            'level' : 6,
            'tree_lod': 8,
            'forest_age': 15000,
            'tree_iteration':300
        },
        code='xc_cloud_tree_creation.py',
        files=['xc_cloud_tree_creation.py', 'xc_lambda-uploaddb.py'],
        update_type='msg')

    return {'success': True, 'complete': False, 'error_info': ''}

def tree_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    if update_type == 'msg':
        lambda_host.log('Tree generation stage complete')
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

def basemeshes_generation_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received base meshes generation data')
    request.properties['my_property'] = 'my_value'
    
    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id
    
    result = vf.update_entity(
        id= entity_id,
        project=project_id, 
        fields={
            'file_type' : vf.entity_type.RawMesh,
            'name' : 'Input files', 
            'file_folder' : folder_id
        })

    pythoncode_active_version_property = request.get_product_property('PYTHON_CODE', 'raw_data')
    treelist_active_version_property = request.get_product_property('TREE_LIST', 'raw_data')
    roaddata_active_version_property = request.get_product_property('ROAD_DATA', 'raw_data')
    basemeshes_active_version_property = request.get_product_property('BASE_MESHES', 'raw_data')
    displacement_active_version_property = request.get_product_property('DISPLACEMENT_MAPS', 'raw_data')
    qtree_active_version_property = request.get_product_property('QUADTREE', 'raw_data')
    tools_active_version_property = request.get_product_property('TOOLS', 'raw_data')

    result = lambda_host.process_lambda_entity(
        workflow_request=request,
        name="Lambda",
        inputs={
            'lambda_entity_id': request.raw_entity_id,
            'project_id': request.project_id,
            'pythoncode_active_version_property': pythoncode_active_version_property,
            'treelist_active_version_property': treelist_active_version_property,
            'roaddata_active_version_property': roaddata_active_version_property,
            'basemeshes_active_version_property': basemeshes_active_version_property,
            'displacement_active_version_property': displacement_active_version_property,
            'qtree_active_version_property': qtree_active_version_property,
            'tools_active_version_property': tools_active_version_property,
            'run_update_basemeshes_assets': False,
            'run_road_exe': False,
            'run_worldgen_road': False,
            'run_make_basemeshes': True,
            'run_upload_basemeshes': True,
            'run_make_tree_instances':False,
            'run_upload_tree_instances': False,
            'run_create_geochem_entity': False,
            'tile_size': 10,
            'tile_x': 8,
            'tile_y': 5,
            'level' : 6,
            'tree_lod': 8,
            'forest_age': 15000,
            'tree_iteration':300
        },
        code='xc_cloud_tree_creation.py',
        files=['xc_cloud_tree_creation.py', 'xc_lambda-uploaddb.py'],
        update_type='msg')

    return {'success': True, 'complete': False, 'error_info': ''}

def basemeshes_generation_on_stage_complete(
        vf_api : voxelfarmclient.rest,
        request : workflow_lambda.request,
        lambda_host : workflow_lambda.workflow_lambda_host):
    
    update_type = request.update_type
    lambda_host.log(f'update_type: {update_type}')
    
    create_view_for_basemesh_entity(vf_api, request, lambda_host)
    
    if update_type == 'msg':
        #todo read the file that we attached
        lambda_host.log('Base generation stage complete')
        return {'success': True, 'complete': True, 'error_info': 'None'}

    return {'success': True, 'complete': False, 'error_info': 'None'}

lambda_host = workflow_lambda.workflow_lambda_host()
lambda_host.set_workflow_definition(
    {
        'id': 'PANGEA_Tree',
        'name': 'Tree Generation Pangea Next',
        'icon': 'mesh',
        'description': 'Pangea Tree Generation Workflow',
        'tracks': 
        [
            {
                'id': 'ASSETS',
                'name': 'Assets',
                'description': 'The assets to be used in generation',
                'icon': 'mesh',
                'tracks': [
                    {
                        'id': 'TOOLS',
                        'name': 'Tools',
                        'description': 'The collection of tools for tree creation',
                        'icon': 'mesh',
                        'on_receive_data': tools_on_receive_data,
                    },
                    {
                        'id': 'QUADTREE',
                        'name': 'Quadtree',
                        'description': 'A quadtree of the world',
                        'icon': 'mesh',
                        'on_receive_data': quadtree_on_receive_data,
                    },
                    {
                        'id': 'ROAD_DATA',
                        'name': 'Road Data',
                        'description': 'A Input data for generate road',
                        'icon': 'mesh',
                        'on_receive_data': road_data_on_receive_data,
                    },
                    {
                        'id': 'DISPLACEMENT_MAPS',
                        'name': 'Displacement Maps',
                        'description': 'A collection of PNG heightmaps',
                        'icon': 'mesh',
                        'on_receive_data': displacement_maps_on_receive_data,
                    },
                    {
                        'id': 'BASE_MESHES',
                        'name': 'Base Meshes',
                        'description': 'A collection of OBJ meshes',
                        'icon': 'mesh',
                        'on_receive_data': base_meshes_on_receive_data,
                    },
                    {
                        'id': 'TREE_LIST',
                        'name': 'Tree List',
                        'description': 'A collection of tree classes',
                        'icon': 'mesh',
                        'on_receive_data': tree_list_on_receive_data,
                    },
                    {
                        'id': 'PYTHON_CODE',
                        'name': 'Python Code Files',
                        'description': 'A collection of python code files',
                        'icon': 'mesh',
                        'on_receive_data': python_code_on_receive_data,
                    },
                ]
            },
            {
                'id': 'TREE_GENERATION',
                'name': 'Tree Generation',
                'description': 'The generation of the tree',
                'icon': 'mesh',
                'on_receive_data': tree_generation_on_receive_data,
                'on_stage_done': tree_generation_on_stage_complete,
            },
            {
                'id': 'BASEMESHES_GENERATION',
                'name': 'Base Meshes Generation',
                'description': 'The generation of the base meshes',
                'icon': 'mesh',
                'on_receive_data': basemeshes_generation_on_receive_data,
                'on_stage_done': basemeshes_generation_on_stage_complete,
            }
        ],
    }
)