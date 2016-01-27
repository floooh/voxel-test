"""fips verb to build the webpage"""

import os
import yaml 
import shutil
import subprocess
import glob
from string import Template

from mod import log, util, project, emscripten

BuildEmscripten = True

#-------------------------------------------------------------------------------
def deploy_webpage(fips_dir, proj_dir, webpage_dir) :
    """builds the final webpage under under fips-deploy/voxel-test-webpage"""
    ws_dir = util.get_workspace_dir(fips_dir)

    # copy other required files
    for name in ['style.css', 'emsc.js', 'favicon.png'] :
        log.info('> copy file: {}'.format(name))
        shutil.copy(proj_dir + '/web/' + name, webpage_dir + '/' + name)

    # generate emscripten HTML page
    if emscripten.check_exists(fips_dir) :
        emsc_deploy_dir = '{}/fips-deploy/voxel-test/emsc-ninja-release'.format(ws_dir)
        name = 'VoxelTest'
        log.info('> generate emscripten HTML page: {}'.format(name))
        for ext in ['js', 'html.mem'] :
            src_path = '{}/{}.{}'.format(emsc_deploy_dir, name, ext)
            if os.path.isfile(src_path) :
                shutil.copy(src_path, webpage_dir)
        with open(proj_dir + '/web/emsc.html', 'r') as f :
            templ = Template(f.read())
        html = templ.safe_substitute(name=name)
        with open('{}/index.html'.format(webpage_dir), 'w') as f :
            f.write(html)

#-------------------------------------------------------------------------------
def build_deploy_webpage(fips_dir, proj_dir) :
    # if webpage dir exists, clear it first
    ws_dir = util.get_workspace_dir(fips_dir)
    webpage_dir = '{}/fips-deploy/voxel-test-webpage'.format(ws_dir)
    if os.path.isdir(webpage_dir) :
        shutil.rmtree(webpage_dir)
    os.makedirs(webpage_dir)

    # compile emscripten, pnacl and android samples
    if BuildEmscripten and emscripten.check_exists(fips_dir) :
        project.gen(fips_dir, proj_dir, 'emsc-ninja-release')
        project.build(fips_dir, proj_dir, 'emsc-ninja-release')
    
    # deploy the webpage
    deploy_webpage(fips_dir, proj_dir, webpage_dir)

    log.colored(log.GREEN, 'Generated web page under {}.'.format(webpage_dir))

#-------------------------------------------------------------------------------
def serve_webpage(fips_dir, proj_dir) :
    ws_dir = util.get_workspace_dir(fips_dir)
    webpage_dir = '{}/fips-deploy/voxel-test-webpage'.format(ws_dir)
    p = util.get_host_platform()
    if p == 'osx' :
        try :
            subprocess.call(
                'open http://localhost:8000 ; python {}/mod/httpserver.py'.format(fips_dir),
                cwd = webpage_dir, shell=True)
        except KeyboardInterrupt :
            pass
    elif p == 'win':
        try:
            subprocess.call(
                'cmd /c start http://localhost:8000 && python {}/mod/httpserver.py'.format(fips_dir),
                cwd = webpage_dir, shell=True)
        except KeyboardInterrupt:
            pass
    elif p == 'linux':
        try:
            subprocess.call(
                'xdg-open http://localhost:8000; python {}/mod/httpserver.py'.format(fips_dir),
                cwd = webpage_dir, shell=True)
        except KeyboardInterrupt:
            pass

#-------------------------------------------------------------------------------
def run(fips_dir, proj_dir, args) :
    if len(args) > 0 :
        if args[0] == 'build' :
            build_deploy_webpage(fips_dir, proj_dir)
        elif args[0] == 'serve' :
            serve_webpage(fips_dir, proj_dir)
        else :
            log.error("Invalid param '{}', expected 'build' or 'serve'".format(args[0]))
    else :
        log.error("Param 'build' or 'serve' expected")

#-------------------------------------------------------------------------------
def help() :
    log.info(log.YELLOW +
             'fips webpage build\n' +
             'fips webpage serve\n' +
             log.DEF +
             '    build voxel-test webpage')

