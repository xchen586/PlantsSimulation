from voxelfarm import voxelfarmclient as vfc
import os
#from dotenv import load_dotenv

#load_dotenv()

print('Publishing Tree Workflow Code')

project_id = '0B0B6CCD4F56423C8196B7E9EA690E97'
workflow_path = 'D:\\xWork\\VoxelFarm\\PlantsSimulation\\PlantsSimulation\\Data\\WorkFlow'
workflow_code = 'workflow-tree.py'
vf_api = 'http://52.226.195.5/'
#vf_api = 'http://localhost'
vf_api = 'https://demo.voxelfarm.com/'
#aad_app_secrets = 

print(f'Project: {project_id}')
print(f'Using VF API: {vf_api}')

vf = vfc.rest(vf_api)

print(f'Apply credentials to VF API: {vf_api}')
#if (aad_app_secrets!=None):
#    vf.set_file_credentials(aad_app_secrets)

vf.deploy_workflow(project_id, workflow_path, workflow_code)    