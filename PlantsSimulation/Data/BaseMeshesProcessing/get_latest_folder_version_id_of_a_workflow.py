import os
from voxelfarm import workflow_lambda
from voxelfarm import voxelfarmclient as vfc
import workflow_framework as wf

# Initialize the VoxelFarm client
vf = vfc.rest(os.getenv('VF_API'))
vf.set_file_credentials(os.getenv("AAD_APP_SECRETS"))
project_id = os.getenv('PROJECT')
framework = wf.workflow_framework(project_id, vf)
lambda_host = workflow_lambda.workflow_lambda_host(framework)


###############################################
# Xuan : Code that you want to copy starts here.
project_entity = vf.get_entity(project_id, project_id)
track_name = 'GCS_MR'
my_workflow_id = project_entity[f'workflow_folder_{track_name}']
print('my_workflow_id', my_workflow_id)
my_workflow_entity = vf.get_entity(my_workflow_id, project_id)
print('my_workflow_entity', my_workflow_entity)
print('version_active', my_workflow_entity['version_active'])
print('version_last', my_workflow_entity['version_last'])

version = int(my_workflow_entity['version_last'])
product_key = 'workflow_folder_' + track_name
if product_key in project_entity:
    product_folder_id = project_entity[product_key]
    if version > 0:
        apiResult = vf.get_collection(project_id)
        if apiResult.success:
            version_name = 'Version ' + str(version)
            entities = apiResult.items
            for e in entities:
                entity = entities[e]
                if ('file_folder' in entity) and (entity['file_folder'] == product_folder_id) and ('file_type' in entity) and (entity['file_type'] == 'FOLDER'):
                    if entity["name"] == version_name:
                        print('latest version_file_folder found : ', entity['file_folder'])
###############################################

''' Output for me is

my_workflow_id D2AEDA7A316D450982B5E1E9CED1C12F
my_workflow_entity {'file_date': '1676663972000', 'file_folder': 'ADDF80BCBDE84E2C8D686822115F8BE0', 'file_type': 'FOLDER', 'name': 'Meshes', 'state': 'COMPLETE', 'type': 'FILE', 'version_active': '6C9C143F5ADA4D0183B0B23998DC3BD3', 'version_last': '74', 'version_last_id': 'F41BF0497345446690B4DEEB85F77650', 'workflow_product': 'GCS_MR', 'workflow_singleton_FULL_INSITU_BLOCK_MODIFIED': '4436C33462C84CC68CAEF6AADFF7C238', 'workflow_singleton_FULL_MESH_QUALITY_MR': 'DEFC8CFF110341E19DB9E43130FA0195', 'workflow_singleton_PLANNING_MESHES_MR': 'D9243C11190D41B4894F2412FEB5F510', 'workflow_singleton_RAW_MESHES_MR': '4D0CF0E2A9FE4776B63344D466B17BCB', 'ID': 'D2AEDA7A316D450982B5E1E9CED1C12F', 'project': '0F3C49D88F364BB39C8C8F3CBE373520'}
version_active 6C9C143F5ADA4D0183B0B23998DC3BD3
version_last 74
latest version_file_folder found :  D2AEDA7A316D450982B5E1E9CED1C12F

'''