from voxelfarm import workflow_lambda
from voxelfarm import voxelfarmclient


def road_data_on_receive_data(
        vf : voxelfarmclient.rest, 
        request : workflow_lambda.request, 
        lambda_host : workflow_lambda.workflow_lambda_host):
    lambda_host.log('Received road data')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating raw entity...') 
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

    lambda_host.log('Updating raw entity...') 
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

    lambda_host.log('Updating raw entity...') 
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

    lambda_host.log('Updating raw entity...') 
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
    lambda_host.log('Received quadtree')

    entity_id = request.raw_entity_id
    project_id = request.project_id
    folder_id = request.version_folder_id

    lambda_host.log('Updating raw entity...') 
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

    roaddata_active_version_property = request.get_product_property('ROAD_DATA', 'raw_data')
    basemeshes_active_version_property = request.get_product_property('BASE_MESHES', 'raw_data')
    displacement_active_version_property = request.get_product_property('DISPLACEMENT_MAPS', 'raw_data')
    qtree_active_version_property = request.get_product_property('QUADTREE', 'raw_data')
    tools_active_version_property = request.get_product_property('TOOLS', 'raw_data')

    result = lambda_host.process_lambda_entity(
        workflow_request=request,
        name="Lambda",
        inputs={
            'project_id': request.project_id,
            'roaddata_active_version_property': roaddata_active_version_property,
            'basemeshes_active_version_property': basemeshes_active_version_property,
            'displacement_active_version_property': displacement_active_version_property,
            'qtree_active_version_property': qtree_active_version_property,
            'tools_active_version_property': tools_active_version_property,
            'tile_size': 10,
            'tile_x': 8,
            'tile_y': 5,
            'level' : 6,
        },
        code='xc_cloud_tree_creation.py',
        files=['xc_cloud_tree_creation.py'],
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
                ]
            },
            {
                'id': 'TREE_GENERATION',
                'name': 'Tree Generation',
                'description': 'The generation of the tree',
                'icon': 'mesh',
                'on_receive_data': tree_generation_on_receive_data,
                'on_stage_done': tree_generation_on_stage_complete,
            }
        ],
    }
)